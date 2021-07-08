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
* @file cmdStream.c
*
* @brief This file contains common functions for mainCmd I/O streams
*
* @version   9
********************************************************************************
*/
#include <cmdShell/os/cmdStreamImpl.h>

/*******************************************************************************
* cmdStreamGrabSystemOutput
*
* DESCRIPTION:
*       cmdStreamRedirectStdout() will grab stdout and stderr streams
*       if this flags set to GT_TRUE
*
*******************************************************************************/
GT_BOOL cmdStreamGrabSystemOutput = GT_TRUE;

/**
* @internal cmdStreamRedirectStdout function
* @endinternal
*
* @brief   Redirect stdout/stderr to given stream
*
* @param[in] stream                   - destination stream
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cmdStreamRedirectStdout
(
    IN IOStreamPTR stream
)
{
    if (!stream)
    {
        /* dup2... */
        return cmdOsBindStdOut(NULL, NULL);
    }
    if (cmdStreamGrabSystemOutput == GT_FALSE)
        return GT_OK;
    if (stream->grabStd)
        stream->grabStd(stream);
    return cmdOsBindStdOut((CPSS_OS_BIND_STDOUT_FUNC_PTR)stream->writeBuf, stream);
}

/************* NULL stream implementation *************************************/


/**
* @internal cmdStreamStartEventLoopHandler function
* @endinternal
*
* @brief   Create new task to run stream handler
*
* @param[in] taskName                 - task name for handler
* @param[in] handler                  - pointer to  function
* @param[in] stream                   - stream
* @param[in] isConsole                - application flag
*
* @param[out] taskId                   - pointer for task ID
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cmdStreamStartEventLoopHandler
(
    IN  char *taskName,
    IN  eventLoopHandlerPTR handler,
    IN  IOStreamPTR stream,
    IN  GT_BOOL isConsole,
    OUT GT_TASK *taskId
)
{
    if (!stream)
        return GT_FAIL;

    stream->isConsole = isConsole;

    return cmdOsTaskCreate(
                taskName,               /* thread name              */
                STREAM_THREAD_PRIO+1,   /* thread priority          */
                65536,                  /* use default stack size   */
                (unsigned (__TASKCONV *)(void*))handler,
                stream,
                taskId);
}


