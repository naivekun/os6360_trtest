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
* mvHwsPortCtrlGeneral.c
*
* DESCRIPTION:
*       Port Control General State Machine
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <mvHwsPortCtrlInc.h>
#include <mvHwsPortCtrlIpc.h>
#include <private/mvPortModeElements.h>
#include "common.h"
#include <mvShmIpc_wrapper.h>
#include <ic.h>

MV_HWS_IPC_CTRL_MSG_STRUCT ipcMsg;

#ifdef BC2_DEV_SUPPORT
extern GT_U32 portBC2SerdesNum[];
extern GT_U32 portAC3SerdesNum[];
#else /*  not BC2_DEV_SUPPORT*/
#if !defined(BC3_DEV_SUPPORT) && !defined(ALDRIN2_DEV_SUPPORT) && !defined(PIPE_DEV_SUPPORT)
extern GT_U32 portSerdesNum[];
#endif
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
#elif defined BOBK_DEV_SUPPORT || defined ALDRIN_DEV_SUPPORT || defined BC3_DEV_SUPPORT || defined ALDRIN2_DEV_SUPPORT
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

    if (priority != MV_HWS_IPC_HIGH_PRI_QUEUE && priority != MV_HWS_IPC_LOW_PRI_QUEUE)
        return GT_FAIL;
    channel = (priority == MV_HWS_IPC_HIGH_PRI_QUEUE)
            ? MV_HWS_IPC_HI_PRI_RX_CH_ID : MV_HWS_IPC_LO_PRI_RX_CH_ID;
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
    MV_HWS_IPC_REPLY_MSG_STRUCT *txReplyMsg
)
{
    int status;

    if (queueId == MV_PORT_CTRL_NO_MSG_REPLY)
    {
        return GT_OK;
    }

    mvPortCtrlIpcLock();
    status = wr_Ipc_tx_queue_send(queueId, txReplyMsg, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
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
GT_STATUS mvHwsIpcRequestSet(GT_U32 type, GT_U32 port, GT_U32 mode, GT_U8 polarity)
{
    ipcMsg.devNum = 0;
    ipcMsg.msgData.portInit.portGroup = 0;
    ipcMsg.msgData.portInit.phyPortNum = port;

    /* Code not relevant for BobK CM3 due to space limit */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
#ifdef REG_PORT_TASK
    if (type == MV_HWS_IPC_PORT_INIT_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_INIT_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_INIT_DATA_STRUCT);
        ipcMsg.msgData.portInit.portMode = mode;
        ipcMsg.msgData.portInit.lbPort = GT_FALSE;
        ipcMsg.msgData.portInit.refClock = MHz_156;
        ipcMsg.msgData.portInit.refClockSource = PRIMARY_LINE_SRC;
        return mvPortCtrlProcessMsgSend(M2_PORT_MNG, &ipcMsg);
    }
#endif
#ifdef AP_GENERAL_TASK
    else
    if (type == MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_INFO_STRUCT);
        ipcMsg.msgData.portLinkStatus.portMode = mode;
        return mvPortCtrlProcessMsgSend(M6_GENERAL, &ipcMsg);
    }
    else if (type == MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT);
        ipcMsg.msgData.portAutoTuneSetExt.portMode = (mode & 0xFFFF);
        ipcMsg.msgData.portAutoTuneSetExt.portTuningMode = ((mode >> 16) & 0xFFFF);
        return mvPortCtrlProcessMsgSend(M6_GENERAL, &ipcMsg);
    }
#endif
#ifdef REG_PORT_TASK
    else
#endif
#endif
    if (type == MV_HWS_IPC_PORT_AP_ENABLE_MSG)
    {
        return mvHwsIpcApRequestSet(type, port, mode, 0, 0, polarity);
    }
    else if (type == MV_HWS_IPC_PORT_AP_DISABLE_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_AP_DISABLE_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT);
        return mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);
    }
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
#ifdef REG_PORT_TASK
    else if (MV_HWS_IPC_PORT_RESET_MSG == type)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_RESET_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_RESET_DATA_STRUCT);
        ipcMsg.msgData.portReset.action = 0 /* PORT_POWER_DOWN */;
        ipcMsg.msgData.portReset.portMode = mode;
        return mvPortCtrlProcessMsgSend(M2_PORT_MNG, &ipcMsg);
    }
#endif
    else if (MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG == type)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_INFO_STRUCT);
        ipcMsg.msgData.portReset.portMode = mode;
        mvPortCtrlSpvHighMsgExecute(&ipcMsg);
        return GT_OK;
    }
