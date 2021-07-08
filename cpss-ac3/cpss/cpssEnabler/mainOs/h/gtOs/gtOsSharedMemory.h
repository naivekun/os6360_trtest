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
* @file gtOsSharedMemory.h
*
* @brief This file contains init routines for Shared Memory approach.
* Intt functions should be called BEFORE any other work with
* with CPSS Shared Library.
*
* @version   1.4
********************************************************************************
*/
#ifndef __gtOsSharedMemoryh
#define __gtOsSharedMemoryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Functions ******************************************************/
/**
* @internal shrMemInitSharedLibrary function
* @endinternal
*
* @brief   Initialize shared library for processes
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*/
GT_STATUS shrMemInitSharedLibrary
(
    GT_VOID
);

/**
* @internal shrMemInitSharedLibrary_FirstClient function
* @endinternal
*
* @brief   Initialize shared library for first process
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*/
GT_STATUS shrMemInitSharedLibrary_FirstClient
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __gtOsSharedMemoryh */


