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
* @file mcdFwPortCtrlPort.h
*
* @brief Port Control Port Initialization
*
* @version   1
********************************************************************************
*/

#ifndef __mcdFwPortCtrlPort_H
#define __mcdFwPortCtrlPort_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Message processing actions */
#define PORT_MSG_PEEK (0)
#define PORT_MSG_PROC (1)
#define PORT_MSG_FREE (2)


/* Regular Port Management API */
/* =========================== */

/**
* @internal mvPortCtrlPortRoutine function
* @endinternal
*
*/
void mvPortCtrlPortRoutine(void* pvParameters);

/**
* @internal mvPortCtrlPortMsg function
* @endinternal
*
*/
void mvPortCtrlPortMsg(void);

/**
* @internal mvPortCtrlPortMsgExec function
* @endinternal
*
*/
GT_STATUS mvPortCtrlPortMsgExec(GT_U32 port,
                                MCD_IPC_CTRL_MSG_STRUCT *msg);

/**
* @internal mvPortCtrlPortMsgReply function
* @endinternal
*
*/
GT_STATUS mvPortCtrlPortMsgReply(GT_U32 rcode, GT_U8 queueId, GT_U8 msgType);

/**
* @internal mvPortCtrlPortDelay function
* @endinternal
*
*/
void mvPortCtrlPortDelay(void);

/* AP/SD Port Management API */
/* ========================= */

/**
* @internal mvPortCtrlApPortRoutine function
* @endinternal
*
*/
void mvPortCtrlApPortRoutine(void* pvParameters);

/**
* @internal mvPortCtrlApPortMsg function
* @endinternal
*
*/
void mvPortCtrlApPortMsg(void);

/**
* @internal mvPortCtrlApPortActive function
* @endinternal
*
*/
void mvPortCtrlApPortActive(void);

/**
* @internal mvPortCtrlApPortDelay function
* @endinternal
*
*/
void mvPortCtrlApPortDelay(void);

#ifndef DISABLE_CLI
/**
* @internal mvPortCtrlApPortStatus function
* @endinternal
*
*/
void mvPortCtrlApPortStatus(void);
#endif /* DISABLE_CLI */

/* Time & Threshold  API */
/* ====================== */

/**
* @internal mvPortCtrlCurrentTs function
* @endinternal
*
*/
GT_U32 mvPortCtrlCurrentTs(void);

/**
* @internal mvPortCtrlThresholdSet function
* @endinternal
*
*/
void mvPortCtrlThresholdSet(GT_U32 delay, GT_U32 *thrershold);

/**
* @internal mvPortCtrlThresholdCheck function
* @endinternal
*
*/
GT_U32 mvPortCtrlThresholdCheck(GT_U32 thrershold);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mcdFwPortCtrlPort_H */



