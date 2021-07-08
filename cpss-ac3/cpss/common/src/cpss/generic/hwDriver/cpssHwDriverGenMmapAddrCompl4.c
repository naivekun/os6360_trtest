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
* @file cpssHwDriverMmapAddrCompl4.c
*
* @brief generic HW driver which use mmap'ed resource
*        with legacy 4-region address completion
*
* @version   1
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>


typedef struct PRV_CPSS_HW_DRIVER_GEN_MMAP_AC4_STCT {
    CPSS_HW_DRIVER_STC  common;
    GT_UINTPTR          base;
    CPSS_OS_MUTEX       mtx;
    GT_U32              addrComplShadow[4];
    GT_U32              compIdx;
} PRV_CPSS_HW_DRIVER_GEN_MMAP_AC4_STC;


/* resolve address completion */
static GT_STATUS prvGetAc(
    IN  PRV_CPSS_HW_DRIVER_GEN_MMAP_AC4_STC *drv,
    IN  GT_U32      regAddr,
    OUT GT_UINTPTR  *complRegion,
    OUT GT_U32      *addrInRegion,
    INOUT GT_BOOL   *locked
)
{
    GT_U32 compl;
    int i;
    if (regAddr < 0x01000000)
    {
        *complRegion = drv->base;
        *addrInRegion = regAddr;
        return GT_OK;
    }
    if (*locked == GT_FALSE)
    {
        cpssOsMutexLock(drv->mtx);
        *locked = GT_TRUE;
    }
    compl = (regAddr >> 24);
    for (i = 1; i < 4; i++)
    {
        if (drv->addrComplShadow[i] == compl)
        {
            *complRegion = drv->base + (i<<24);
            *addrInRegion = (regAddr & 0x00ffffff);
            return GT_OK;
        }
    }
    i = drv->compIdx;
    drv->compIdx++;
    if (drv->compIdx >= 4)
        drv->compIdx = 1;
    drv->addrComplShadow[i] = compl;
    compl = (drv->addrComplShadow[3]<<24) |
            (drv->addrComplShadow[2]<<16) |
            (drv->addrComplShadow[1]<<8);
    compl = CPSS_32BIT_LE(compl);
    /* write the address completion 3 times.
    because the PP have a 2 entry write buffer
    so, the 3 writes will make sure we do get
    to the hardware register itself */
    *((volatile GT_U32*)(drv->base)) = compl;
    GT_SYNC;
    *((volatile GT_U32*)(drv->base)) = compl;
    GT_SYNC;
    *((volatile GT_U32*)(drv->base)) = compl;
    GT_SYNC;
    *complRegion = drv->base + (i<<24);
    *addrInRegion = (regAddr & 0x00ffffff);
    return GT_OK;
}

static GT_STATUS prvCpssHwDriverGenMmapAc4Read(
    IN  PRV_CPSS_HW_DRIVER_GEN_MMAP_AC4_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
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

#ifdef _VISUALC
    (void)addrSpace;
#endif

    rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
    if (rc != GT_OK)
        count = 0; /* to skip cycle */
    for (; count; count--,dataPtr++)
    {
        if (addrInRegion >= 0x01000000)
        {
            rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
            if (rc != GT_OK)
                break;
        }
        data = *((volatile GT_U32*)(complRegion+addrInRegion));
        GT_SYNC; /* to avoid read combining */
        *dataPtr = CPSS_32BIT_LE(data);
        addrInRegion += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);
    return rc;
}

static GT_STATUS prvCpssHwDriverGenMmapAc4WriteMask(
    IN  PRV_CPSS_HW_DRIVER_GEN_MMAP_AC4_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
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

#ifdef _VISUALC
    (void)addrSpace;
#endif

    rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
    if (rc != GT_OK)
        count = 0; /* to skip cycle */
    for (; count; count--,dataPtr++)
    {
        if (addrInRegion >= 0x01000000)
        {
            rc = prvGetAc(drv, regAddr, &complRegion, &addrInRegion, &locked);
            if (rc != GT_OK)
                break;
        }
        if (mask != 0xffffffff)
        {
            data = CPSS_32BIT_LE(*((volatile GT_U32*)(complRegion+addrInRegion)));
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
        *((volatile GT_U32*)(complRegion+addrInRegion)) = CPSS_32BIT_LE(data);
        GT_SYNC; /* to avoid from write combining */
        addrInRegion += 4;
        regAddr += 4;
    }
    if (locked != GT_FALSE)
        cpssOsMutexUnlock(drv->mtx);
    return rc;
}

static GT_STATUS prvCpssHwDriverGenMmapAc4DestroyDrv(
    IN  PRV_CPSS_HW_DRIVER_GEN_MMAP_AC4_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverGenMmapAc4CreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*        with 4-region address completion
*
* @param[in] base           - resource virtual address
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size must be 64M
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapAc4CreateDrv(
    IN  GT_UINTPTR  base
)
{
    PRV_CPSS_HW_DRIVER_GEN_MMAP_AC4_STC *drv;
    char buf[64];

    if (!base)
        return NULL;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvCpssHwDriverGenMmapAc4Read;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvCpssHwDriverGenMmapAc4WriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvCpssHwDriverGenMmapAc4DestroyDrv;

    drv->base = base;
    drv->compIdx = 1;
    cpssOsSprintf(buf, "addrCompl%p", (void*)base);
    cpssOsMutexCreate(buf, &(drv->mtx));

    /* Reset Hw Address Completion          */
    *(volatile GT_U32 *)(base) = 0;
    *(volatile GT_U32 *)(base) = 0;
    *(volatile GT_U32 *)(base) = 0;
    GT_SYNC;

    return (CPSS_HW_DRIVER_STC*)drv;
}
