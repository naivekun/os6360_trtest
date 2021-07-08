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
* @file mvHwsPortCtrlAp.c
*
* @brief Port Control AP Detection State Machine
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#ifdef BV_DEV_SUPPORT
#include "mcdApiTypes.h"
#include "mcdApiRegs.h"
#include "mcdHwCntl.h"
#include "mcdHwSerdesCntl.h"
#include "mcdAPI.h"
#include "mcdDiagnostics.h"
#include "mcdAPIInternal.h"
#include "mcdFwDownload.h"
#include "mcdInitialization.h"
extern MCD_DEV mcdDevDb;
#endif

#include <mvHwsPortCtrlAp.h>
extern GT_STATUS mvHwsGetPortParameters(GT_U32                  portNum,
                                        MV_HWA_AP_PORT_MODE     apPortMode,
                                        MV_HWS_PORT_INIT_PARAMS *portParams);

GT_U32 mvHwsApDetectExecutionEnabled = 0;
GT_BOOL mvHwsApPortEnableCtrl = GT_TRUE;

#ifdef ALDRIN_DEV_SUPPORT
GT_U8 refClockSourceDb[ALDRIN_PORTS_NUM];
#endif

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_U8 refClockSourceDb[MV_HWS_MAX_PORT_NUM];
#endif

static MV_HWS_PORT_CTRL_COMMON *infoPtr;

/**
* @internal mvHwsApDetectStateSet function
* @endinternal
*
* @brief   AP Detection process enable / disable set
*/
void mvHwsApDetectStateSet(GT_U32 state)
{
    mvHwsApDetectExecutionEnabled = state;
}

/**
* @internal mvHwsApPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training � Host or Service CPU (default value � service CPU)
*/
void mvHwsApPortEnableCtrlSet(GT_BOOL portEnableCtrl)
{
    mvHwsApPortEnableCtrl = portEnableCtrl;
}


/**
* @internal mvHwsApPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training � Host or Service CPU (default value � service CPU
*/
void mvHwsApPortEnableCtrlGet(GT_BOOL *portEnableCtrl)
{
    *portEnableCtrl = mvHwsApPortEnableCtrl;
}


/**
* @internal mvHwsApDetectRoutine function
* @endinternal
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvHwsApDetectRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvHwsPortCtrlApPortDetect.info);

    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+=============+
        ** + Event \ State  +  Active O1  +   Msg O2    +  Delay O3   +
        ** +================+=============+=============+=============+
        ** + Active         +     O1      +   Invalid   +    ==> O1   +
        ** +================+=============+=============+=============+
        ** + Msg            +   ==> O2    +     O2      +   Invalid   +
        ** +================+=============+=============+=============+
        ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
        ** +================+=============+=============+=============+
        */
        if (mvHwsPortCtrlApPortDetect.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvHwsPortCtrlApPortDetect.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Detect, Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
    }
}

/**
* @internal mvPortCtrlApPortDetectionActiveExec function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Exract port state
*         - Execute state functionality
*/
void mvPortCtrlApPortDetectionActiveExec(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    switch (apSm->state)
    {
    case AP_PORT_SM_IDLE_STATE:
        return;

    case AP_PORT_SM_INIT_STATE:
        mvApPortInit(portIndex);
        break;

    case AP_PORT_SM_TX_DISABLE_STATE:
        mvApPortTxDisable(portIndex);
        break;

    case AP_PORT_SM_RESOLUTION_STATE:
        mvApPortResolution(portIndex);
        break;

    case AP_PORT_SM_ACTIVE_STATE:
        mvApPortLinkUp(portIndex);
        break;

    case AP_PORT_SM_DELETE_STATE:
    case AP_PORT_SM_DISABLE_STATE:
        mvApPortDeleteValidate(portIndex);
        break;

    default:
        mvPcPrintf("Error, AP Detect, PortIndex-%d Invalid state %d!!!\n", portIndex, apSm->state);
        break;
    }
}

