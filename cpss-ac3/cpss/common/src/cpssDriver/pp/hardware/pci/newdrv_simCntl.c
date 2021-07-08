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
* @file newdrv_simCntl.c
*
* @brief CPSS hwPp driver for asic simulation mode
*
* @version   1
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <asicSimulation/SCIB/scib.h>


typedef struct CPSS_HW_DRIVER_SIMULATION_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_U32              simDevId;
    GT_BOOL             useAddressCompletion;
    GT_UINTPTR          base;
    struct {
        GT_U32          val;
        GT_UINTPTR      reg;
        GT_UINTPTR      region;
    } compl[7]; /* completion for regions 1..7 */
    int numRegions;
    int nextIndex;
} CPSS_HW_DRIVER_SIMULATION_STC;


/* resolve address completion */
static GT_STATUS prvGetAc(
    IN  CPSS_HW_DRIVER_SIMULATION_STC *drv,
    IN  GT_U32      regAddr,
    OUT GT_UINTPTR  *complRegion,
    OUT GT_U32      *addrInRegion,
    INOUT GT_BOOL   *locked
)
{
    GT_U32 compl;
    int i;
    if (regAddr < 0x80000)
    {
        /* region 0, access without address completion */
        *complRegion = drv->base;
        *addrInRegion = regAddr;
        return GT_OK;
    }
    if (!drv->numRegions)
        return GT_FAIL;
    if (*locked == GT_FALSE)
    {
        cpssOsMutexLock(drv->mtx);
        *locked = GT_TRUE;
    }
    compl = (regAddr >> 19);
    for (i = 0; i < drv->numRegions; i++)
    {
        if (drv->compl[i].val == compl)
        {
            *complRegion = drv->compl[i].region;
            *addrInRegion = (regAddr & 0x0007ffff);
            return GT_OK;
        }
    }
    i = drv->nextIndex;
    drv->nextIndex++;
    if (drv->nextIndex >= drv->numRegions)
        drv->nextIndex = 0;
    drv->compl[i].val = compl;
    compl = CPSS_32BIT_LE(compl);
    scibWriteMemory(drv->simDevId, drv->compl[i].reg, 1, &compl);
    *complRegion = drv->compl[i].region;
    *addrInRegion = (regAddr & 0x0007ffff);
    return GT_OK;
}

static GT_STATUS prvCpssHwDriverSimAc8Read(
    IN  CPSS_HW_DRIVER_SIMULATION_STC *drv,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_UINTPTR  complRegion;
    GT_U32      addrInRegion;
    GT_U32      data;
    GT_BOOL     locked = GT_FALSE;
    GT_STATUS   rc;

    rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
    if (rc != GT_OK)
        count = 0; /* to skip cycle */
    for (; count; count--,dataPtr++)
    {
        if (addrInRegion >= 0x80000)
        {
            rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
            if (rc != GT_OK)
                break;
        }
        scibReadMemory(drv->simDevId, complRegion+addrInRegion, 1, &data);
        *dataPtr = CPSS_32BIT_LE(data);
        addrInRegion += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);
    return rc;
}

