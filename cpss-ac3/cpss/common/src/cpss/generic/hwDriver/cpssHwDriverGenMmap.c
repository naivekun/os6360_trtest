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
* @file cpssHwDriverGenMmap.c
*
* @brief generic HW driver which use mmap'ed resource
*
* @version   1
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>

typedef struct CPSS_HW_DRIVER_GEN_MMAP_STCT {
    CPSS_HW_DRIVER_STC  common;
    GT_UINTPTR          base;
    GT_UINTPTR          size;
} CPSS_HW_DRIVER_GEN_MMAP_STC;

static GT_STATUS cpssHwDriverGenMmapRead(
    IN  CPSS_HW_DRIVER_GEN_MMAP_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32      data;

#ifdef _VISUALC
    (void)addrSpace;
#endif

    for (;count;count--,regAddr+=4,dataPtr++)
    {
        if (regAddr+3 >= drv->size)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        data = *((volatile GT_U32*)(drv->base+regAddr));
        GT_SYNC; /* to avoid read combining */
        *dataPtr = CPSS_32BIT_LE(data);
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverGenMmapWriteMask(
    IN  CPSS_HW_DRIVER_GEN_MMAP_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32      data;

#ifdef _VISUALC
    (void)addrSpace;
#endif

    for (;count;count--,regAddr+=4,dataPtr++)
    {
        if (regAddr+3 >= drv->size)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        if (mask != 0xffffffff)
        {
            data = CPSS_32BIT_LE(*((volatile GT_U32*)(drv->base+regAddr)));
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
        *((volatile GT_U32*)(drv->base+regAddr)) = CPSS_32BIT_LE(data);
        GT_SYNC; /* to avoid from write combining */
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverGenMmapDestroyDrv(
    IN  CPSS_HW_DRIVER_GEN_MMAP_STC *drv
)
{
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverGenMmapCreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*
* @param[in] base   - resource virtual address
* @param[in] size   - mapped resource size
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapCreateDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size
)
{
    CPSS_HW_DRIVER_GEN_MMAP_STC *drv;

    if (!base || size < 4)
        return NULL;
    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverGenMmapRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverGenMmapWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverGenMmapDestroyDrv;

    drv->base = base;
    drv->size = size;
    return (CPSS_HW_DRIVER_STC*)drv;
}