/**
* @internal mvPortCtrlApDetectActive function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Scan all ports and call port execution function
*/
void mvPortCtrlApDetectActive(void)
{
    GT_U8 portIndex;
    GT_U8 maxApPortNum = 0;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_BC2_MAX_AP_PORT_NUM;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_AC3_MAX_AP_PORT_NUM;
    }
    else
    {
        maxApPortNum = 0;
    }
#else
    maxApPortNum = MV_PORT_CTRL_MAX_AP_PORT_NUM;
#endif

    infoPtr->state = AP_PORT_DETECT_ACTIVE_PORT_STATE;

    for (portIndex = 0; portIndex < maxApPortNum; portIndex++)
    {
        mvPortCtrlApPortDetectionActiveExec(portIndex);
    }

    infoPtr->event = AP_PORT_DETECT_DELAY_EVENT;
}

/**
* @internal mvPortCtrlApPortMsgReply function
* @endinternal
*
* @brief   AP Port mng info message reply
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlApPortMsgReply(MV_HWS_IPC_REPLY_MSG_STRUCT *replyMsg, GT_U8 queueId)
{
    /* Build and Send IPC reply */
    CHECK_STATUS(mvHwsIpcReplyMsg(queueId, replyMsg));

    return GT_OK;
}

/**
* @internal mvPortCtrlApPortDetectionMsgExec function
* @endinternal
*
* @brief   AP Detect message state execution
*         - Extract Message type, Message params, and Port index
*         - Handle AP Enable message
*         AP State == IDLE
*         Extract and update message parameters
*         Execute port initial configuration
*         Trigger state change
*         AP State != IDLE
*         Send reply error message to calling applicaion
*         - Handle AP Disable message
*         AP State != IDLE
*         Execute port delete
*         AP State == IDLE
*         Print error message
*/
void mvPortCtrlApPortDetectionMsgExec(MV_HWS_IPC_CTRL_MSG_STRUCT *msg)
{
    GT_U8 msgType;
    GT_U8 portIndex;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT *msgParams;
    MV_HWS_AP_SM_INFO *apSm;
    MV_HWS_AP_SM_STATS *apStats;
    MV_HWS_AP_DETECT_ITEROP *apIntrop;
    MV_HWS_IPC_PORT_AP_INTROP_STRUCT  *apIntropSetMsg;
    MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT  *portEnableCtrlSetMsg;
    GT_U8 portNumBase = 0;
    MV_HWS_PORT_CTRL_PORT_SM       *portSm;
#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
    MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC *portRxCfgOverrideMsg;
    GT_U8   i;
#endif
    MV_HWS_IPC_REPLY_MSG_STRUCT replyMsg;

    msgType = msg->ctrlMsgType;
    msgParams = (MV_HWS_IPC_PORT_AP_DATA_STRUCT*)&(msg->msgData);

    replyMsg.replyTo    = msgType;
    replyMsg.returnCode = GT_OK;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE;
    }
    else
    {
        portNumBase = 0;
    }
#else
    portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;
#endif
    /* Validate input port number */
    if ((msgParams->phyPortNum < portNumBase) &&
        (msgType != MV_HWS_IPC_PORT_AP_INTROP_GET_MSG) &&
        (msgType != MV_HWS_IPC_PORT_AP_INTROP_SET_MSG) &&
        (msgType != MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG))
    {
        AP_PRINT_MAC(("mvPortCtrlApPortDetectionMsgExec GT_BAD_PARAM msgParams->phyPortNum %d portNumBase %d\n",msgParams->phyPortNum, portNumBase));
        replyMsg.returnCode = GT_BAD_PARAM;
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        return;
    }

    portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(msgParams->phyPortNum, portNumBase);
    apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    switch (msgType)
    {
    case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
#if defined (ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
        refClockSourceDb[msgParams->phyPortNum] = msgParams->refClockSrcParams;
#endif
        if (apSm->state == AP_PORT_SM_IDLE_STATE)
        {
#ifdef BOBK_DEV_SUPPORT
            MV_HWS_PORT_INIT_PARAMS portParams;
#endif

#ifdef BV_DEV_SUPPORT
            mcdDevDb.apCfg[portIndex].apLaneNum = msgParams->laneNum;
            mcdDevDb.apCfg[portIndex].modesVector = msgParams->advMode;
            mcdDevDb.apCfg[portIndex].fcPause = AP_CTRL_FC_PAUSE_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].fcAsmDir = AP_CTRL_FC_ASM_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].fecSup = AP_CTRL_FEC_ABIL_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].fecReq = AP_CTRL_FEC_REQ_GET(msgParams->options);
