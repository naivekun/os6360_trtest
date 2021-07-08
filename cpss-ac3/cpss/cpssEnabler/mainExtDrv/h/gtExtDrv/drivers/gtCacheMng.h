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
* @file gtCacheMng.h
*
* @brief Enable managment of cache memory
*
* @version   1.1.2.1
********************************************************************************
*/


#ifndef __gtCacheMngh
#define __gtCacheMngh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
 
#include <gtExtDrv/os/extDrvOs.h>
#include <stddef.h>

/**
* @enum GT_MGMT_CACHE_TYPE_ENT
 *
 * @brief This type defines used cache types
*/
typedef enum{

    /** cache of commands */
    GT_MGMT_INSTRUCTION_CACHE_E,

    /** @brief cache of data
     *  Note:
     *  Don't modify the order or values of this enum.
     */
    GT_MGMT_DATA_CACHE_E

} GT_MGMT_CACHE_TYPE_ENT;

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
* @retval GT_FAIL                  - othersise.
*/

GT_STATUS extDrvMgmtCacheFlush
(
    IN GT_MGMT_CACHE_TYPE_ENT   type, 
    IN void                     *address_PTR, 
    IN size_t                   size
);

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
* @retval GT_FAIL                  - othersise.
*/

GT_STATUS extDrvMgmtCacheInvalidate 
(
    IN GT_MGMT_CACHE_TYPE_ENT   type, 
    IN void                     *address_PTR, 
    IN size_t                   size
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



