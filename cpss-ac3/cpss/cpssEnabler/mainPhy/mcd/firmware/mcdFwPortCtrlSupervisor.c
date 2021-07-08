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
* @file mcdFwPortCtrlSupervisor.c
*
* @brief Port Control Supervisor State Machine
*
* @version   1
********************************************************************************
*/
#include <mcdFwPortCtrlInc.h>
#ifdef BV_DEV_SUPPORT
#include <mcdApiTypes.h>
#include <mcdHwSerdesCntl.h>
extern MCD_DEV mcdDevDb;
#endif
static MCD_PORT_CTRL_COMMON *infoPtr;
static GT_U32 spvHighMsgCount = 0;
static GT_U32 spvLowMsgCount  = 0;

#ifdef MV_SERDES_AVAGO_IP
#ifdef BV_DEV_SUPPORT
extern void mcdSerdesAaplAddrGet(MCD_DEV_PTR pDev,unsigned int  *devAddr, unsigned int  *devSize);

extern void mcdSerdesAaplInit( MCD_DEV_PTR  pDev);

extern MCD_STATUS mcdSquelchRxClockParamsSet
(
    MCD_U32                     phyPortNum,
    MCD_U32                     portMode,
    MCD_RX_CLOCK_CTRL_TYPE      clckCtrl
);
#else

extern void mvHwsAvagoAaplAddrGet(unsigned char devNum, unsigned int *devAddr,
                                  unsigned int *devSize);
extern void mvHwsAvagoAaplInit(unsigned char devNum);

#endif
#ifndef MV_HWS_AVAGO_NO_VOS_WA
extern GT_STATUS mvHwsAvagoSerdesManualVosParamsSet(unsigned char devNum,
                                                    unsigned long *vosParamsPtr,
                                                    unsigned char bufferIdx,
                                                    unsigned char bufferLength);
extern GT_STATUS mvHwsAvagoSerdesVosOverrideModeSet(GT_U8 devNum, GT_BOOL vosOverride);
void mvHwsAvagoAaplInit(unsigned char devNum);
#endif /* #ifndef MV_HWS_AVAGO_NO_VOS_WA */
#endif /*MV_SERDES_AVAGO_IP*/
#if defined(MV_HWS_FREE_RTOS) && !defined(CM3)
/* This is needed only for service CPU on MSYS */
#include <hw/common.h>
#define IPC_CACHE_INVALIDATE(_ptr,_len) armv7_dcache_wbinv_range((unsigned long)(_ptr), (_len))
#else
#define IPC_CACHE_INVALIDATE(_ptr,_len)
#endif /* MV_HWS_FREE_RTOS */
/**
* @internal mvPortCtrlSpvRoutine function
* @endinternal
*
* @brief   Supervisor process execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvPortCtrlSpvRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvPortCtrlSpv.info);
    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+=============+
        ** + Event \ State  + High Msg O1 + Low Msg O2  + Delay O3    +
        ** +================+=============+=============+=============+
        ** + High Msg       +     O1      +     O2      +    ==> O1   +
        ** +================+=============+=============+=============+
        ** + Low Msg        +   ==> O2    +   Invalid   +   Invalid   +
        ** +================+=============+=============+=============+
        ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
        ** +================+=============+=============+=============+
        */
        if (mvPortCtrlSpv.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvPortCtrlSpv.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Ctrl, Supervisor Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
    }
}

/**
* @internal mvPortCtrlSpvHighMsgExecute function
* @endinternal
*
* @brief   Supervisor High priority message process state processing
*         - Handle "Pizza Arbbiter" Configuration
*         Read message from high priority queue
*         Message received
*         Update database
*         Add Log entry
*/
void mvPortCtrlSpvHighMsgExecute(MCD_IPC_CTRL_MSG_STRUCT *recvMsg)
{
    MCD_IPC_PORT_INFO_STRUCT *msgParams;

    /* Extract target port from IPC message and update system config valid status */
    /* System configuration use the format of Port Init message */
    msgParams = (MCD_IPC_PORT_INFO_STRUCT*)&(recvMsg->msgData);

    if (recvMsg->ctrlMsgType == MCD_IPC_PORT_AP_SYS_CFG_VALID_MSG)
    {
        mvHwsPortCtrlApPortMng.apPortTimer[msgParams->phyPortNum].sysCfStatus = PORT_SM_SERDES_SYSTEM_VALID;
        mvPortCtrlLogAdd(SPV_LOG(SPV_HIGH_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, REG_PORT));
        /* TODO - Add handling of system state DISABLE / ENABLE */

        spvHighMsgCount++;
        if (spvHighMsgCount >= mvPortCtrlSpv.highMsgThreshold)
        {
            spvHighMsgCount = 0;
            infoPtr->event = SPV_LOW_MSG_EVENT;
        }
    }
}

