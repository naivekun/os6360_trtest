/*******************************************************************************
*              (c), Copyright 2007, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* IOStreamImpl.h
*
* DESCRIPTION:
*       This file contains types and routines for mainCmd I/O streams
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifndef __IOStreamImplh
#define __IOStreamImplh

#include <extUtils/IOStream/IOStream.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal IOStreamCreateSocket function
* @endinternal
*
* @brief   Create socket stream
*
* @param[in] socket                   -  descriptor
*
* @retval stream                   - socket stream
*                                       NULL if error
*/
IOStreamPTR IOStreamCreateSocket
(
    IN CPSS_SOCKET_FD socket
);

/**
* @internal IOStreamCreateTelnet function
* @endinternal
*
* @brief   Create Telnet protocol stream
*
* @param[in] socket                   -  I/O stream
*
* @retval stream                   - socket stream
*                                       NULL if error
*/
IOStreamPTR IOStreamCreateTelnet
(
    IN IOStreamPTR socket
);

/* WNC_LUA_CLI_ENHANCE_IOP start */
IOStreamPTR IOStreamCreateTelnet_PreAllocate
(
    void
);
/* WNC_LUA_CLI_ENHANCE_IOP end */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __IOStreamImplh */

