/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <gtExtDrv/drivers/gtCacheMng.h>
/*#include <cacheLib.h>*/
#include <gtExtDrv/drivers/pssBspApis.h>


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
	bspCacheType_ENT bspCacheType;
   
	switch (type)
	{
	case GT_MGMT_INSTRUCTION_CACHE_E:
		bspCacheType = bspCacheType_InstructionCache_E;      
		break;

	case GT_MGMT_DATA_CACHE_E:
		bspCacheType = bspCacheType_DataCache_E;      
		break;
	  
	default:
		return GT_BAD_PARAM; 
	}

	return bspCacheFlush(bspCacheType, address_PTR, size);
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
	bspCacheType_ENT bspCacheType;
   
	switch (type)
	{
	case GT_MGMT_INSTRUCTION_CACHE_E:
		bspCacheType = bspCacheType_InstructionCache_E;      
		break;
   
	case GT_MGMT_DATA_CACHE_E:
		bspCacheType = bspCacheType_DataCache_E;      
		break;
      
	default:
		return GT_BAD_PARAM; 
	}
   
	return bspCacheInvalidate(bspCacheType, address_PTR, size);
}