/**
* @internal mvPortCtrlSpvHighMsg function
* @endinternal
*
* @brief   Supervisor High priority message process state execution
*/
void mvPortCtrlSpvHighMsg(void)
{
    MCD_IPC_CTRL_MSG_STRUCT recvMsg;

    infoPtr->state = SPV_HIGH_MSG_PROCESS_STATE;
    osMemSet(&recvMsg, 0, sizeof(recvMsg));

    /* Process messages from High priority IPC message queue */
    if (mvHwsIpcRequestGet(MCD_IPC_HIGH_PRI_QUEUE,
                           sizeof(MCD_IPC_CTRL_MSG_STRUCT),
                           (char*)&recvMsg) != GT_OK)
    {
        spvHighMsgCount = 0;
        infoPtr->event = SPV_LOW_MSG_EVENT;
    }
    else
    {
         mvPortCtrlSpvHighMsgExecute(&recvMsg);
    }
}

/**
* @internal mvPortCtrlSpvLowMsgExecute function
* @endinternal
*
* @brief   Supervisor Low priority message process state processing
*         - Validate system process can receive messgae
*         In case Valid
*         Handle Port Init / Reset IPC Message
*         Add log entry
*         Send message to Port process
*         Handle AP Port IPC Message
*         Add log entry
*         Send message to AP Detection process
*         Handle General IPC Message
*         Add log entry
*         Send message to General process
*         Message not received
*         Trigger state change
*         - Not Valid
*         Trigger state change
*/
void mvPortCtrlSpvLowMsgExecute(MCD_IPC_CTRL_MSG_STRUCT *recvMsg)
{
    MCD_IPC_PORT_INIT_DATA_STRUCT *msgParams;
    MCD_IPC_REPLY_MSG_STRUCT replyMsg;
    unsigned int avagoAaplAddr;
    unsigned int avagoAaplSize;
    GT_U32 avagoGuiState;

    msgParams = (MCD_IPC_PORT_INIT_DATA_STRUCT*)&(recvMsg->msgData);
    switch (recvMsg->ctrlMsgType)
    {
    case MCD_IPC_RX_CLOCK_CTRL_PIN_SET_MSG:
        replyMsg.returnCode = mcdSquelchRxClockParamsSet(recvMsg->msgData.rxClockControl.phyPortNum,
                                                         recvMsg->msgData.rxClockControl.portMode,
                                                         recvMsg->msgData.rxClockControl.clckCtrlPin);
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MCD_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
#ifndef MV_HWS_AVAGO_NO_VOS_WA
        replyMsg.returnCode = mvHwsAvagoSerdesManualVosParamsSet(0,
                                &(recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferPtr[0]),
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferIdx,
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferLength);
#else
        replyMsg.returnCode = GT_OK;
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
        case MCD_IPC_VOS_OVERRIDE_MODE_SET_MSG:
#ifndef MV_HWS_AVAGO_NO_VOS_WA
        replyMsg.returnCode = mvHwsAvagoSerdesVosOverrideModeSet(0,
                                recvMsg->msgData.vosOverrideModeSet.vosOverrideMode);
#else
        replyMsg.returnCode = GT_OK;
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
        case MCD_IPC_PORT_AVAGO_SERDES_INIT_MSG:

#ifdef BV_DEV_SUPPORT
        mcdSerdesAaplAddrGet(&mcdDevDb, &avagoAaplAddr, &avagoAaplSize);
#elif MV_SERDES_AVAGO_IP
        mvHwsAvagoAaplAddrGet(0, &avagoAaplAddr, &avagoAaplSize);
#else
        avagoAaplAddr = 0;
        avagoAaplSize = 0;
#endif /* MV_SERDES_AVAGO_IP */
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        replyMsg.readData.portAvagoAaplGet.addr = avagoAaplAddr;
        replyMsg.readData.portAvagoAaplGet.size = avagoAaplSize;
        /* Host is about to deliver the full content of AAPL structure.
         * Need to invalidate this area to be able to see content.*/
        IPC_CACHE_INVALIDATE(avagoAaplAddr, avagoAaplSize);
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MCD_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
#ifdef MV_SERDES_AVAGO_IP
#ifndef BV_DEV_SUPPORT
        mvHwsAvagoAaplInit(0);
#else
        mcdSerdesAaplInit(&mcdDevDb);
#endif
#endif
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MCD_IPC_PORT_AVAGO_GUI_SET_MSG:
        avagoGuiState = recvMsg->msgData.avagoGuiSet.state;
        avagoGuiState &= MV_PORT_CTRL_AVAGO_GUI_MASK;
        mvHwsApDetectStateSet(avagoGuiState);
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;

    /* Send messages to Port Mng process message queue */
    case MCD_IPC_PORT_INIT_MSG:
    case MCD_IPC_PORT_RESET_MSG:
    case MCD_IPC_PORT_RESET_EXT_MSG:
    /* Code not relevant for BobK CM3 due to space limit */
        mvPortCtrlPortSm[msgParams->phyPortNum].type = REG_PORT;
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, REG_PORT));
        mvPortCtrlProcessMsgSend(M2_PORT_MNG, recvMsg);
        break;

    case MCD_IPC_PORT_POLLING_START_MSG:
    case MCD_IPC_PORT_POLLING_STOP_MSG:
        mvPortCtrlProcessMsgSend(M2_PORT_MNG, recvMsg);
        /* immediately reply MCD_OK */
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = MCD_OK;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MCD_IPC_PORT_POLLING_GET_LINK_ON_BITMAP_MSG:
        mvPortCtrlProcessMsgSend(M2_PORT_MNG, recvMsg);
        /* reply will be sent by polling task */
        break;

    /* Send messages to AP Detection process message queue */
    case MCD_IPC_PORT_AP_STATUS_MSG:
    case MCD_IPC_PORT_AP_ENABLE_MSG:
    case MCD_IPC_PORT_AP_DISABLE_MSG:
    case MCD_IPC_PORT_AP_CFG_GET_MSG:
    case MCD_IPC_PORT_AP_STATS_MSG:
    case MCD_IPC_PORT_AP_STATS_RESET_MSG:
    case MCD_IPC_PORT_AP_INTROP_GET_MSG:
    case MCD_IPC_PORT_AP_INTROP_SET_MSG:
    case MCD_IPC_PORT_AP_DEBUG_GET_MSG:
    case MCD_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
    case MCD_IPC_PORT_SERDES_TX_CONFIG_OVERRIDE_MSG:
        mvPortCtrlPortSm[msgParams->phyPortNum].type = AP_PORT;
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
        mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, recvMsg);
        break;

    case MCD_IPC_EXECUTE_CMDLINE_MSG:
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 0xFFFFFFFF, REG_PORT));
        mvPortCtrlProcessMsgSend(M6_GENERAL, recvMsg);
        break;

    case MCD_IPC_HWS_LOG_GET_MSG:
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, REG_PORT));
        mvPortCtrlProcessMsgSend(M6_GENERAL, recvMsg);
        break;

    /* Send messages to General process message queue */
    default:
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, REG_PORT));
        mvPortCtrlProcessMsgSend(M6_GENERAL, recvMsg);
        break;
    }
    spvLowMsgCount++;
}

