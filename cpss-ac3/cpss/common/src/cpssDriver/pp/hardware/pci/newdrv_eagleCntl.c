/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file newdrv_eagleCntl.c
*
* @brief Driver for EAGLE PEX
*       Supports full mamory mapped address space
*       (Naive windowing support included)
* Resources are mapped to user-space
*
* @version   1
********************************************************************************
*/
#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/log/prvCpssLog.h>
#ifdef ASIC_SIMULATION
#include <asicSimulation/SCIB/scib.h>
#endif

#include <cpss/common/cpssTypes.h>

#define CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM 64
/* Current window sise 1M */
#define WINDOW_SIZE          _1M
#define REGISTER_OFFSET_BITS_MASK (WINDOW_SIZE - 1)
#define REGISTER_WINDOW_BITS_MASK ~(WINDOW_SIZE - 1)

typedef struct CPSS_HW_DRIVER_GEN_AMAP_WIN64_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_UINTPTR          base;
    GT_UINTPTR          size;
    struct {
        GT_UINTPTR      reg;        /* Register offset in BAR0 to configure the "mapAddress" value */
        GT_U32          mapAddress; /* Translated Base Address */
        GT_UINTPTR      winBase;    /* Base Address in BAR = BAR base + offset */
    } window[CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM]; /* address mapping windows 0..63 */
    GT_U32 numOfWindows;
    GT_U32 nextWindowIndex;
    CPSS_HW_DRIVER_STC* bar0;
    struct {
        GT_U32   busNo;
        GT_U32   devSel;
        GT_U32   funcNo;
    } hwAddr;
} CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC;

/* PEX ATU (Address Translation Unit) registers */
#define ATU_REGISTERS_OFFSET_IN_BAR0  0x1200
#define ATU_REGION_CTRL_1_REG         0x100
#define ATU_REGION_CTRL_2_REG         0x104
#define ATU_LOWER_BASE_ADDRESS_REG    0x108
#define ATU_UPPER_BASE_ADDRESS_REG    0x10C
#define ATU_LIMIT_ADDRESS_REG         0x110
#define ATU_LOWER_TARGET_ADDRESS_REG  0x114
#define ATU_UPPER_TARGET_ADDRESS_REG  0x118

#define iATU_REGISTER_ADDRESS(_iWindow, _register)  \
    (ATU_REGISTERS_OFFSET_IN_BAR0 + (_iWindow)*0x200 + (_register))

/*#define PEX_DELAY*/
#ifdef PEX_DELAY
#define PEX_DELAY_MS cpssOsTimerWkAfter(5)
#else
#define PEX_DELAY_MS
#endif

#define PEX_ACCESS_TRACE
#ifdef PEX_ACCESS_TRACE
GT_BOOL pex_access_trace_enable = GT_FALSE;
#define PEX_ACCESS_TRACE_PRINTF(...) \
    if( GT_TRUE == pex_access_trace_enable) \
    {                                       \
        cpssOsPrintf(__VA_ARGS__);          \
    }
#else
#define PEX_ACCESS_TRACE_PRINTF(...)
#endif

#define ATU_DEBUG_STAT
#ifdef ATU_DEBUG_STAT
CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *debugDrv = NULL;
GT_U32 numOfAccess = 0;
GT_U32 numOfHits = 0;
GT_U32 windowHit[CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM];
GT_U32 windowReplace[CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM];
#endif

