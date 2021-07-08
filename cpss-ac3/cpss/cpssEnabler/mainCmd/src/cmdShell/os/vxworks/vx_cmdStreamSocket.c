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
* @file vx_cmdStreamSocket.c
*
* @brief This file contains mainCmd socket stream implementation for vxWorks
*
* @version   3
********************************************************************************
*/
#include <ioLib.h>
#include <sioLib.h>
#include <ttyLib.h>
#include <sysLib.h>
#include <selectLib.h>
#include <string.h>
#include <cmdShell/os/cmdStreamImpl.h>

/* include generic code */
#include "../generic/cmdStreamSocket.c"

/***** Global variables ************************************************/
static GT_BOOL osExInitialized = GT_FALSE;

extern int  taskNameToId (char *name);

/***** Initialization **************************************************/


/**
* @internal cmdStreamSocketInit function
* @endinternal
*
* @brief   Initialize TCP/IP socket engine
*/
GT_STATUS cmdStreamSocketInit(void)
{
    /* initialize only once */
    int tid = 0;
    GT_U32 old_priority;

    if (!osExInitialized)
    {
        osExInitialized = GT_TRUE;
        tid = taskNameToId("tNetTask");
        if (tid != 0)
        {    /* same as the keep alive task of  the terminal*/
            if (cmdOsSetTaskPrior(tid,STREAM_THREAD_PRIO-1,&old_priority)!= GT_OK)
            {
                return GT_FAIL;
            }
        } 
        else
        {
            return GT_FAIL;
        }
    }

    return GT_OK;
}

/**
* @internal cmdStreamSocketFinish function
* @endinternal
*
* @brief   Close socket engine
*/
GT_STATUS cmdStreamSocketFinish(void)
{
    if (osExInitialized)
        osExInitialized = GT_FALSE;

    return GT_OK;
}

static int socketGrabStd(cmdStreamPTR streamP)
{
    socketStreamPTR stream = (socketStreamPTR)streamP;

    if (!stream)
        return -1;

    /* ioGlobalStdSet(STD_IN, stream->socket); */
    ioGlobalStdSet(STD_OUT, stream->socket);
    ioGlobalStdSet(STD_ERR, stream->socket);

    return 1;
}


