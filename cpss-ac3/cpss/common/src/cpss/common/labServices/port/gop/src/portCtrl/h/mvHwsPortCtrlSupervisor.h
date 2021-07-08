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
* @file mvHwsPortCtrlSupervisor.h
*
* @brief Port Control Supervisor State Machine
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlSuperVisor_H
#define __mvHwsPortCtrlSuperVisor_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    PORT_SET = 0,
    PORT_CLR = 1,

}MV_PORT_ACTION;

/**
* @internal mvPortCtrlSpvRoutine function
* @endinternal
*
*/
void mvPortCtrlSpvRoutine(void* pvParameters);

/**
* @internal mvPortCtrlSpvHighMsg function
* @endinternal
*
*/
void mvPortCtrlSpvHighMsg(void);

/**
* @internal mvPortCtrlSpvHighMsgExecute function
* @endinternal
*
*/
void mvPortCtrlSpvHighMsgExecute(MV_HWS_IPC_CTRL_MSG_STRUCT *recvMsg);

/**
* @internal mvPortCtrlSpvLowMsg function
* @endinternal
*
*/
void mvPortCtrlSpvLowMsg(void);

/**
* @internal mvPortCtrlSpvLowMsgExecute function
* @endinternal
*
*/
void mvPortCtrlSpvLowMsgExecute(MV_HWS_IPC_CTRL_MSG_STRUCT *recvMsg);

/**
* @internal mvPortCtrlSpvDelay function
* @endinternal
*
*/
void mvPortCtrlSpvDelay(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlSuperVisor_H */


