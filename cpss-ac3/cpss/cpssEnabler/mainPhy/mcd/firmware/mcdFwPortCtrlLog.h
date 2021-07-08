/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mcdFwPortCtrlLog.h
*
* @brief Port Control Log
*
* @version   1
********************************************************************************
*/

#ifndef __mcdFwPortCtrlLog_H
#define __mcdFwPortCtrlLog_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal mvPortCtrlLogInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlLogInit(void);

/**
* @internal mvPortCtrlLogInfoGet function
* @endinternal
*
*/
void mvPortCtrlLogInfoGet(GT_U32 *fwBaseAddr, GT_U32 *logBaseAddr, GT_U32 *logPointer,
                          GT_U32 *logCount, GT_U32 *logReset);

void mvHwsLogInfoGet(GT_U32 *fwBaseAddr, GT_U32 *hwsLogBaseAddr, GT_U32 *hwsLogPointerAddr,
                     GT_U32 *hwsLogCountAddr, GT_U32 *hwsLogResetAddr, GT_U32 *poolingDbAddr);

/**
* @internal mvPortCtrlLogAdd function
* @endinternal
*
*/
void mvPortCtrlLogAdd(GT_U32 entry);

/**
* @internal mvPortCtrlLogDump function
* @endinternal
*
*/
void mvPortCtrlLogDump(GT_U32 port);

/**
* @internal mvPortCtrlLogLock function
* @endinternal
*
*/
GT_STATUS mvPortCtrlLogLock(void);
GT_STATUS mvPortCtrlLogUnlock(void);

/**
* @internal mvPortCtrlIpcLock function
* @endinternal
*
*/
GT_STATUS mvPortCtrlIpcLock(void);
GT_STATUS mvPortCtrlIpcUnlock(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mcdFwPortCtrlLog_H */


