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
* @file gtOsSharedFunctionPointers.h
*
* @brief This file contains bind routines for ARM architecture to avoid
* pointer issue. Work-around uses dlsym system call to avoid
* private addresses to functions.
*
* @version   1
********************************************************************************
*/
#ifndef __gtOsSharedFunctionPointersh
#define __gtOsSharedFunctionPointersh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Functions ******************************************************/

/**
* @internal shrMemGetDefaultOsBindFuncsThruDynamicLoader function
* @endinternal
*
* @brief   Receives default cpss bind from OS
*
* @param[out] osFuncBindPtr            - (pointer to) set of call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Uses dlsym system call to overcome private-pointer-to-func issue
*       observed for ARM GnuEABI.
*
*/
GT_STATUS shrMemGetDefaultOsBindFuncsThruDynamicLoader(
    OUT CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
);

/**
* @internal shrMemGetDefaultExtDrvFuncs function
* @endinternal
*
* @brief   Receives default cpss bind from extDrv
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*/
GT_STATUS shrMemGetDefaultExtDrvFuncs
(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __gtOsSharedFunctionPointersh */