#endif
            /*apSm->queueId = msg->msgQueueId;*/
            AP_CTRL_QUEUEID_SET(apSm->ifNum, msg->msgQueueId)
            apSm->portNum = msgParams->phyPortNum;
            AP_CTRL_LANE_SET(apSm->ifNum, msgParams->laneNum);
#ifdef BOBK_DEV_SUPPORT
            mvHwsGetPortParameters(msgParams->macNum, Port_10GBase_R, &portParams);
            AP_CTRL_PCS_SET(apSm->ifNum, msgParams->pcsNum + (portParams.firstLaneNum - msgParams->laneNum));
#else
            AP_CTRL_PCS_SET(apSm->ifNum, msgParams->pcsNum);
#endif
            AP_CTRL_MAC_SET(apSm->ifNum, msgParams->macNum);
            apSm->capability = 0;
            AP_CTRL_ADV_ALL_SET(apSm->capability, msgParams->advMode);
            apSm->options = 0;
            AP_CTRL_LB_EN_SET(apSm->options, AP_CTRL_LB_EN_GET(msgParams->options));
            AP_CTRL_FC_PAUSE_SET(apSm->options, AP_CTRL_FC_PAUSE_GET(msgParams->options));
            AP_CTRL_FC_ASM_SET(apSm->options, AP_CTRL_FC_ASM_GET(msgParams->options));
            AP_CTRL_FEC_ABIL_SET(apSm->options, AP_CTRL_FEC_ABIL_GET(msgParams->options));
            AP_CTRL_FEC_REQ_SET(apSm->options, AP_CTRL_FEC_REQ_GET(msgParams->options));
            AP_CTRL_FEC_ABIL_CONSORTIUM_SET(apSm->options, AP_CTRL_FEC_ABIL_CONSORTIUM_GET(msgParams->options));
            AP_CTRL_FEC_REQ_CONSORTIUM_SET(apSm->options, AP_CTRL_FEC_REQ_CONSORTIUM_GET(msgParams->options));
            AP_CTRL_FEC_ADVANCED_REQ_SET(apSm->options, AP_CTRL_FEC_ADVANCED_REQ_GET(msgParams->options));
#if defined(BC3_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
            AP_CTRL_CTLE_BIAS_VAL_SET(apSm->options, AP_CTRL_CTLE_BIAS_VAL_GET(msgParams->options));
