/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*******************************************************************************/
#include <mcdFwPortCtrlInc.h>
#include <mcdFwPortCtrlIpc.h>
#include "common.h"
#include <mvShmIpc_wrapper.h>
#include <ic.h>

MCD_IPC_CTRL_MSG_STRUCT ipcMsg;

#ifdef BC2_DEV_SUPPORT
extern GT_U32 portBC2SerdesNum[];
extern GT_U32 portAC3SerdesNum[];
#else /*  not BC2_DEV_SUPPORT*/
extern GT_U32 portSerdesNum[];
#endif




#ifndef CONFIG_MV_IPC_FREERTOS_DRIVER
/**
* @internal host2servicecpu_isr function
* @endinternal
*
* @brief   This interrupt handling only works when servicecpu is not working in ipc mode
*         It's trigged when host sends a doorbell irq to servicecpu
*         This interrupt handling only works for MSYS family boards
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static long host2servicecpu_isr(void *data)
{
    MV_U32 reg;

    mv_os_printf("Interrupt from host CPU\n");

    MV_REG_BIT_RESET(CPU_DOORBELL_IN_MASK_REG, 1 << IPC_BASE_DOORBELL);
    MV_REG_WRITE(CPU_DOORBELL_IN_REG, ~(1 << IPC_BASE_DOORBELL));

    /* Generate a doorbell irq to host CPU */
    mv_os_printf("Generate a doorbell irq to host CPU back\n");
    MV_REG_WRITE(CPU_DOORBELL_OUT_REG, 1 << IPC_BASE_DOORBELL);

    MV_REG_BIT_SET(CPU_DOORBELL_IN_MASK_REG, (1));

    return 0;
}
#endif /* CONFIG_MV_IPC_FREERTOS_DRIVER */

#if 0
/**
* @internal ipc_request_irq function
* @endinternal
*
* @brief   This API register polling task handler
*         It is implemented via Port Control Supervisour task
*         The API is for compatibility with IPC link structure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static int ipc_request_irq(unsigned int link_id, void *dev_id)
{
    /* This API register polling task handler */
    /* It is implemented via Port Control Supervisour task */
    return 0;
}
#endif

/**
* @internal mvIpcSendTriggerPolling function
* @endinternal
*
*/
MV_VOID mvIpcSendTriggerPolling(MV_U32 linkId, MV_U32 chnId)
{
    /* Do nothing for polling */
    return;
}

#if (defined BC2_DEV_SUPPORT) || (defined AC3_DEV_SUPPORT)
/**
* @internal mvIpcSendTriggerPCIDrbl function
* @endinternal
*
*/
MV_VOID mvIpcSendTriggerPCIDrbl(MV_U32 linkId, MV_U32 chnId)
{
    MV_U32 doorbellNum;

    /* now all ipc channels share one doorbell - IPC_BASE_DOORBELL*/
    /* so we do not use chnId now, but keep it as an input parameter*/
    doorbellNum  = IPC_BASE_DOORBELL;

    /* Use outbound doorbell IPC_BASE_DOORBELL(12)  for IPC */
    MV_REG_WRITE(CPU_DOORBELL_OUT_REG, 1 << doorbellNum);

    return;
}
#endif /* #if (defined BC2_DEV_SUPPORT) || (defined AC3_DEV_SUPPORT)*/

#if 0
/**
* @internal ipc_trigger_irq function
* @endinternal
*
* @brief   Trigger Doorbell interrupt from MSYS to A385
*/
static void ipc_trigger_irq(unsigned int linkId, unsigned int chnId, void *dev_id)
{
#if defined BC2_DEV_SUPPORT

    mvIpcSendTriggerPCIDrbl(linkId, chnId);
#elif defined BOBK_DEV_SUPPORT || defined ALDRIN_DEV_SUPPORT
    cm3_to_msys_doorbell();

#endif
}
#endif

#if (defined BC2_DEV_SUPPORT) || (defined AC3_DEV_SUPPORT)
/**
* @internal mvIpcPCIDrblInit function
* @endinternal
*
*/
MV_VOID mvIpcPCIDrblInit(void)
{
    /* Forward Outbound doorbell to PCIe Host */
    /* now all ipc channels share one doorbell - IPC_BASE_DOORBELL*/
    MV_REG_WRITE(CPU_DOORBELL_OUT_MASK_REG, 1 << IPC_BASE_DOORBELL);

    MV_REG_BIT_SET(CPU_DOORBELL_IN_MASK_REG, 1 << IPC_BASE_DOORBELL);

    MV_REG_WRITE(CPU_INT_SRC_CTRL(IRQ_OUT_DOORBELL_SUM),
                 (1 << ICBIT_SRC_CTRL_EP_MASK)  |
                 (1 << ICBIT_SRC_CTRL_INT_EN)   |
                 (1 << ICBIT_SRC_CTRL_PRIORITY) |
                 (1 << ICBIT_SRC_CTRL_EP_SEL));

#ifndef CONFIG_MV_IPC_FREERTOS_DRIVER
    /* Register Doorbell interrupt handler */
    iICRegisterHandler(IRQ_IN_DOORBELL_LOW, host2servicecpu_isr, (void *)0,
                       IRQ_ENABLE, IRQ_PRIO_DEFAULT);
    iICRegisterHandler(IRQ_IN_DOORBELL_HIGH, host2servicecpu_isr, (void *)1,
                       IRQ_ENABLE, IRQ_PRIO_DEFAULT);
#endif /* CONFIG_MV_IPC_FREERTOS_DRIVER */

    return;
}
#endif /* #if (defined BC2_DEV_SUPPORT) || (defined AC3_DEV_SUPPORT)*/

