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
* @file mvHwsPortCtrlIpc.h
*
* @brief Port Control Internal IPC Message Definitions
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlIpc_H
#define __mvHwsPortCtrlIpc_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <mvHwsIpcDefs.h>

#ifdef CONFIG_MV_IPC_FREERTOS_DRIVER
#define IPC_REMOTE_FREERTOS_NODE_ID   MV_IPC_NODE_ID_MASTER

#if !defined(BOBK_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BC3_DEV_SUPPORT) && !defined(PIPE_DEV_SUPPORT) && !defined(ALDRIN2_DEV_SUPPORT)

#define IPC_BASE_DOORBELL             12
#define IPC_DOORBELL_NUM(chnId)       (IPC_BASE_DOORBELL + chnId )

/* ARM Doorbell Registers Map */
#define CPU_SW_TRIG_IRQ               (MV_MBUS_REGS_OFFSET + 0xA04)
#define CPU_INT_SRC_CTRL( i )         (MV_MBUS_REGS_OFFSET + ICREG_SRC_CTRL(i))
#define CPU_DOORBELL_IN_REG           (MV_CPUIF_LOCAL_REGS_OFFSET + 0x78)
#define CPU_DOORBELL_IN_MASK_REG      (MV_CPUIF_LOCAL_REGS_OFFSET + 0x7C)
#define CPU_DOORBELL_OUT_REG          (MV_CPUIF_LOCAL_REGS_OFFSET + 0x70)
#define CPU_DOORBELL_OUT_MASK_REG     (MV_CPUIF_LOCAL_REGS_OFFSET + 0x74)
#define CPU_HOST_TO_ARM_DRBL_REG(cpu) (MV_CPUIF_REGS_BASE(cpu) + 0x78)
#define CPU_HOST_TO_ARM_MASK_REG(cpu) (MV_CPUIF_REGS_BASE(cpu) + 0x7C)
#define CPU_ARM_TO_HOST_DRBL_REG(cpu) (MV_CPUIF_REGS_BASE(cpu) + 0x70)
#define CPU_ARM_TO_HOST_MASK_REG(cpu) (MV_CPUIF_REGS_BASE(cpu) + 0x74)

#endif /* BOBK_DEV_SUPPORT */
#endif /* CONFIG_MV_IPC_FREERTOS_DRIVER */

#define MV_PORT_CTRL_NO_MSG_REPLY     (0xFF)

typedef struct
{
    unsigned int link;
    unsigned int chn;
} mv_ipc_chn_info;

/**
* @internal mvPortCtrlIpcInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlIpcInit(void);

/**
* @internal mvPortCtrlIpcActivate function
* @endinternal
*
*/
GT_STATUS mvPortCtrlIpcActivate(void);

/**
* @internal mvHwsIpcRequestGet function
* @endinternal
*
*/
GT_STATUS mvHwsIpcRequestGet(GT_U32 priority, GT_U32 msgSize, char *msg);

/**
* @internal mvHwsIpcReplyMsg function
* @endinternal
*
*/
GT_STATUS mvHwsIpcReplyMsg(GT_U32 queueId, MV_HWS_IPC_REPLY_MSG_STRUCT *txReplyMsg);

/**
* @internal mvHwsIpcRequestSet function
* @endinternal
*
*/
GT_STATUS mvHwsIpcRequestSet(GT_U32 type, GT_U32 port, GT_U32 mode, GT_U8 polarity);

/**
* @internal mvHwsIpcApRequestSet function
* @endinternal
*
*/
GT_STATUS mvHwsIpcApRequestSet(
    GT_U32 type,
    GT_U32 portNum,
    GT_U32 portMode,
    GT_U32 fecAbil,
    GT_U32 fecReq,
    GT_U8 polarity
);


GT_STATUS mvHwsIpcApEnable(
    GT_U32 portMacNum,
    GT_U32 relLaneNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_U32 fecAbil,
    GT_U32 fecReq,
    GT_U32 fcPause,
    GT_U32 fcAsmDir,
    GT_U8 polarity
);

/**
* @internal mvPortCtrlProcessPendMsgNum function
* @endinternal
*
*/
GT_STATUS mvPortCtrlProcessPendMsgNum(MV_PORT_CTRL_MODULE module, GT_U32 *msgNum);

/**
* @internal mvPortCtrlProcessMsgRecv function
* @endinternal
*
*/
GT_STATUS mvPortCtrlProcessMsgRecv(MV_PORT_CTRL_MODULE module,
                                   GT_U32 duration,
                                   MV_HWS_IPC_CTRL_MSG_STRUCT *msgPtr);

/**
* @internal mvPortCtrlProcessMsgSend function
* @endinternal
*
*/
GT_STATUS mvPortCtrlProcessMsgSend(MV_PORT_CTRL_MODULE module,
                                   MV_HWS_IPC_CTRL_MSG_STRUCT *msgPtr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlIpc_H */


