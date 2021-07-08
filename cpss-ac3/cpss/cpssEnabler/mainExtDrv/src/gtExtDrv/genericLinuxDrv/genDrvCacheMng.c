/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file genDrvCacheMng.c
*
* @brief Includes cache management functions wrappers implementation.
*
* @version   3
********************************************************************************
*/
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "kerneldrv/include/presteraGlob.h"
/* ?? */
#define PAGESIZE 4096

extern GT_32 gtPpFd;
/**
* @internal extDrvMgmtCacheFlush function
* @endinternal
*
* @brief   Flush to RAM content of cache
*
* @param[in] type                     -  of cache memory data/intraction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvMgmtCacheFlush
(
    IN GT_MGMT_CACHE_TYPE_ENT   type, 
    IN void                     *address_PTR, 
    IN size_t                   size
)
{
    unsigned long address;
    struct GT_RANGE_STC range;
    /* page size is a power of 2 */
    /* align address to page boundery */
    address = ((unsigned long) address_PTR) & ~(PAGESIZE - 1);
    /* increase size if needed */
    size += ((unsigned long) address_PTR) - address;

#if 1
    range.address = address;
    range.length = size;
    
    /* call driver function */
    if (ioctl(gtPpFd, PRESTERA_IOC_FLUSH, &range) < 0)
    {
        return GT_FAIL;
    }
    return GT_OK;
#else    
    if (msync((void *)address, size, MS_SYNC))
    {
        return GT_FAIL;
    }
    else
    {
        return GT_FAIL;
    }
#endif
}


/**
* @internal extDrvMgmtCacheInvalidate function
* @endinternal
*
* @brief   Invalidate current content of cache
*
* @param[in] type                     -  of cache memory data/intraction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvMgmtCacheInvalidate 
(
    IN GT_MGMT_CACHE_TYPE_ENT   type, 
    IN void                     *address_PTR, 
    IN size_t                   size
)
{
    unsigned long address;
    struct GT_RANGE_STC range;
    
    /* page size is a power of 2 */
    /* align address to page boundery */
    address = ((unsigned long) address_PTR) & ~(PAGESIZE - 1);
    /* increase size if needed */
    size += ((unsigned long) address_PTR) - address;
    
#if 1
    range.address = address;
    range.length = size;
    
    /* call driver function */
    if (ioctl(gtPpFd, PRESTERA_IOC_INVALIDATE, &range) < 0)
    {
        return GT_FAIL;
    }
    return GT_OK;
#else    
    if (msync((void *)address, size, MS_INVALIDATE))
    {
        return GT_FAIL;
    }
    else
    {
        return GT_FAIL;
    }
#endif
}

/**
* @internal extDrvGetDmaBase function
* @endinternal
*
* @brief   Get the base address of the DMA area need for Virt2Phys and Phys2Virt
*         translaton
*
* @param[out] dmaBase                  - he base address of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaBase
(
	OUT GT_UINTPTR * dmaBase
)
{
    mv_phys_addr_t base;
    if (ioctl(gtPpFd, PRESTERA_IOC_GETBASEADDR, &base) < 0)
    {
        return GT_FAIL;
    }
    *dmaBase = (GT_UINTPTR)base;
    return GT_OK;
}

/**
* @internal extDrvGetDmaVirtBase function
* @endinternal
*
* @brief   Get the base address of the DMA area in userspace
*
* @param[out] dmaVirtBase              - the base address of the DMA area in userspace.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaVirtBase
(
	OUT GT_UINTPTR *dmaVirtBase
)
{
    struct GT_PCI_VMA_ADDRESSES_STC vmConfig;

    if (ioctl(gtPpFd, PRESTERA_IOC_GETVMA, &vmConfig) == 0)
    {
        *dmaVirtBase = (GT_UINTPTR)vmConfig.dmaBase;
    }
    else
    {
        *dmaVirtBase = LINUX_VMA_DMABASE;
    }
    return GT_OK;
}

/**
* @internal extDrvGetDmaSize function
* @endinternal
*
* @brief   Get the size of the DMA area
*
* @param[out] dmaSize                  - The size of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
extern GT_STATUS extDrvGetDmaSize
(
	OUT GT_U32 * dmaSize
)
{
    mv_kmod_size_t size;
    if (ioctl(gtPpFd, PRESTERA_IOC_GETDMASIZE, &size) < 0)
    {
        return GT_FAIL;
    }
    *dmaSize = (GT_U32)size;
    return GT_OK;
}