/**
* @internal mvPortCtrlIpcInit function
* @endinternal
*
* @brief   Initialize Port control IPC infrastructure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlIpcInit(void)
{
#if defined BC2_DEV_SUPPORT
    mvIpcPCIDrblInit();
#endif

    wr_Ipc_Init((void *)SHMEM_BASE, (void*)SHMEM_REGS, SHMEM_SIZE);

    return GT_OK;
}

/**
* @internal mvPortCtrlIpcActivate function
* @endinternal
*
* @brief   Initialize Port control IPC infrastructure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlIpcActivate(void)
{
    return GT_OK;
}

/**
* @internal mvHwsIpcRequestGet function
* @endinternal
*
* @brief   The API is called from Supervisour process to retrive message
*         from IPC driver
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcRequestGet(GT_U32 priority, GT_U32 msgSize, char *msg)
{
    int channel;

#ifdef MICRO_INIT
    if (wr_Ipc_boot_channel_ready())
    {
        if(wr_Ipc_boot_channel_receive())
        {
            return GT_FAIL;
        }
    }
#endif

    if (priority != MCD_IPC_HIGH_PRI_QUEUE && priority != MCD_IPC_LOW_PRI_QUEUE)
        return GT_FAIL;
    channel = (priority == MCD_IPC_HIGH_PRI_QUEUE)
            ? MCD_IPC_HI_PRI_RX_CH_ID : MCD_IPC_LO_PRI_RX_CH_ID;
    if (wr_Ipc_rx_msg(channel, msg) == 1)
    {
        return GT_OK;
    }

    return GT_FAIL;
}

/**
* @internal mvHwsIpcReplyMsg function
* @endinternal
*
* @brief   Buils and Send reply from HW Services to the Host
*
* @param[in] queueId                  - queue ID
* @param[in] txReplyMsg               - pointer to message
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcReplyMsg
(
    GT_U32                      queueId,
    MCD_IPC_REPLY_MSG_STRUCT *txReplyMsg
)
{
    int status;

    if (queueId == MV_PORT_CTRL_NO_MSG_REPLY)
    {
        return GT_OK;
    }

    mvPortCtrlIpcLock();
    status = wr_Ipc_tx_queue_send(queueId, txReplyMsg, sizeof(MCD_IPC_REPLY_MSG_STRUCT));
    mvPortCtrlIpcUnlock();
    if (status == -1)
    {
        mv_os_printf("IPC: Failed to sent reply message via channel %d\n", queueId);
    }

    return GT_OK;
}

/**
* @internal mvHwsIpcRequestSet function
* @endinternal
*
* @brief   IPC command simulation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcRequestSet(GT_U32 type, GT_U32 port, GT_U32 mode)
{
    /* Code not relevant for BobK CM3 due to space limit */
    if (type == MCD_IPC_PORT_INIT_MSG)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_INIT_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_INIT_DATA_STRUCT);
        ipcMsg.msgData.portInit.portGroup = 0;
        ipcMsg.msgData.portInit.phyPortNum = port;
        ipcMsg.msgData.portInit.portMode = mode;
        ipcMsg.msgData.portInit.lbPort = GT_FALSE;
        ipcMsg.msgData.portInit.refClock = MHz_156;
        ipcMsg.msgData.portInit.refClockSource = PRIMARY_LINE_SRC;

        return mvPortCtrlProcessMsgSend(M2_PORT_MNG, &ipcMsg);
    }
    else
    if (type == MCD_IPC_PORT_LINK_STATUS_GET_MSG)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_LINK_STATUS_GET_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_INFO_STRUCT);
        ipcMsg.msgData.portLinkStatus.portGroup = 0;
        ipcMsg.msgData.portLinkStatus.phyPortNum = port;
        ipcMsg.msgData.portLinkStatus.portMode = mode;

        return mvPortCtrlProcessMsgSend(M6_GENERAL, &ipcMsg);
    }
    else if (type == MCD_IPC_PORT_AUTO_TUNE_SET_EXT_MSG)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_AUTO_TUNE_SET_EXT_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT);
        ipcMsg.msgData.portAutoTuneSetExt.portGroup = 0;
        ipcMsg.msgData.portAutoTuneSetExt.phyPortNum = port;
        ipcMsg.msgData.portAutoTuneSetExt.portMode = (mode & 0xFFFF);
        ipcMsg.msgData.portAutoTuneSetExt.portTuningMode = ((mode >> 16) & 0xFFFF);

        return mvPortCtrlProcessMsgSend(M6_GENERAL, &ipcMsg);
    }
    else
    if (type == MCD_IPC_PORT_AP_ENABLE_MSG)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_AP_ENABLE_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_AP_DATA_STRUCT);
        ipcMsg.msgData.apConfig.retimerMode = 0;
        ipcMsg.msgData.apConfig.phyPortNum = port;
