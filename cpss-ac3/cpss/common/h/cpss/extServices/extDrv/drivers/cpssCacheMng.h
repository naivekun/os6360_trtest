/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file cpssCacheMng.h
*
* @brief Enable management of cache memory
*
*
* @version   1
********************************************************************************
*/


#ifndef __cpssCacheMngh
#define __cpssCacheMngh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/**
* @enum CPSS_MGMT_CACHE_TYPE_ENT
 *
 * @brief This type defines used cache types
*/
typedef enum{

    /** cache of commands */
    CPSS_MGMT_INSTRUCTION_CACHE_E,

    /** @brief cache of data
     *  Note:
     *  Don't modify the order or values of this enum.
     */
    CPSS_MGMT_DATA_CACHE_E

} CPSS_MGMT_CACHE_TYPE_ENT;

/*******************************************************************************
* CPSS_EXT_DRV_MGMT_CACHE_FLUSH_FUNC
*
* DESCRIPTION:
*       Flush to RAM content of cache
*
* INPUTS:
*       type        - type of cache memory data/instruction
*       address_PTR - starting address of memory block to flush
*       size        - size of memory block
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_EXT_DRV_MGMT_CACHE_FLUSH_FUNC)
(
    IN CPSS_MGMT_CACHE_TYPE_ENT   type,
    IN void                     *address_PTR,
    IN size_t                   size
);

/*******************************************************************************
* CPSS_EXT_DRV_MGMT_CACHE_INVALIDATE_FUNC
*
* DESCRIPTION:
*       Invalidate current content of cache
*
* INPUTS:
*       type        - type of cache memory data/instruction
*       address_PTR - starting address of memory block to flush
*       size        - size of memory block
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_EXT_DRV_MGMT_CACHE_INVALIDATE_FUNC)
(
    IN CPSS_MGMT_CACHE_TYPE_ENT type,
    IN void                     *address_PTR,
    IN size_t                   size
);

/* CPSS_EXT_DRV_MGMT_CACHE_STC -
    structure that hold the "external driver cache manage" functions needed be
    bound to cpss.
*/
typedef struct{
    CPSS_EXT_DRV_MGMT_CACHE_FLUSH_FUNC       extDrvMgmtCacheFlush;
    CPSS_EXT_DRV_MGMT_CACHE_INVALIDATE_FUNC  extDrvMgmtCacheInvalidate;
}CPSS_EXT_DRV_MGMT_CACHE_STC;



#ifdef __cplusplus
}
#endif /* __cpssCacheMngh */

#endif