/* resolve address mapping window */
static GT_STATUS prvGetWindow(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv,
    IN  GT_U32      regAddr,
    OUT GT_UINTPTR  *winBase,
    OUT GT_U32      *addrInWin,
    INOUT GT_BOOL   *locked
)
{
    GT_U32 ii;
    GT_U32 data;
    GT_STATUS   rc;

    if (!drv->numOfWindows)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (*locked == GT_FALSE)
    {
        cpssOsMutexLock(drv->mtx);
        *locked = GT_TRUE;
    }

#ifdef ATU_DEBUG_STAT
    numOfAccess++;
#endif

    data = regAddr & REGISTER_WINDOW_BITS_MASK;

    for (ii = 0; ii < drv->numOfWindows; ii++)
    {
        /* compare window mapping (1M size) to reg address */
        if (drv->window[ii].mapAddress == data)
        {
            *winBase = drv->window[ii].winBase;
            *addrInWin = (regAddr & REGISTER_OFFSET_BITS_MASK);

#ifdef ATU_DEBUG_STAT
            numOfHits++;
            windowHit[ii]++;
#endif

            return GT_OK;
        }
    }
    ii = drv->nextWindowIndex;
    drv->nextWindowIndex++;
    if (drv->nextWindowIndex >= drv->numOfWindows)
        drv->nextWindowIndex = 0;

#ifdef ATU_DEBUG_STAT
            windowReplace[ii]++;
#endif

    /* access to BAR0 to configure mapped address*/
    rc = drv->bar0->writeMask(drv->bar0, 0,
            drv->window[ii].reg, &data, 1, 0xFFFFFFFF);
    PEX_ACCESS_TRACE_PRINTF("PEX BAR0 write: rc %d, regAddr 0x%8.8x, data 0x%8.8x\n", rc, drv->window[ii].reg, data);
    if (rc != GT_OK)
        return rc;

    GT_SYNC; /* to avoid from write combining */

/* Patch due to PEX timing\race problem */
    {
       GT_U32 tempData;
       rc = drv->bar0->read(drv->bar0, 0, drv->window[ii].reg, &tempData, 1);
       if (rc != GT_OK)
           return rc;
    }

    PEX_DELAY_MS;

    drv->window[ii].mapAddress = data;
    *winBase = drv->window[ii].winBase;
    *addrInWin = (regAddr & REGISTER_OFFSET_BITS_MASK);
    return GT_OK;
}

static GT_STATUS cpssHwDriverEagleAmapWin64Read(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_UINTPTR  winBase;
    GT_U32      addrInWin = WINDOW_SIZE;
    GT_U32      data;
    GT_BOOL     locked = GT_FALSE;
    GT_STATUS rc = GT_BAD_PARAM;

    PEX_ACCESS_TRACE_PRINTF("PEX Read: regAddr 0x%8.8x\n", regAddr);

    /* CPSS_HW_DRIVER_AS_CNM_E is used to enable access to the configuration */
    /* space for debug purpose with Internal PCI APIs. */
    if( addrSpace == CPSS_HW_DRIVER_AS_PCI_CONFIG_E ||
        addrSpace == CPSS_HW_DRIVER_AS_CNM_E)
    {
        /* Configuration Space access */
#ifdef ASIC_SIMULATION
        scibPciRegRead(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
        return cpssExtDrvPciConfigReadReg(drv->hwAddr.busNo,
                                          drv->hwAddr.devSel,
                                          drv->hwAddr.funcNo,
                                          regAddr, dataPtr);
#endif
    }
    else if ( addrSpace == CPSS_HW_DRIVER_AS_ATU_E )
    {
        return(drv->bar0->read(drv->bar0, 0, regAddr, dataPtr, count));
    }

    for (; count; count--,dataPtr++)
    {
        if (addrInWin >= WINDOW_SIZE)
        {
            rc = prvGetWindow(drv, regAddr, &winBase, &addrInWin, &locked);
            if (rc != GT_OK)
                break;
        }
#ifdef ASIC_SIMULATION
        scibReadMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
        data = *((volatile GT_U32*)(winBase + addrInWin));
#endif
        GT_SYNC; /* to avoid read combining */
        PEX_DELAY_MS;
        *dataPtr = CPSS_32BIT_LE(data);
        addrInWin += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);

    PEX_ACCESS_TRACE_PRINTF("PEX Read: rc %d, regAddr 0x%8.8x, data 0x%8.8x\n", rc, regAddr-4, *(dataPtr-1));

    return rc;
}

static GT_STATUS cpssHwDriverEagleAmapWin64WriteMask(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_UINTPTR  winBase;
    GT_U32      addrInWin = WINDOW_SIZE;
    GT_U32      data;
    GT_BOOL     locked = GT_FALSE;
    GT_STATUS rc = GT_BAD_PARAM;

    PEX_ACCESS_TRACE_PRINTF("PEX Write: regAddr 0x%8.8x, data 0x%8.8x, mask 0x%8.8x\n", regAddr, *dataPtr, mask);

    /* CPSS_HW_DRIVER_AS_CNM_E is used to enable access to the configuration */
    /* space for debug purpose with Internal PCI APIs. */
    if( addrSpace == CPSS_HW_DRIVER_AS_PCI_CONFIG_E ||
        addrSpace == CPSS_HW_DRIVER_AS_CNM_E)
    {
        /* Configuration Space access */
#ifdef ASIC_SIMULATION
        scibPciRegWrite(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
        return cpssExtDrvPciConfigWriteReg(drv->hwAddr.busNo,
                                           drv->hwAddr.devSel,
                                           drv->hwAddr.funcNo,
                                           regAddr, *dataPtr);
#endif
    }
    else if ( addrSpace == CPSS_HW_DRIVER_AS_ATU_E )
    {
        return(drv->bar0->writeMask(drv->bar0, 0, regAddr, dataPtr, count, mask));
    }

    for (; count; count--,dataPtr++)
    {
        if (addrInWin >= WINDOW_SIZE)
        {
            rc = prvGetWindow(drv, regAddr, &winBase, &addrInWin, &locked);
            if (rc != GT_OK)
                break;
        }

        if (mask != 0xffffffff)
        {
#ifdef ASIC_SIMULATION
            scibReadMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
            data = CPSS_32BIT_LE(*((volatile GT_U32*)(winBase + addrInWin)));
            GT_SYNC; /* to avoid read combining */
            PEX_DELAY_MS;
#endif
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
#ifdef ASIC_SIMULATION
        scibWriteMemory(scibGetDeviceId(drv->base), regAddr, 1, &data);
#else
        *((volatile GT_U32*)(winBase + addrInWin)) = CPSS_32BIT_LE(data);
#endif
        GT_SYNC; /* to avoid from write combining */
        PEX_DELAY_MS;
        addrInWin += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);

    PEX_ACCESS_TRACE_PRINTF("PEX Write: rc %d\n", rc);

    return rc;
}

static GT_STATUS cpssHwDriverEagleAmapWin64DestroyDrv(
    IN  CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverEagleAmapWin64CreateDrv function
* @endinternal
*
* @brief Create driver instance for Address Mapping based on
*        available 64 windows
*
* @param[in] base           - resource virtual address
* @param[in] size           - mapped resource size
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size currently expected to be up to 64M (the default
*        4M)
*/
CPSS_HW_DRIVER_STC *cpssHwDriverEagleAmapWin64CreateDrv(
    IN  CPSS_HW_DRIVER_STC *bar0,
    IN  CPSS_HW_INFO_STC *hwInfo
)
{
    CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC *drv;
    char        buf[64];
    GT_U32      ii;
    GT_U32      data;
    GT_STATUS   rc;

    if (!hwInfo->resource.switching.start)
        return NULL;
    if (hwInfo->resource.switching.size < _1M)
        return NULL;

    /* Currently for the naive flow (=NFLOW) we are expecting up to 64M size */
    /* the default is 4M size */
    if (hwInfo->resource.switching.size > _64M)
        return NULL;


    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverEagleAmapWin64Read;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverEagleAmapWin64WriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverEagleAmapWin64DestroyDrv;

    drv->bar0 = bar0;
    drv->base = hwInfo->resource.switching.start;
    drv->size = hwInfo->resource.switching.size;

    drv->hwAddr.busNo  = hwInfo->hwAddr.busNo;
    drv->hwAddr.devSel = hwInfo->hwAddr.devSel;
    drv->hwAddr.funcNo = hwInfo->hwAddr.funcNo;

    for (ii = 0; ii < CPSS_HW_DRIVER_EAGLE_AMAP_WINDOWS_NUM; ii++)
    {
        /* Currently, for the NFLOW we are using 1M windows offset in bar */
        if ( (ii+1)*WINDOW_SIZE > hwInfo->resource.switching.size)
            break; /* region don't fit to mapped area */

        /* region base address - bar1 base + region*1M */
        data = hwInfo->resource.switching.phys + ii*WINDOW_SIZE;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_LOWER_BASE_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        /* region limit address - bar1 base + region*1M + (1M -1) */
        data += (WINDOW_SIZE - 1);
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_LIMIT_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        data = (hwInfo->resource.switching.phys >> 32);
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_UPPER_BASE_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        data = 0x0;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_UPPER_TARGET_ADDRESS_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        /* type of region to be mem */
        data = 0x0;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_REGION_CTRL_1_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        /* enable the region */
        data = 0x80000000;
        rc = bar0->writeMask(bar0, 0,
                             iATU_REGISTER_ADDRESS(ii, ATU_REGION_CTRL_2_REG),
                             &data, 1, 0xFFFFFFFF);
        if( GT_OK != rc)
        {
            return NULL;
        }

        drv->window[ii].reg = iATU_REGISTER_ADDRESS(ii, ATU_LOWER_TARGET_ADDRESS_REG);
        drv->window[ii].winBase = hwInfo->resource.switching.start + ii*WINDOW_SIZE;
        drv->numOfWindows++;
    }

    if (drv->numOfWindows)
    {
        cpssOsSprintf(buf, "eagleAmap%p", (void*)hwInfo->resource.switching.start);
        cpssOsMutexCreate(buf, &(drv->mtx));
    }

#ifdef ATU_DEBUG_STAT
    debugDrv = drv;
    for (ii = 0 ; ii < debugDrv->numOfWindows ; ii++ )
    {
        windowHit[ii] = 0;
        windowReplace[ii] = 0;
    }
#endif

    return (CPSS_HW_DRIVER_STC*)drv;
}

#ifdef ASIC_SIMULATION
static GT_STATUS cpssHwDriverSimulationEaglePexBar0Read(
    IN  CPSS_HW_DRIVER_STC *drv GT_UNUSED,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr GT_UNUSED,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
#ifdef _VISUALC
    (void)drv;
    (void)addrSpace;
    (void)regAddr;
#endif
    for (; count; count--,dataPtr++)
    {
        *dataPtr = 0;
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverSimulationEaglePexBar0WriteMask(
    IN  CPSS_HW_DRIVER_STC *drv GT_UNUSED,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr GT_UNUSED,
    IN  GT_U32 *dataPtr GT_UNUSED,
    IN  GT_U32  count GT_UNUSED,
    IN  GT_U32  mask GT_UNUSED
)
{
#ifdef _VISUALC
    (void)drv;
    (void)addrSpace;
    (void)regAddr;
    (void)dataPtr;
    (void)count;
    (void)mask;
#endif
    return GT_OK;
}

static GT_STATUS cpssHwDriverSimulationEaglePexBar0Destroy(
    IN  CPSS_HW_DRIVER_STC *drv
)
{
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverSimulationEaglePexBar0CreateDrv function
* @endinternal
*
* @brief Create driver instance for simulated BAR0 access -
*        actually a NOP driver
*
* @param[in] hwInfo         - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSimulationEaglePexBar0CreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo GT_UNUSED
)
{
    CPSS_HW_DRIVER_STC *drv;

#ifdef _VISUALC
    (void)hwInfo;
#endif

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverSimulationEaglePexBar0Read;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverSimulationEaglePexBar0WriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverSimulationEaglePexBar0Destroy;

    return drv;
}
#endif

/**(CPSS_HW_DRIVER_STC*)
* @internal cpssHwDriverEaglePexCreateDrv function
* @endinternal
*
* @brief Create driver instance for Eagle PEX - PP Resources are
*        mapped to user-space
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverEaglePexCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
)
{
    CPSS_HW_DRIVER_STC *drv, *bar0;

    if (hwInfo->resource.switching.start == 0)
        return NULL;
#ifdef ASIC_SIMULATION
    bar0 = cpssHwDriverSimulationEaglePexBar0CreateDrv(hwInfo);
#else
    bar0 = cpssHwDriverGenMmapCreateDrv(
                hwInfo->resource.cnm.start,
                hwInfo->resource.cnm.size);
#endif

#if __WORDSIZE == 64
    if (hwInfo->resource.switching.start &&
        hwInfo->resource.switching.size >= 0x80000000L)
    {
CPSS_TBD_BOOKMARK_FALCON_EMULATOR
/* The direct usage of Generic driver in case of 2G window is no longer valid */
/* Since we need access to BAR0 registers for outbound ATU and also to the    */
/* Configuration Space */
        return cpssHwDriverGenMmapCreateDrv(
                            hwInfo->resource.switching.start,
                            hwInfo->resource.switching.size
                            );
    }
#endif

    if (hwInfo->resource.cnm.start)
    {
        drv = cpssHwDriverEagleAmapWin64CreateDrv(bar0, hwInfo);

CPSS_TBD_BOOKMARK_FALCON_EMULATOR
#ifndef ASIC_SIMULATION
        {
            /* TEMPORARY AMB bridge windows configurations. */
            /* Will be embedded in the Asic HW design (including emulator) */
            GT_U32 AMB_CnM_Config[][2] = { /* MG0 */
                                           {0x1dffff80, 0x0},
                                           {0x1dffff84, 0x1d000000},
                                           {0x1dffff80, 0xf0041},
                                           /* MG1 */
                                           {0x1dffff88, 0x0},
                                           {0x1dffff8c, 0x1d100000},
                                           {0x1dffff88, 0xf0051},
                                           /* MG2 */
                                           {0x1dffff90, 0x0},
                                           {0x1dffff94, 0x1d200000},
                                           {0x1dffff90, 0xf0061},
                                           /* MG3 */
                                           {0x1dffff98, 0x0},
                                           {0x1dffff9c, 0x1d300000},
                                           {0x1dffff98, 0xf0071},
                                           /* MG4 */
                                           {0x1dffffa0, 0x0},
                                           {0x1dffffa4, 0x1d400000},
                                           {0x1dffffa0, 0xf0081},
                                           /* MG5 */
                                           {0x1dffffa8, 0x0},
                                           {0x1dffffac, 0x1d500000},
                                           {0x1dffffa8, 0xf0091},
                                           /* MG6 */
                                           {0x1dffffb0, 0x0},
                                           {0x1dffffb4, 0x1d600000},
                                           {0x1dffffb0, 0xf00a1},
                                           /* MG7 */
                                           {0x1dffffb8, 0x0},
                                           {0x1dffffbc, 0x1d700000},
                                           {0x1dffffb8, 0xf00b1},
                                           /* Runit-internal */
                                           {0x1e00ff80, 0x0},
                                           {0x1e00ff84, 0x1e000000},
                                           {0x1e00ff80, 0x10011},
                                           /* Runit-bottom */
                                           {0x1e00ff88, 0x0},
                                           {0x1e00ff8c, 0x1e020000},
                                           {0x1e00ff88, 0x1d11},
                                           /* Runit-uart-0 */
                                           {0x1e00ff90, 0x0},
                                           {0x1e00ff94, 0x1e100000},
                                           {0x1e00ff90, 0xf0111},
                                           /* Runit-uart-1 */
                                           {0x1e00ff98, 0x0},
                                           {0x1e00ff9c, 0x1e200000},
                                           {0x1e00ff98, 0xf0111},
                                           /* Runit-SPI-0 */
                                           {0x1e00ffa0, 0x0},
                                           {0x1e00ffa4, 0x1e800000},
                                           {0x1e00ffa0, 0x7f1e11},
                                           /* Runit-SPI-1 */
                                           {0x1e00ffa8, 0x0},
                                           {0x1e00ffac, 0x1f000000},
                                           {0x1e00ffa8, 0x7f5e11},
                                           /* Runit-lunit */
                                           {0x1e00ffb0, 0x0},
                                           {0x1e00ffb4, 0x1f800000},
                                           {0x1e00ffb0, 0xc1},
                                           /* Runit-Nunit */
                                           {0x1e00ffb8, 0x0},
                                           {0x1e00ffbc, 0x1f810000},
                                           {0x1e00ffb8, 0x7e0001},

                                           /* DFX Server AMB WA */
                                           {0x1befff80, 0xf0080},
                                           {0x1befff84, 0x0},
                                           {0x1befff80, 0xf0081},

                                        };
            GT_U32 data;
            GT_U32 ii;

            for( ii = 0 ;
                 ii < (sizeof(AMB_CnM_Config)/(2*sizeof(GT_U32))) ;
                 ii++)
            {
                data = AMB_CnM_Config[ii][1];
                cpssHwDriverEagleAmapWin64WriteMask(
                (CPSS_HW_DRIVER_GEN_AMAP_WIN64_STC*)drv,
                    CPSS_HW_DRIVER_AS_SWITCHING_E,
                    AMB_CnM_Config[ii][0] , &data, 1, 0xffffffff);
            }
        }
#endif /* #ifndef ASIC_SIMULATION */
        return drv;
    }


    return NULL;
}


#ifdef PEX_ACCESS_TRACE
void pexAccessTraceEnable(GT_BOOL enable)
{
    pex_access_trace_enable = enable;
}
#endif

#ifdef ATU_DEBUG_STAT
void printAtuStat(void)
{
    GT_U32 ii;

    if( NULL == debugDrv )
    {
        cpssOsPrintf("No driver created.\n");
        return;
    }

    cpssOsPrintf("Num of access: %d\nNum of hits: %d\n", numOfAccess, numOfHits);
    numOfAccess = 0;
    numOfHits = 0;

    for (ii = 0 ; ii < debugDrv->numOfWindows ; ii++ )
    {
        cpssOsPrintf("Window %2d: Hits - %d, Replaces - %d\n", ii, windowHit[ii], windowReplace[ii]);
        windowHit[ii] = 0;
        windowReplace[ii] = 0;
    }
    return;
}

void printAtuMap(void)
{
    GT_U32 ii;

    if( NULL == debugDrv )
    {
        cpssOsPrintf("No driver created.\n");
        return;
    }

    for (ii = 0 ; ii < debugDrv->numOfWindows ; ii++ )
    {
        cpssOsPrintf("Window %2d: map 0x%08x\n", ii, debugDrv->window[ii].mapAddress);
    }
    return;
}
#endif