static GT_STATUS prvCpssHwDriverSimAc8WriteMask(
    IN  CPSS_HW_DRIVER_SIMULATION_STC *drv,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_UINTPTR  complRegion;
    GT_U32      addrInRegion;
    GT_U32      data;
    GT_BOOL     locked = GT_FALSE;
    GT_STATUS   rc;

    rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
    if (rc != GT_OK)
        count = 0; /* to skip cycle */
    for (; count; count--,dataPtr++)
    {
        if (addrInRegion >= 0x80000)
        {
            rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
            if (rc != GT_OK)
                break;
        }
        if (mask != 0xffffffff)
        {
            scibReadMemory(drv->simDevId, complRegion+addrInRegion, 1, &data);
            data = CPSS_32BIT_LE(data);
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
        data = CPSS_32BIT_LE(data);
        scibWriteMemory(drv->simDevId, complRegion+addrInRegion, 1, &data);
        addrInRegion += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);
    return rc;
}

static GT_STATUS cpssHwDriverSimulationRead(
    IN  CPSS_HW_DRIVER_SIMULATION_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32      data;

    if (addrSpace == CPSS_HW_DRIVER_AS_SWITCHING_E
        && drv->useAddressCompletion == GT_TRUE)
    {
        return prvCpssHwDriverSimAc8Read(drv, regAddr, dataPtr, count);
    }
    for (; count; count--,dataPtr++)
    {
        switch (addrSpace)
        {
            case CPSS_HW_DRIVER_AS_CNM_E:
                scibPciRegRead(drv->simDevId, regAddr, 1, &data);
                break;
            case CPSS_HW_DRIVER_AS_SWITCHING_E:
                scibReadMemory(drv->simDevId, regAddr, 1, &data);
                break;
            case CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E:
                scibMemoryClientRegRead(drv->simDevId,
                    SCIB_MEM_ACCESS_DFX_E, regAddr, 1, &data);
                break;
            case CPSS_HW_DRIVER_AS_MG1_E:
            case CPSS_HW_DRIVER_AS_MG2_E:
            case CPSS_HW_DRIVER_AS_MG3_E:
                /*TBD need to cmplete sim support
                scibMemoryClientRegRead(
                    SIM_DEV_ID(devNum,0), SCIB_MEM_ACCESS_DFX_E, address + mgNum, 1, &data);*/
                data = 0;
                break;

            default:
                return GT_BAD_PARAM;
        }
        *dataPtr = CPSS_32BIT_LE(data);
        regAddr += 4;
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverSimulationWriteMask(
    IN  CPSS_HW_DRIVER_SIMULATION_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32      data;
    GT_STATUS   rc;

    if (addrSpace == CPSS_HW_DRIVER_AS_SWITCHING_E
        && drv->useAddressCompletion == GT_TRUE)
    {
        return prvCpssHwDriverSimAc8WriteMask(drv, regAddr, dataPtr, count, mask);
    }
    for (; count; count--,dataPtr++)
    {
        if (mask != 0xffffffff)
        {
            rc = cpssHwDriverSimulationRead(drv, addrSpace, regAddr, &data, 1);
            if (rc != GT_OK)
                return rc;
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
        data = CPSS_32BIT_LE(data);
        switch (addrSpace)
        {
            case CPSS_HW_DRIVER_AS_CNM_E:
                scibPciRegWrite(drv->simDevId, regAddr, 1, &data);
                break;
            case CPSS_HW_DRIVER_AS_SWITCHING_E:
                scibWriteMemory(drv->simDevId, regAddr, 1, &data);
                break;
            case CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E:
                scibMemoryClientRegWrite(drv->simDevId,
                    SCIB_MEM_ACCESS_DFX_E, regAddr, 1, &data);
                break;
            case CPSS_HW_DRIVER_AS_MG1_E:
            case CPSS_HW_DRIVER_AS_MG2_E:
            case CPSS_HW_DRIVER_AS_MG3_E:
                /*TBD need to cmplete sim support
                scibMemoryClientRegWrite(
                    SIM_DEV_ID(devNum,0), SCIB_MEM_ACCESS_DFX_E, address + mgNum, 1, &data);*/
                break;
            default:
                return GT_BAD_PARAM;
        }
        regAddr += 4;
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverSimulationDestroy(
    IN  CPSS_HW_DRIVER_SIMULATION_STC *drv
)
{
    if (drv->useAddressCompletion == GT_TRUE)
    {
        cpssOsMutexDelete(drv->mtx);
    }
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverSimulationCreateDrv function
* @endinternal
*
* @brief Create driver instance for simulated PP
*
* @param[in] hwInfo         - HW info
* @param[in] compRegionMask - the bitmap of address completion regions to use
*                             If the bit N is set to 1 then region N can be used
*                             by driver
*                             The value 0xffffffff is used to indicate legacy
*                             4-region address completion
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSimulationCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  GT_U32      compRegionMask
)
{
    CPSS_HW_DRIVER_SIMULATION_STC *drv;
    GT_U32 i;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverSimulationRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverSimulationWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverSimulationDestroy;

    drv->base = hwInfo->resource.switching.start;
    drv->simDevId = scibGetDeviceId(drv->base);
    drv->useAddressCompletion = scibAddressCompletionStatusGet(drv->simDevId);
    if (compRegionMask == 0xffffffff)
    {
        /* legacy PEX driver
         * The address completion currently not supported
         */
        drv->useAddressCompletion = GT_FALSE;
    }
    if (drv->useAddressCompletion == GT_TRUE)
    {
        char buf[64];
        /* Initialize address completion */
        for (i = 1; i < 8; i++)
        {
            if ((compRegionMask & (1<<i)) == 0)
                continue;

            drv->compl[drv->numRegions].reg = drv->base + 0x120 + (i * 4);
            drv->compl[drv->numRegions].region = drv->base + (i * 0x80000);
            drv->numRegions++;
        }
        if (drv->numRegions)
        {
            cpssOsSprintf(buf, "addrCompl%p", (void*)drv->base);
            cpssOsMutexCreate(buf, &(drv->mtx));
        }
    }

    if ((compRegionMask != 0xffffffff) && (drv->useAddressCompletion == GT_TRUE))
    {
        /* set 8-region mode: regAddr = 0x140, set bit16 to 0 */
        i = 0;
        drv->common.writeMask(&(drv->common), CPSS_HW_DRIVER_AS_SWITCHING_E, 0x140, &i, 1, 0x00010000);
    }

    return (CPSS_HW_DRIVER_STC*)drv;
}


