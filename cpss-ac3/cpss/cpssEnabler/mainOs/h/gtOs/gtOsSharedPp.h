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
* @file gtOsSharedPp.h
*
* @brief File contains routines for Packet Processor Initialization
* (applicable ONLY for BM).
* Also routine for sections mapping registry is implemented here.
*
* @version   2
********************************************************************************
*/
#ifndef __gtOsSharedPph
#define __gtOsSharedPph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************* Includes *******************************************************/

#include <sys/mman.h>
#include <gtOs/gtGenTypes.h>

#define PP_REGISTRY_SIZE_CNS 64

/************* Typedefs ********************************************************/
/*
 * Typedef: struct SECTIONS_STCT
 *
 * Description:
 *      This structure presents 
 *
 * Fields:i
 *      startPtr    - start address to be mapped
 *      length      - length of block to be mapped
 *      offset      - offset into file to be mapped
 *
 *******************************************************************************/
typedef struct PP_MMAP_INFO_STCT
{
    GT_VOID	*startPtr;
    size_t	length;
    off_t   offset;
} PP_MMAP_INFO_STC;

/************* Functions ******************************************************/

/**
* @internal shrMemRegisterPpMmapSection function
* @endinternal
*
* @brief   Registers section which should be mmaped by all non-first clients.
*
* @param[in] startPtr                 - start address to be mapped
* @param[in] length                   -  of block to be mapped
* @param[in] offset                   -  into file to be mapped
*                                       GT_OK on success
*
* @retval GT_OUT_OF_RANGE          - too many sections for static registry,
*                                       must be increased
*
* @note Operation is relevant only for SHARED_MEMORY=1
*
*/
GT_STATUS shrMemRegisterPpMmapSection
(
    IN GT_VOID  *startPtr,
    IN size_t   length,
    IN off_t    offset
);

/**
* @internal shrMemSharedPpInit function
* @endinternal
*
* @brief   Performs sharing operation for DMA region.
*
* @note Operation is relevant only for SHARED_MEMORY=1
*
*/
GT_STATUS shrMemSharedPpInit(IN GT_BOOL isFirstCLient);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __gtOsSharedPph */


