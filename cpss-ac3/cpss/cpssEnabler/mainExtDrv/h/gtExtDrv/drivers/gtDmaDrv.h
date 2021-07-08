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
* @file gtDmaDrv.h
*
* @brief Includes DMA functions wrappers.
*
* @version   4
********************************************************************************
*/
#ifndef __gtDmaDrvh
#define __gtDmaDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/os/extDrvOs.h>


/**
* @internal extDrvDmaWrite function
* @endinternal
*
* @brief   Write a given buffer to the given address using the Dma.
*
* @param[in] address                  - The destination  to write to.
* @param[in] buffer                   - The  to be written.
* @param[in] length                   - Length of buffer in words.
* @param[in] burstLimit               - Number of words to be written on each burst.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note 1. The given buffer is allways 4 bytes aligned, any further allignment
*       requirements should be handled internally by this function.
*       2. The given buffer may be allocated from an uncached memory space, and
*       it's to the function to handle the cache flushing.
*       3. The Prestera Driver assumes that the implementation of the DMA is
*       blocking, otherwise the Driver functionality might be damaged.
*
*/
GT_STATUS extDrvDmaWrite
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      *buffer,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit
);

/**
* @internal extDrvDmaRead function
* @endinternal
*
* @brief   Read a memory block from a given address.
*
* @param[in] address                  - The  to read from.
* @param[in] length                   - Length of the memory block to read (in words).
* @param[in] burstLimit               - Number of words to be read on each burst.
*
* @param[out] buffer                   - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note 1. The given buffer is allways 4 bytes aligned, any further allignment
*       requirements should be handled internally by this function.
*       2. The given buffer may be allocated from an uncached memory space, and
*       it's to the function to handle the cache flushing.
*       3. The Prestera Driver assumes that the implementation of the DMA is
*       blocking, otherwise the Driver functionality might be damaged.
*
*/
GT_STATUS extDrvDmaRead
(
    IN  GT_UINTPTR  address,
    IN  GT_U32      length,
    IN  GT_U32      burstLimit,
    OUT GT_U32      *buffer
);

/**
* @internal extDrvCacheDmaMalloc function
* @endinternal
*
* @brief   Allocate a cache-safe buffer of specified size for DMA devices
*         and drivers
* @param[in] size                     - bytes to allocate
*
* @param[out] ptr                      - pointer to allocated memory
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvCacheDmaMalloc
(
    IN  GT_U32 size,
    OUT void **ptr
);

/**
* @internal extDrvCacheDmaFree function
* @endinternal
*
* @brief   Free the buffer acquired with extDrvCacheDmaMalloc
*
* @param[in] ptr                      - pointer to malloc/free buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvCacheDmaFree
(
    IN void *ptr
);

/**
* @internal extDrvPhy2Virt function
* @endinternal
*
* @brief   Converts physical address to virtual.
*
* @param[in] phyAddr                  - physical address
*
* @param[out] virtAddr                 - virtual address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvPhy2Virt
(
    IN  GT_UINTPTR phyAddr,
    OUT GT_UINTPTR *virtAddr
);

/**
* @internal extDrvVirt2Phy function
* @endinternal
*
* @brief   Converts virtual address to physical.
*
* @param[in] virtAddr                 - virtual address
*
* @param[out] phyAddr                  - physical address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvVirt2Phy
(
    IN  GT_UINTPTR virtAddr,
    OUT GT_UINTPTR *phyAddr
);

GT_STATUS extDrvGetDmaBase(OUT GT_UINTPTR * dmaBase);
GT_STATUS extDrvGetDmaSize(OUT GT_U32 * dmaSize);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtDmaDrvh */




