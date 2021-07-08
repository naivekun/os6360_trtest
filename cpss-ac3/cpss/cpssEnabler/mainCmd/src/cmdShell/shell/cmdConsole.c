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
* @file cmdConsole.c
*
* @brief commander shell pipeline implementation
*
*
* @version   26
********************************************************************************
*/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/os/cmdExecute.h>
#include <cmdShell/os/cmdShell.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/shell/cmdConsole.h>
#include <cmdShell/shell/cmdEngine.h>
#include <cmdShell/shell/cmdMain.h>
#include <cmdShell/shell/cmdSystem.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cmdShell/os/cmdOs.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>




/***** Defines  ********************************************************/
/* generic system replies and messages */
#define CMD_PROMPT  "->"
#define CMD_FIELD_PROMPT "::"
#define CMD_ERROR   "syntax error!"
#define CMD_BANNER  "***************************************\n"     \
                    " Prestera commander shell server ready \n"     \
                    "***************************************\n\n"

#define IS_ECHO_OFF_CMD(s) ( cmdOsStrCmp((s), "echo off") == 0 )
#define IS_ECHO_ON_CMD(s) ( cmdOsStrCmp((s), "echo on") == 0 )

/* holds locking semaphore */
extern GT_MUTEX  gStdioMtx;

IOStreamPTR defaultStdio = NULL;

/*Holds the current stdio stream*/
static IOStreamPTR currentStream;

static GT_U32  notExitFlag = 1;


/***** Externs *********************************************************/
#ifdef  CMD_LUA_CLI
GT_STATUS luaCLI_EventLoop(IOStreamPTR IOStream);
#endif

/***** Global macros ***************************************************/

/***** Private Types ***************************************************/

/***** Private Types ***************************************************/


/***** Private Functions ***********************************************/

/***** Public Functions ************************************************/

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
GT_STATUS cmdTakeGlobalStdio(IOStreamPTR IOStream)
{
    GT_STATUS rc; /*return code*/

    rc = cmdOsMutexLock(gStdioMtx);

    /* Set the current stream*/
    if (rc == GT_OK)
    {
        currentStream = IOStream;
        if (currentStream != defaultStdio)
            cmdStreamRedirectStdout(IOStream);
    }
    return rc;
}

/**
* @internal cmdReleaseGlobalStdio function
* @endinternal
*
* @brief   Release the global stdio for other taks used.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cmdReleaseGlobalStdio(GT_VOID)
{
    GT_STATUS rc; /*return code*/

    if (currentStream != defaultStdio)
    {
        currentStream = defaultStdio;
        cmdStreamRedirectStdout(defaultStdio);
    }
    rc = cmdOsMutexUnlock(gStdioMtx);

    return rc;
}

/***** Public Functions ************************************************/

/**
* @internal cmdPrintf function
* @endinternal
*
* @brief   Write a formatted string to the current I/O stream.
*/
int cmdPrintf(const GT_CHAR* format, ...)
{
    GT_CHAR buff[2048];
    va_list args;
    int i;

    if (!currentStream)
        return 0;
    va_start(args, format);
    i = vsprintf(buff, format, args);
    va_end(args);
    if (i <= 0)
        return i;

    return currentStream->writeBuf(currentStream, buff, i);
}

/**
* @internal cmdGets function
* @endinternal
*
* @brief   Read line from current stream.
*         Returned line is zero terminated with stripped EOL symbols
*         Zero termination is added if there is a space in a buffer.
*         Otherwise returned value should be used as line length
*/
int cmdGets(GT_CHAR* bufferPtr, int bufferLen)
{
    if (!currentStream)
        return 0;
    if (!currentStream->connected(currentStream))
        return (-1); /* EOF */
    return currentStream->readLine(currentStream, bufferPtr, bufferLen);
}

/**
* @internal cmdEventLoop function
* @endinternal
*
* @brief   command line shell pipeline; should be spawned as an independend thread
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdEventLoop
(
    IOStreamPTR IOStream
)
{
    GT_CHAR inputBuffer[4 * CMD_MAX_ARGS] = {0};
    GT_CHAR fieldBuffer[8 * CMD_MAX_FIELDS] = {0};

    GT_CHAR *outputBuffer;
    CMD_STATUS status;          /* temporary error status */

#ifdef  CMD_LUA_CLI
    if (((IOStream->flags & IO_STREAM_FLAG_C_FIRSTSHELL) != 0)
        && (cmdDefaultShellType == CMD_STARTUP_SHELL_LUACLI_E))
    {
        cmdOsStrCpy(inputBuffer, "luaCLI");
    }