#endif
            AP_PRINT_MAC(("detect port:%d idx:%d capability:0x%x advMode:0x%x options:0x%x\n",msgParams->phyPortNum, portIndex, apSm->capability, msgParams->advMode, apSm->options));
            apSm->polarityVector = msgParams->polarityVector;

            /* Initial configuration */
            mvHwsInitialConfiguration(portIndex);

            apSm->status = AP_PORT_START_EXECUTE;
            apSm->state = AP_PORT_SM_INIT_STATE;
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is under execution[0x%x], AP Enable is not valid!!!\n", msgParams->phyPortNum, apSm->state);
            replyMsg.returnCode = GT_CREATE_ERROR;
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        break;

    case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
        if (apSm->state != AP_PORT_SM_IDLE_STATE)
        {
            mvApPortDeleteMsg(portIndex);
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is NOT under execution, AP Disable is not valid!!!\n", msgParams->phyPortNum);
            replyMsg.returnCode = GT_FAIL;
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        break;

    case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
        replyMsg.readData.portApCfgGet.ifNum      = apSm->ifNum;
        replyMsg.readData.portApCfgGet.capability = apSm->capability;
        replyMsg.readData.portApCfgGet.options    = apSm->options;
        AP_PRINT_MAC(("detect MV_HWS_IPC_PORT_AP_CFG_GET_MSG port:%d idx:%d capability:0x%x options:0x%x\n",msgParams->phyPortNum, portIndex, apSm->capability, apSm->options));
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

        case MV_HWS_IPC_PORT_AP_STATUS_MSG:

          /* return AP disabled only after both state machines reached idle state and application asked to disable AP on port */
        replyMsg.readData.portApStatusGet.state       = ((PORT_SM_IDLE_STATE == portSm->state) && (AP_PORT_SM_IDLE_STATE == apSm->state))?
                                                         0 : (AP_PORT_SM_IDLE_STATE == apSm->state)? 1: apSm->state;
        replyMsg.readData.portApStatusGet.status      = apSm->status;
        replyMsg.readData.portApStatusGet.laneNum     = AP_CTRL_LANE_GET(apSm->ifNum);
        replyMsg.readData.portApStatusGet.ARMSmStatus = apSm->ARMSmStatus;
        replyMsg.readData.portApStatusGet.hcdStatus   = apSm->hcdStatus;
        AP_PRINT_MAC(("detect MV_HWS_IPC_PORT_AP_STATUS_MSG port:%d idx:%d capability:0x%x options:0x%x hcdStatus:0x%x\n",msgParams->phyPortNum, portIndex, apSm->capability, apSm->options,apSm->hcdStatus));
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATS_MSG:
        replyMsg.readData.portApStatsGet.txDisCnt          = apStats->txDisCnt;
        replyMsg.readData.portApStatsGet.abilityCnt        = apStats->abilityCnt;
        replyMsg.readData.portApStatsGet.abilitySuccessCnt = apStats->abilitySuccessCnt;
        replyMsg.readData.portApStatsGet.linkFailCnt       = apStats->linkFailCnt;
        replyMsg.readData.portApStatsGet.linkSuccessCnt    = apStats->linkSuccessCnt;
        replyMsg.readData.portApStatsGet.hcdResoultionTime = apStats->hcdResoultionTime;
        replyMsg.readData.portApStatsGet.linkUpTime        = apStats->linkUpTime;
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
        mvPortCtrlDbgStatsReset(msgParams->phyPortNum);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
        apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
        replyMsg.readData.portApIntropGet.txDisDuration          = apIntrop->txDisDuration;
        replyMsg.readData.portApIntropGet.abilityDuration        = apIntrop->abilityDuration;
        replyMsg.readData.portApIntropGet.abilityMaxInterval     = apIntrop->abilityMaxInterval;
        replyMsg.readData.portApIntropGet.abilityFailMaxInterval = apIntrop->abilityFailMaxInterval;
        replyMsg.readData.portApIntropGet.apLinkDuration         = apIntrop->apLinkDuration;
        replyMsg.readData.portApIntropGet.apLinkMaxInterval      = apIntrop->apLinkMaxInterval;
        replyMsg.readData.portApIntropGet.pdLinkDuration         = apIntrop->pdLinkDuration;
        replyMsg.readData.portApIntropGet.pdLinkMaxInterval      = apIntrop->pdLinkMaxInterval;
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
        apIntropSetMsg  = (MV_HWS_IPC_PORT_AP_INTROP_STRUCT*)&(msg->msgData);
        mvPortCtrlDbgIntropCfg(apIntropSetMsg);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
        mvPortCtrlLogInfoGet(&replyMsg.readData.logGet.fwBaseAddr,
                &replyMsg.readData.logGet.logBaseAddr,
                &replyMsg.readData.logGet.logPointerAddr,
                &replyMsg.readData.logGet.logCountAddr,
                &replyMsg.readData.logGet.logResetAddr);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
        portEnableCtrlSetMsg = (MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT*)&(msg->msgData);
        mvHwsApPortEnableCtrlSet((GT_BOOL)portEnableCtrlSetMsg->portEnableCtrl);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

   case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
        mvHwsApPortEnableCtrlGet((GT_BOOL*)&replyMsg.readData.portEnableCtrlGet.portEnableCtrl);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
   case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
        portRxCfgOverrideMsg = (MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC*)&(msg->msgData);
        if (portRxCfgOverrideMsg->rxCfg.serdesSpeed == _10_3125G) {
            i = MV_HWS_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS;
        }
        else
        {
            i = MV_HWS_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS;
        }
        portSm->apPortRxConfigOverride[i].bandLoopWidth = (GT_U8)((portRxCfgOverrideMsg->rxCfg.ctleParams.bandWidth & 0xF) |
            ((portRxCfgOverrideMsg->rxCfg.ctleParams.loopBandwidth & 0xF) << MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_LOOPWIDTH_SHIFT));
        portSm->apPortRxConfigOverride[i].dcGain = (GT_U8)(portRxCfgOverrideMsg->rxCfg.ctleParams.dcGain & 0xFF);
        portSm->apPortRxConfigOverride[i].lfHf = (GT_U8)((portRxCfgOverrideMsg->rxCfg.ctleParams.lowFrequency & 0xF) |
                ((portRxCfgOverrideMsg->rxCfg.ctleParams.highFrequency& 0xF)<< MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_HF_SHIFT));
        portSm->apPortRxConfigOverride[i].squelch = (GT_U16)(portRxCfgOverrideMsg->rxCfg.ctleParams.squelch & 0xFFFF);
        portSm->apPortRxConfigOverride[i].fieldOverrideBmp = (GT_U16)(portRxCfgOverrideMsg->rxCfg.fieldOverrideBmp & 0xFFFF);
        portSm->apPortRxConfigOverride[i].etlMinDelay = (GT_U8)(portRxCfgOverrideMsg->rxCfg.etlParams.etlMinDelay & 0xFF);
        portSm->apPortRxConfigOverride[i].etlMaxDelay = (GT_U8)((portRxCfgOverrideMsg->rxCfg.etlParams.etlMaxDelay & 0x7F) |
            ((portRxCfgOverrideMsg->rxCfg.etlParams.etlEnableOverride & 0x1) << MV_HWS_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT));
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;
#endif
    }
}

