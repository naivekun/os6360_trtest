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
* @file cmdConsole.h
*
* @brief commander console api header
*
*
* @version   8
********************************************************************************
*/

#ifndef __cmdConsole_h__
#define __cmdConsole_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/os/cmdStreamImpl.h>

/**
* @internal cmdTakeGlobalStdio function
* @endinternal
*
* @brief   Set the global stdio streams. No task can set unless the one took has
*         released it (Critical Section).
*
* @retval GT_OK                    - on success
*
* @note Each console uses the stdio must set the global stdio stream in order to
*       redirect all other task outputs into the current stream
*
*/
GT_STATUS cmdTakeGlobalStdio(IOStreamPTR IOStream);

/**
* @internal cmdReleaseGlobalStdio function
* @endinternal
*
* @brief   Release the global stdio for other taks used.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cmdReleaseGlobalStdio(GT_VOID);

/**
* @internal cmdPrintf function
* @endinternal
*
* @brief   Write a formatted string to the current I/O stream.
*/
int cmdPrintf(const char* format, ...);

/**
* @internal cmdGets function
* @endinternal
*
* @brief   Read line from current stream.
*         Returned line is zero terminated with stripped EOL symbols
*         Zero termination is added if there is a space in a buffer.
*         Otherwise returned value should be used as line length
*/
int cmdGets(char* bufferPtr, int bufferLen);

/**
* @internal cmdEventLoop function
* @endinternal
*
* @brief   command line shell pipeline; should be spawned as an independent thread
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdEventLoop
(
    IOStreamPTR IOStream
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdConsole_h__ */



