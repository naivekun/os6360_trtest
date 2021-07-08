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
* @file os_w32IO.c
*
* @brief win32 extended system wrapper layer
*
* this module implements pipe io and standard io redirection routines
*
* @version   4
********************************************************************************
*/

/***** Include files ***************************************************/

/* WA to avoid next warning :
   due to include to : #include <windows.h>
    c:\program files\microsoft visual studio\vc98\include\rpcasync.h(45) :
    warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
*/
struct _RPC_ASYNC_STATE;

#include <cmdShell/os/cmdExecute.h>
#include <windows.h>
#include <stdio.h>
#include <gtOs/gtGenTypes.h>

typedef GT_STATUS (*SHELL_FUNC)(char* command);
/* global, defined cmdConsole.c */
extern SHELL_FUNC osSlellFunction;


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
*
* @note NOT IMPLEMENTED (WIN32)
*
*/
GT_STATUS   osShellExecute
(
    IN  char*   command
)
{
    if (osSlellFunction != NULL)
    {
        return osSlellFunction(command);
    }
    return GT_FAIL;
}

/*******************************************************************************
* osShellPrintf
*
* description:
*       Write a formatted string to the shell's output stream.
*
* INPUTS:
*       format  - format string to write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The number of characters written, or a negative value if
*       an output error occurs.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int osShellPrintf(const char* format, ...)
{
    char buff[2048];
    va_list args;
    int i;

    va_start(args, format);
    i = vsprintf(buff, format, args);
    va_end(args);

	return cmdOsPrintf("%s", buff);
}




