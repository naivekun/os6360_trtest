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
* @file u_cmdStreamSocket.c
*
* @brief This file contains mainCmd socket stream implementation for UNIX OSes
*
* @version   2
********************************************************************************
*/
#include <cmdShell/os/cmdStreamImpl.h>
#include <unistd.h>
#include <signal.h>

/* include generic code */
#include "cmdStreamSocket.c"

/**
* @internal cmdStreamSocketInit function
* @endinternal
*
* @brief   Initialize TCP/IP socket engine
*/
GT_STATUS cmdStreamSocketInit(void)
{
    signal(SIGPIPE, SIG_IGN);
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
    return GT_OK;
}