#ifdef BC2_DEV_SUPPORT
        if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
        {
            ipcMsg.msgData.apConfig.laneNum = portBC2SerdesNum[port - MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE];
        }
        else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
        {
            ipcMsg.msgData.apConfig.laneNum = portAC3SerdesNum[port - MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE];
        }
        else
        {
            return GT_NOT_IMPLEMENTED;
        }
#else /*!BC2_DEV_SUPPORT */
        ipcMsg.msgData.apConfig.laneNum = portSerdesNum[port - MV_PORT_CTRL_AP_PORT_NUM_BASE];
#endif
        ipcMsg.msgData.apConfig.pcsNum = port;
        ipcMsg.msgData.apConfig.macNum = port;
        ipcMsg.msgData.apConfig.advMode = 0;
        AP_PRINT_MCD2_MAC(("mvHwsIpcRequestSet mode:0x%x\n",mode));
        switch(mode & 0xFFFF)
        {
        case _1000Base_X:
            AP_CTRL_1000Base_KX_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _10GBase_KX4:
            AP_CTRL_10GBase_KX4_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _10GBase_KR:
            AP_CTRL_10GBase_KR_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _40GBase_KR:
            AP_CTRL_40GBase_KR4_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _25GBase_KR:
            AP_CTRL_25GBase_KR1_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _25GBase_KR_C:
            AP_CTRL_25GBase_KR1_CONSORTIUM_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _50GBase_KR2_C:
            AP_CTRL_50GBase_KR2_CONSORTIUM_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        default:
            return GT_FAIL;
        }
        ipcMsg.msgData.apConfig.options = 0x200; /* [09:09] loopback Enable */

        return mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);
    }
    else if (type == MCD_IPC_PORT_AP_DISABLE_MSG)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_AP_DISABLE_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_AP_DATA_STRUCT);
        ipcMsg.msgData.apConfig.retimerMode = 0;
        ipcMsg.msgData.apConfig.phyPortNum = port;

        return mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);
    }
    /* Code not relevant for BobK CM3 due to space limit */
    else if (MCD_IPC_PORT_RESET_MSG == type)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_RESET_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_RESET_DATA_STRUCT);
        ipcMsg.msgData.portReset.action = 0 /* PORT_POWER_DOWN */;
        ipcMsg.msgData.portReset.portGroup = 0;
        ipcMsg.msgData.portReset.phyPortNum = port;
        ipcMsg.msgData.portReset.portMode = mode;

        return mvPortCtrlProcessMsgSend(M2_PORT_MNG, &ipcMsg);
    }

    return GT_NOT_IMPLEMENTED;
}

GT_STATUS mvHwsIpcApRetimerRequestSet
(
    GT_U32  type,
    GT_U32  port,
    GT_U32  mode,
    GT_U8   laneNum,
    GT_U16  options,
    GT_U8   polarityVector,
    GT_U32  retimerMode,
    GT_U16  rxRemapVector,
    GT_U16  txRemapVector
)
{
    if (type == MCD_IPC_PORT_AP_ENABLE_MSG)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_AP_ENABLE_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_AP_DATA_STRUCT);
        ipcMsg.msgData.apConfig.retimerMode = retimerMode;
        ipcMsg.msgData.apConfig.phyPortNum = port;
        ipcMsg.msgData.apConfig.laneNum = laneNum;
        ipcMsg.msgData.apConfig.pcsNum = laneNum;
        ipcMsg.msgData.apConfig.macNum = port;
        ipcMsg.msgData.apConfig.advMode = 0;
        AP_PRINT_MCD2_MAC(("mvHwsIpcRequestSet mode:0x%x\n",mode));
        ipcMsg.msgData.apConfig.advMode = mode;
        ipcMsg.msgData.apConfig.options = options; /* [09:09] loopback Enable */
        ipcMsg.msgData.apConfig.polarityVector = polarityVector;
        ipcMsg.msgData.apConfig.rxRemapVector = rxRemapVector;
        ipcMsg.msgData.apConfig.txRemapVector = txRemapVector;
        return mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);
    }
    else if (type == MCD_IPC_PORT_AP_DISABLE_MSG)
    {
        ipcMsg.devNum = 0;
        ipcMsg.ctrlMsgType = MCD_IPC_PORT_AP_DISABLE_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MCD_IPC_PORT_AP_DATA_STRUCT);
        ipcMsg.msgData.apConfig.retimerMode = 0;
        ipcMsg.msgData.apConfig.phyPortNum = port;
        return mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);
    }
    return GT_NOT_IMPLEMENTED;
}


