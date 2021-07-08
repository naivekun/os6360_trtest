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
* @file gtOsInit.h
*
* @brief File contains entry point routines
*
* @version   1
********************************************************************************
*/
#ifndef __gtOsInith
#define __gtOsInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Functions ******************************************************/

/**
* @internal osStartEngine function
* @endinternal
*
* @brief   Initialize mainOs engine, then start entryPoint routine
*         Need to be inplemented for Linux/FreeBSD
* @param[in] argC                     - main() parameter
* @param[in] argV[]                   - main() parameter
* @param[in] name                     - task name
* @param[in] entryPoint               - entry point function
*                                       GT_OK on success
*/
GT_STATUS osStartEngine
(
    IN int argC,
    IN const char* argV[],
    IN const char* name,
    IN GT_VOIDFUNCPTR entryPoint
);

/**
* @internal osStopEngine function
* @endinternal
*
* @brief   Stop engine
*         Need to be inplemented for Linux/FreeBSD
*/
GT_STATUS osStopEngine(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __gtOsInith */


