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
* @file userInitCmdDb.h
*
* @brief The "cmdUserInitDatabase()" function placeholder.
* contains the empty function, dedicated to be overwitten
* from [main] make/win32 directory at link build step.
*
* @version   2
********************************************************************************
*/

#ifndef __userInitCmdDbh
#define __userInitCmdDbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal cmdUserInitDatabase function
* @endinternal
*
* @brief   The "cmdUserInitDatabase()" function placeholder.
*         Called by "cmdInit" and "cmdInitDebug" functions.
*         Enables some command shell features .
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdUserInitDatabase
(
	GT_VOID
) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __userInitCmdDbh*/