#endif
    
    if (IOStream->isConsole)
    {
#if 0
        IOStream->ioctl(IOStream, LINE_EDIT_ENABLE);
#endif
        cmdTakeGlobalStdio(IOStream);
        cmdAppShowBoardsList();
        /* optionally print banner text */
#ifdef CMD_BANNER
        IOStream->writeLine(IOStream, CMD_BANNER);
#endif
        cmdReleaseGlobalStdio();
    }

    if (IOStream->isConsole
#ifdef CMD_LUA_CLI
            && (inputBuffer[0] == 0)
#endif
            )
        IOStream->writeLine(IOStream, CMD_PROMPT);

    /* accept and handle input as long as connected */
    while (IOStream->connected(IOStream) && (notExitFlag))
    {
#ifdef  CMD_LUA_CLI
        if ((IOStream->flags & IO_STREAM_FLAG_C_FIRSTSHELL) && 
            (inputBuffer[0] != 0))
        {
            /* first shell and luaCLI */
        }
        else
#endif
        if (IOStream->readLine(IOStream, inputBuffer, sizeof(inputBuffer)) < 0)
        {
            /* input faled, possible connection failed */
            break;
        }
        IOStream->flags &= ~IO_STREAM_FLAG_C_FIRSTSHELL;
#ifdef  CMD_LUA_CLI
        if (cmdOsStrCmp(inputBuffer, "luaCLI") == 0)
        {
            luaCLI_EventLoop(IOStream);
            if (IOStream->isConsole)
                IOStream->writeLine(IOStream, CMD_PROMPT);
            IOStream->customPtr = NULL;
            
            continue;
        }
#endif /* CMD_LUA_CLI */

        if (cmdOsStrCmp(inputBuffer, "osShell") == 0)
        {
            cmdOsShell(IOStream);

            if (IOStream->isConsole)
                IOStream->writeLine(IOStream, CMD_PROMPT);
            continue;
        }
        
        if (IS_ECHO_OFF_CMD(inputBuffer))
        {
            IOStream->flags &= ~IO_STREAM_FLAG_I_ECHO;
            IOStream->isConsole = GT_FALSE;
            continue;
        }
        if (IS_ECHO_ON_CMD(inputBuffer))
        {
            IOStream->flags |= IO_STREAM_FLAG_I_ECHO;
            IOStream->isConsole = GT_TRUE;
            continue;
        }
        if (!inputBuffer[0])
        {
            if (IOStream->isConsole)
                IOStream->writeLine(IOStream, CMD_PROMPT);
            continue;
        }

        /* check and read fields if needed */
        if (cmdEventFields(inputBuffer) == GT_TRUE)
        {
#ifdef FIELD_PROMPT
            if (IOStream->isConsole)
                IOStream->writeLine(IOStream, CMD_FIELD_PROMPT);
#endif
            if (IOStream->readLine(IOStream, fieldBuffer, sizeof(fieldBuffer)) < 0)
            {
                /* input failed, possible connection failed */
                break;
            }
        }

        cmdTakeGlobalStdio(IOStream);

        outputBuffer = NULL;
        /* execute command: pass command buffer to cmdEventRun */
        status = cmdEventRun(inputBuffer, fieldBuffer, &outputBuffer);

        /* check error code returned by cmdEventRun */
        if (status == CMD_OK)
        {
            /* check whether output buffer contains any data */
            if ((outputBuffer != NULL) && (cmdOsStrlen(outputBuffer) != 0))
            {

                /* Append the prompt to the last message*/
                if (IOStream->isConsole)
                    cmdOsStrCat(outputBuffer, CMD_PROMPT);
                /* send output buffer generated by api call */
                IOStream->writeLine(IOStream, outputBuffer);
                IOStream->writeLine(IOStream, "\n\n");
            }
            else
            {
                /* Send the prompt only */
                if (IOStream->isConsole)
                    IOStream->writeLine(IOStream, CMD_PROMPT);
            }
        }
        else
        {
            static GT_CHAR galtisError[4096];

            /* discard any pending data redirected from stdout */

            /* send error message and continue command loop */
            galtisOutput((GT_8*)galtisError, status, "");

            /* Append the prompt to the last messgae*/
            if (IOStream->isConsole)
                cmdOsStrCat(galtisError, CMD_PROMPT);
            IOStream->writeLine(IOStream, galtisError);
        }
        cmdReleaseGlobalStdio();
    }

    return GT_OK;
}


GT_VOID cmdClose(GT_U32 devNum)
{
    /*closeGaltis(devNum);*/
    notExitFlag = 0;
    return;
}



