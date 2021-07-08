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
* @file w32_cmdStreamSocket.c
*
* @brief This file contains mainCmd socket stream implementation for Win32
*
* @version   3
********************************************************************************
*/

/* WA to avoid next warning :
   due to include to : #include <windows.h>
    c:\program files\microsoft visual studio\vc98\include\rpcasync.h(45) :
    warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
*/
struct _RPC_ASYNC_STATE;

#include <cmdShell/os/cmdStreamImpl.h>
#include <windows.h>
#include <winsock.h>
#include <gtOs/gtGenTypes.h>

/* include generic code */
#include "../generic/cmdStreamSocket.c"

/***** Global variables ************************************************/
static GT_BOOL osExInitialized = GT_FALSE;
/***** Initialization **************************************************/


/**
* @internal cmdStreamSocketInit function
* @endinternal
*
* @brief   Initialize TCP/IP socket engine
*/
GT_STATUS cmdStreamSocketInit(void)
{
    WSADATA wsaData;

    /* initialize only once */
    if (!osExInitialized)
    {
        /* initialize winsock application interface */
        if (WSAStartup(0x202, &wsaData) != 0)
        {
            WSACleanup();
            return GT_FAIL;
        }
        osExInitialized = GT_TRUE;
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
    {
        WSACleanup();
        osExInitialized = GT_FALSE;
    }

    return GT_OK;
}


