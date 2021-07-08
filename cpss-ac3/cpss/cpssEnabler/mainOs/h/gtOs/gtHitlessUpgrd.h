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
* @file gtHitlessUpgrd.h
*
* @brief Definitions for hitless SW upgrade.
*
* @version   2
********************************************************************************
*/

#ifndef __gtOsHitlessUpgrd_h
#define __gtOsHitlessUpgrd_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtOsMem.h>

/**
* @internal osHitlessUpgradeDetach function
* @endinternal
*
* @brief   Detaches the mainOs lib
*
* @retval GT_OK                    - operation succeeded
* @retval GT_FAIL                  - operation failed
*/
GT_STATUS osHitlessUpgradeDetach (void);


/**
* @internal osHitlessUpgradeAttach function
* @endinternal
*
* @brief   Attaches the mainOs lib to the stored system state.
*
* @param[in] memAllocFuncPtr          - the function to be used in memory allocation
*                                      (refer to osMemLibInit)
*
* @retval GT_OK                    - operation succeeded
* @retval GT_FAIL                  - operation failed
*/
GT_STATUS osHitlessUpgradeAttach
(    
    IN   GT_MEMORY_ALLOC_FUNC    memAllocFuncPtr
);

/**
* @internal osMemBindMemAllocFunc function
* @endinternal
*
* @brief   Binds the the memory allocation function
*
* @param[in] memAllocFuncPtr          - the function to be used in memory allocation
*                                      (refer to osMemLibInit)
*
* @retval GT_OK                    - operation succeeded
* @retval GT_FAIL                  - operation failed
*/
GT_STATUS osMemBindMemAllocFunc
(    
    IN   GT_MEMORY_ALLOC_FUNC    memAllocFuncPtr
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtOsHitlessUpgrd_h */








