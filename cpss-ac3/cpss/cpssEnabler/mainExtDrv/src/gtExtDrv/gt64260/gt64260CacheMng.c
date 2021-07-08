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
* @file gt64260CacheMng.c
*
* @brief Includes cache management functions wrappers implementation for the
* GT-64260 system controller.
*
* @version   1.1.2.1
********************************************************************************
*/

#include <gtExtDrv/drivers/gtCacheMng.h>
#include <cacheLib.h>

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
  if (cacheFlush((CACHE_TYPE)type, address_PTR, size) == OK)
      return (GT_OK);
  else
      return (GT_FAIL);
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
  if (cacheInvalidate((CACHE_TYPE)type, address_PTR, size) == OK)
      return (GT_OK);
  else
      return (GT_FAIL);
}