#endif
    return GT_NOT_IMPLEMENTED;
}


GT_STATUS mvHwsIpcApRequestSet(GT_U32 type,
                               GT_U32 portMacNum,
                               GT_U32 mode,
                               GT_U32 fecAbil,
                               GT_U32 fecReq,
                               GT_U8 polarity
                               )
{

    if (type == MV_HWS_IPC_PORT_AP_ENABLE_MSG)
        return mvHwsIpcApEnable(portMacNum, 0, mode, fecAbil, fecReq, 0, 0, polarity);
    else
        return GT_NOT_IMPLEMENTED;
}

/**
* @internal mvHwsIpcApRequestSet function
* @endinternal
*
* @brief   IPC command simulation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcApEnable(GT_U32 portMacNum,
                           GT_U32 relLaneNum,
                           MV_HWS_PORT_STANDARD portMode,
                           GT_U32 fecAbil,
                           GT_U32 fecReq,
                           GT_U32 fcPause,
                           GT_U32 fcAsmDir,
                           GT_U8 polarity
                           )
{
    GT_BOOL consortium = GT_FALSE;
    GT_U32 fec = 0;
    GT_U32 reqLanes;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    //osPrintf("MacNum %d LaneNum %d portMode %d fecAbil %d fecReq %d fcPause %d fcAsmDir %d polarity %d\n",
    //     portMacNum, relLaneNum, portMode,fecAbil, fecReq, fcPause, fcAsmDir, polarity);

    ipcMsg.devNum = 0;
    ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_AP_ENABLE_MSG;
    ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
    ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT);
    ipcMsg.msgData.apConfig.portGroup = 0;
    ipcMsg.msgData.apConfig.phyPortNum = portMacNum;

    if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, portMacNum, portMode, &curPortParams))
    {
        // osPrintf("curPortParams is NULL\n");
        return GT_BAD_PARAM;
    } else {
        reqLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;
        if (relLaneNum >= reqLanes ) {
            // osPrintf("failed: laneNum %d reqLanes %d\n", relLaneNum, reqLanes);
            return GT_BAD_PARAM;
        }
    }

#if defined(MICRO_INIT)
#ifdef ALDRIN_DEV_SUPPORT
        /* Build params for the refClockSourceDb with refClock=MHz_78 (bits 4-7), refClockSource=SECONDARY_LINE_SRC (bits 1-3) and isValid=1 (bit 0) */
        ipcMsg.msgData.apConfig.refClockSrcParams = 0x13;

#endif
        ipcMsg.msgData.apConfig.polarityVector = polarity;
#endif
#ifdef BC2_DEV_SUPPORT
        if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
        {
            ipcMsg.msgData.apConfig.laneNum = portBC2SerdesNum[portMacNum - MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE];
        }
        else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
        {
            ipcMsg.msgData.apConfig.laneNum = portAC3SerdesNum[portMacNum - MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE];
        }
        else
        {
            return GT_NOT_IMPLEMENTED;
        }
#elif defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.laneNum = portMacNum;
#else /* if bobk */
        ipcMsg.msgData.apConfig.laneNum = portSerdesNum[portMacNum - MV_PORT_CTRL_AP_PORT_NUM_BASE];