/**
* @internal mvPortCtrlSpvLowMsg function
* @endinternal
*
* @brief   Supervisor Low priority message process state execution
*/
void mvPortCtrlSpvLowMsg(void)
{
    GT_STATUS                  rcode;
    GT_U32                     portMsgNum;
    GT_U32                     apMsgNum;
    GT_U32                     genMsgNum;
    MCD_IPC_CTRL_MSG_STRUCT    recvMsg;

    infoPtr->state = SPV_LOW_MSG_PROCESS_STATE;

    /* Validate system process can receive messgaes
    ** Threshold is set to max of 6 message at each processing interval
    ** under the validation that the target queue has enough space
    */
    /* Code not relevant for BobK CM3 due to space limit */
    rcode = mvPortCtrlProcessPendMsgNum(M2_PORT_MNG, &portMsgNum);
    rcode |= mvPortCtrlProcessPendMsgNum(M5_AP_PORT_DET, &apMsgNum);
    rcode |= mvPortCtrlProcessPendMsgNum(M6_GENERAL, &genMsgNum);
    if (rcode != GT_OK)
    {
        mvPcPrintf("Error, Supervisor, Failed to get number of pending messages\n");
        return;
    }
    /* Code not relevant for BobK CM3 due to space limit */
    if ((apMsgNum       < MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD) &&
        (genMsgNum      < MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD) &&
        (spvLowMsgCount < mvPortCtrlSpv.lowMsgThreshold))
    {
        /* Process messages from Low priority IPC message queue */
        if (mvHwsIpcRequestGet(MCD_IPC_LOW_PRI_QUEUE,
                               sizeof(MCD_IPC_CTRL_MSG_STRUCT),
                               (char*)&recvMsg) != GT_OK)
        {
            spvLowMsgCount = 0;
            infoPtr->event = SPV_DELAY_EVENT;
        }
        else
        {
           mvPortCtrlSpvLowMsgExecute(&recvMsg);
        }
    }
    else
    {
        spvLowMsgCount = 0;
        infoPtr->event = SPV_DELAY_EVENT;
    }
}

/**
* @internal mvPortCtrlSpvDelay function
* @endinternal
*
* @brief   Superviosr message delay state execution
*         - Execute delay
*         - Trigger state change
*/
void mvPortCtrlSpvDelay(void)
{
    infoPtr->state = SPV_DELAY_STATE;
    mvPortCtrlProcessDelay(infoPtr->delayDuration);
    infoPtr->event = SPV_HIGH_MSG_EVENT;
}



