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
* @file cpssHwDriverMmapAddrCompl8.c
*
* @brief generic HW driver which use mmap'ed resource
*        with 8-region address completion
*
* @version   1
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>



typedef struct CPSS_HW_DRIVER_GEN_MMAP_AC8_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_UINTPTR          base;
    GT_UINTPTR          size;
    struct {
        GT_U32          val;
        GT_UINTPTR      reg;
        GT_UINTPTR      region;
    } compl[7]; /* completion for regions 1..7 */
    int numRegions;
    int nextIndex;
} CPSS_HW_DRIVER_GEN_MMAP_AC8_STC;


/* resolve address completion */
static GT_STATUS prvGetAc(
    IN  CPSS_HW_DRIVER_GEN_MMAP_AC8_STC *drv,
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
    *((volatile GT_U32*)(drv->compl[i].reg)) = CPSS_32BIT_LE(compl);
    GT_SYNC; /* to avoid from write combining */
    drv->compl[i].val = compl;
    *complRegion = drv->compl[i].region;
    *addrInRegion = (regAddr & 0x0007ffff);
    return GT_OK;
}

static GT_STATUS cpssHwDriverGenMmapAc8Read(
    IN  CPSS_HW_DRIVER_GEN_MMAP_AC8_STC *drv,
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
        if (addrInRegion >= 0x80000)
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

static GT_STATUS cpssHwDriverGenMmapAc8WriteMask(
    IN  CPSS_HW_DRIVER_GEN_MMAP_AC8_STC *drv,
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
        if (addrInRegion >= 0x80000)
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

static GT_STATUS cpssHwDriverGenMmapAc8DestroyDrv(
    IN  CPSS_HW_DRIVER_GEN_MMAP_AC8_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverGenMmapAc8CreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*        with 8-region address completion
*
* @param[in] base           - resource virtual address
* @param[in] size           - mapped resource size
* @param[in] compRegionMask - the bitmap of address completion regions to use
*                             If the bit N is set to 1 then region N can be used
*                             by driver
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size must be >= 0x80000
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapAc8CreateDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size,
    IN  GT_U32      compRegionMask
)
{
    CPSS_HW_DRIVER_GEN_MMAP_AC8_STC *drv;
    char buf[64];
    GT_U32 i;

    if (!base)
        return NULL;
    if (size < 0x80000)
        return NULL;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverGenMmapAc8Read;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverGenMmapAc8WriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverGenMmapAc8DestroyDrv;

    drv->base = base;
    drv->size = size;
    for (i = 1; i < 8; i++)
    {
        if ((compRegionMask & (1<<i)) == 0)
            continue;

        if (i*0x80000+0x7ffff >= size)
            continue; /* region don't fit to mmapped area */

        drv->compl[drv->numRegions].reg = base + 0x120 + (i * 4);
        drv->compl[drv->numRegions].region = base + (i * 0x80000);
        drv->numRegions++;
    }
    if (drv->numRegions)
    {
        cpssOsSprintf(buf, "addrCompl%p", (void*)base);
        cpssOsMutexCreate(buf, &(drv->mtx));
    }

    return (CPSS_HW_DRIVER_STC*)drv;
}