#endif
        ipcMsg.msgData.apConfig.laneNum += relLaneNum;
        ipcMsg.msgData.apConfig.pcsNum = ipcMsg.msgData.apConfig.laneNum;
        ipcMsg.msgData.apConfig.macNum = ipcMsg.msgData.apConfig.phyPortNum;

        ipcMsg.msgData.apConfig.advMode = 0;
        AP_PRINT_MAC(("mvHwsIpcRequestSet mode:0x%x\n", portMode));
        switch(portMode & 0xFFFF)
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
        case _40GBase_KR4:
            AP_CTRL_40GBase_KR4_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _40GBase_CR4:
            AP_CTRL_40GBase_CR4_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _100GBase_KR4:
            AP_CTRL_100GBase_KR4_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _100GBase_CR4:
            AP_CTRL_100GBase_CR4_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _25GBase_KR:
            AP_CTRL_25GBase_KR1_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _25GBase_CR:
            AP_CTRL_25GBase_CR1_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _25GBase_KR_S:
            AP_CTRL_25GBase_KR1S_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _25GBase_CR_S:
            AP_CTRL_25GBase_CR1S_SET(ipcMsg.msgData.apConfig.advMode, 1);
            break;
        case _25GBase_KR_C:
            AP_CTRL_25GBase_KR_CONSORTIUM_SET(ipcMsg.msgData.apConfig.advMode, 1);
            consortium = GT_TRUE;
            break;
        case _50GBase_KR2_C:
            AP_CTRL_50GBase_KR2_CONSORTIUM_SET(ipcMsg.msgData.apConfig.advMode, 1);
            consortium = GT_TRUE;
            if((fecAbil & AP_ST_HCD_FEC_RES_FC) &&
               ((portMacNum % 4) != 0))
            {
                /* only slot 0 in MSPG can work with 50G and FC-FEC mode */
                AP_PRINT_MAC(("mvHwsIpcApRequestSet fc fec is not supported on port %d speed 50000\n",portMacNum));
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case _25GBase_CR_C:
            AP_CTRL_25GBase_CR_CONSORTIUM_SET(ipcMsg.msgData.apConfig.advMode, 1);
            consortium = GT_TRUE;
            break;
        case _50GBase_CR2_C:
            AP_CTRL_50GBase_CR2_CONSORTIUM_SET(ipcMsg.msgData.apConfig.advMode, 1);
            consortium = GT_TRUE;
            if((fecAbil & AP_ST_HCD_FEC_RES_FC) &&
               ((portMacNum % 4) != 0))
            {
                /* only slot 0 in MSPG can work with 50G and FC-FEC mode */
                AP_PRINT_MAC(("mvHwsIpcApRequestSet fc fec is not supported on port %d speed 50000\n",portMacNum));
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            return GT_FAIL;
        }
        ipcMsg.msgData.apConfig.options = 0x200; /* [09:09] loopback Enable */

        /*
        ** Options
        ** [00:00] Flow Control Pause Enable
        ** [01:01] Flow Control Asm Direction
        ** [02:02] Remote Flow Control Pause Enable
        ** [03:03] Remote Flow Control Asm Direction
        ** [04:04] FEC Suppress En
        ** [05:05] FEC Request
        ** [06:07] Reserved  = RS-FEC requested, BASE-R FEC requested (for 25G IEEE)
        ** [09:09] loopback Enable
        ** [10:10]consortium (RS-FEC) ability (F1)
        ** [11:11]consortium (BASE-R FEC) ability (F2)
        ** [12:12]consortium (RS-FEC) request (F3)
        ** [13:13]consortium (BASE-R FEC) request (F4)
        ** [14:15] Reserved
        */

        switch(fecAbil & 0xF)
        {
        case AP_ST_HCD_FEC_RES_FC: /*fc fec*/
            if (consortium) {
                fec = 2;
            } else {
                fec = 1;
            }
            break;
        case AP_ST_HCD_FEC_RES_RS: /*RS fec*/
            if (consortium) {
                fec = 1;
            } else {
                /*TODO - add RS fec to IEEE*/
            }
            break;
        case AP_ST_HCD_FEC_RES_BOTH: /*both fec*/
            if (consortium) {
                fec = 3;
            } else {
                /*TODO - add RS fec to IEEE*/
            }
            break;
        case AP_ST_HCD_FEC_RES_NONE: /*disable fec*/
        default:
            fec = 0;
            break;
        }

        if (consortium) {
            AP_CTRL_FEC_ABIL_CONSORTIUM_SET(ipcMsg.msgData.apConfig.options, fec);
        } else {
            AP_CTRL_FEC_ABIL_SET(ipcMsg.msgData.apConfig.options, fec);
        }

        switch(fecReq & 0xF)
        {
        case AP_ST_HCD_FEC_RES_FC: /*fc fec*/
            if (consortium) {
                fec = 2;
            } else {
                fec = 1;
            }
            break;
        case AP_ST_HCD_FEC_RES_RS: /*RS fec*/
            if (consortium) {
                fec = 1;
            } else {
                /*TODO - add RS fec to IEEE*/
            }
            break;
        case AP_ST_HCD_FEC_RES_BOTH: /*both fec*/
            return GT_FAIL;
        case AP_ST_HCD_FEC_RES_NONE: /*disable fec*/
        default:
            fec = 0;
            break;
        }
        if (consortium) {
            AP_CTRL_FEC_REQ_CONSORTIUM_SET(ipcMsg.msgData.apConfig.options, fec);
        } else {
            AP_CTRL_FEC_REQ_SET(ipcMsg.msgData.apConfig.options, fec);
        }

        AP_CTRL_FC_PAUSE_SET(ipcMsg.msgData.apConfig.options, fcPause);
        AP_CTRL_FC_ASM_SET(ipcMsg.msgData.apConfig.options, fcAsmDir);

        return mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);

}

