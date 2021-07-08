/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#ifndef __cmdExecuteh
#define __cmdExecuteh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmdShell/common/cmdCommon.h>

#define OS_DEF_BUFFSIZE 4096    /* default buffer size  */

/**
* @internal osShellGetFunctionByName function
* @endinternal
*
* @brief   Lookup executable file symbol table for function
*
* @param[in] funcName                 - null terminated command string
*
* @retval NULL                     - if function is not found
*                                       function pointer
*/
GT_VOIDFUNCPTR osShellGetFunctionByName
(
    IN  const char* funcName
);

/**
* @internal osShellExecute function
* @endinternal
*
* @brief   execute command through OS shell
*
* @param[in] command                  - null terminated  string
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on timeout
*/
GT_STATUS   osShellExecute
(
    IN  char*   command
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ifndef __cmdExecuteh */