/**
* @internal mvPortCtrlApDetectMsg function
* @endinternal
*
* @brief   AP Detect message state execution
*         - Read message from AP detection message queue
*         Message received
*         Execute message to completion
*         Message not received
*         Trigger state change
*/
void mvPortCtrlApDetectMsg(void)
{
    GT_STATUS rcode;

    MV_HWS_IPC_CTRL_MSG_STRUCT recvMsg;

    osMemSet(&recvMsg, 0, sizeof(recvMsg));
    infoPtr->state = AP_PORT_DETECT_MSG_PROCESS_STATE;

    do
    {
        rcode = mvPortCtrlProcessMsgRecv(M5_AP_PORT_DET, MV_PROCESS_MSG_RX_DELAY, &recvMsg);
        if (rcode == GT_OK)
        {
            mvPortCtrlApPortDetectionMsgExec(&recvMsg);
        }
    } while (rcode == GT_OK);

    infoPtr->event = AP_PORT_DETECT_ACTIVE_PORT_EVENT;
}

/**
* @internal mvPortCtrlApDetectDelay function
* @endinternal
*
* @brief   AP Delay state execution
*/
void mvPortCtrlApDetectDelay(void)
{
    infoPtr->state = AP_PORT_DETECT_DELAY_STATE;

    do
    {
        /* When Avago GUI is enabled, AP periodic execution is placed on hold
        ** AP process will not execute any activity
        */
        mvPortCtrlProcessDelay(mvHwsPortCtrlApPortDetect.info.delayDuration);
    } while (mvHwsApDetectExecutionEnabled == MV_PORT_CTRL_AVAGO_GUI_ENABLED);

    infoPtr->event = AP_PORT_DETECT_MSG_PROCESS_EVENT;
}



