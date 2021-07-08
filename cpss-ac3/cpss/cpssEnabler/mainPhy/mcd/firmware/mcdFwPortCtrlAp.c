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
* @file mcdFwPortCtrlAp.c
*
* @brief Port Control AP Detection State Machine
*
* @version   1
********************************************************************************
*/
#include <mcdFwPortCtrlInc.h>
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
GT_U32 mvHwsApDetectExecutionEnabled = 0;

static MCD_PORT_CTRL_COMMON *infoPtr;

extern MCD_SERDES_CONFIG_DATA serdesElectricalParams[];
extern MCD_MAN_TUNE_TX_CONFIG_OVERRIDE_DB serdesTxApOverrideParams[8][2];

/* These extern requires for MCD_STAT_AP_SM_STATUS_INSERT and MCD_STAT_AP_SM_STATE_INSERT */
extern void  mcdStatApSmStateInsert(GT_U8 portIndex, GT_U16 state);
extern void  mcdStatApSmStatusInsert(GT_U8 portIndex, GT_U16 status);

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
    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

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
        mvApPortDeleteValidate(portIndex);
        break;

    case AP_PORT_SM_AP_DISABLE_STATE:
        mvApPortDeleteMsg(portIndex);
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
GT_STATUS mvPortCtrlApPortMsgReply(MCD_IPC_REPLY_MSG_STRUCT *replyMsg, GT_U8 queueId)
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
*         Print error message and reply 'NO RESOURCE' message to calling application
*/
void mvPortCtrlApPortDetectionMsgExec(MCD_IPC_CTRL_MSG_STRUCT *msg)
{
    GT_U8 msgType;
    GT_U8 portIndex,i;
    MCD_IPC_PORT_AP_DATA_STRUCT *msgParams;
    MCD_AP_SM_INFO *apSm;
    MCD_AP_SM_STATS *apStats;
    MCD_AP_DETECT_ITEROP *apIntrop;
    MCD_IPC_PORT_AP_INTROP_STRUCT  *apIntropSetMsg;
    GT_U8 portNumBase = 0;
    MCD_PORT_CTRL_PORT_SM       *portSm;
    MCD_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STRUCT *portRxCfgOverrideMsg;
    MCD_IPC_PORT_SERDES_TX_CONFIG_OVERRIDE_STRUCT *portTxCfgOverrideMsg;

    MCD_IPC_REPLY_MSG_STRUCT replyMsg;

    msgType = msg->ctrlMsgType;
    msgParams = (MCD_IPC_PORT_AP_DATA_STRUCT*)&(msg->msgData);

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
        (msgType != MCD_IPC_PORT_AP_INTROP_GET_MSG) &&
        (msgType != MCD_IPC_PORT_AP_INTROP_SET_MSG) &&
        (msgType != MCD_IPC_PORT_AP_DEBUG_GET_MSG))
    {
        mvPcPrintf("%s GT_BAD_PARAM msgParams->phyPortNum %d portNumBase %d\n",__func__, msgParams->phyPortNum, portNumBase);
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
    case MCD_IPC_PORT_AP_ENABLE_MSG:
        if (apSm->state == AP_PORT_SM_IDLE_STATE)
        {
#ifdef BV_DEV_SUPPORT
            mcdDevDb.apCfg[portIndex].apLaneNum = msgParams->laneNum;
            mcdDevDb.apCfg[portIndex].modesVector = msgParams->advMode;
            mcdDevDb.apCfg[portIndex].retimerMode = msgParams->retimerMode;
            mcdDevDb.apCfg[portIndex].fcPause = AP_CTRL_FC_PAUSE_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].fcAsmDir = AP_CTRL_FC_ASM_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].fecSup = AP_CTRL_FEC_ABIL_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].fecReq = AP_CTRL_FEC_REQ_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].ctleBiasVal = AP_CTRL_CTLE_BIAS_VAL_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].noPpmMode = AP_CTRL_NO_PPM_MODE_GET(msgParams->options);
            mcdDevDb.apCfg[portIndex].enSdTuningApRes = msgParams->enSdTuningApRes;
            mcdDevDb.laneRemapCfg[portIndex/4].lineRxRemapVector = msgParams->rxRemapVector;
            mcdDevDb.laneRemapCfg[portIndex/4].lineTxRemapVector = msgParams->txRemapVector;
            if (msgParams->rxRemapVector != 0)
            {
                mcdDevDb.laneRemapCfg[portIndex/4].lineRemapMode = MCD_LANE_REMAP_ENABLE;
            }
            AP_PRINT_MCD2_MAC(("%s apLaneNum %d modesVector %x retimerMode %d options %x\n",__func__, msgParams->laneNum, msgParams->advMode,msgParams->retimerMode,msgParams->options));
#endif
            /*apSm->queueId = msg->msgQueueId;*/
            AP_CTRL_QUEUEID_SET(apSm->ifNum, msg->msgQueueId)
            apSm->portNum = msgParams->phyPortNum;
            apSm->options = 0;
            apSm->capability = 0;
            AP_CTRL_LANE_SET(apSm->ifNum, msgParams->laneNum);
            AP_CTRL_PCS_SET(apSm->ifNum, msgParams->pcsNum);
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
            AP_CTRL_CTLE_BIAS_VAL_SET(apSm->options,AP_CTRL_CTLE_BIAS_VAL_GET(msgParams->options));
            AP_CTRL_NO_PPM_MODE_SET(apSm->options,AP_CTRL_NO_PPM_MODE_GET(msgParams->options));
            /*TODO -  For 25GBASE-KR-S and 25GBASE-CR-S PHYs if either PHY requests RS-FEC or BASE-R FEC then
            BASE-R operation is enabled. This is because 25GBASE-KR-S and 25GBASE-CR-S PHYs do not support
            RS-FEC operation*/
            apSm->polarityVector = msgParams->polarityVector;
            /* change default polarity */
            for(i = 0; i < 4; i++)
            {
                serdesElectricalParams[(portIndex & 4 )+ i].txPolarity = ((msgParams->polarityVector >> (2*i+1)) & 1);
                serdesElectricalParams[(portIndex & 4)+ i].rxPolarity = ((msgParams->polarityVector >> (2*i)) & 1);
            }
            /* Initial configuration */
            mvHwsInitialConfiguration(portIndex);
            apStats->portEnableTime = mvPortCtrlCurrentTs();

            apSm->status = AP_PORT_START_EXECUTE;
            MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_START_EXECUTE);
            apSm->state = AP_PORT_SM_INIT_STATE;
            MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_INIT_STATE);
            AP_PRINT_MCD_MAC(("detect port:%d idx:%d capability:0x%x advMode:0x%x options:0x%x\n",msgParams->phyPortNum, portIndex, apSm->capability, msgParams->advMode, apSm->options));
        }
        /* In case the state is AP_PORT_SM_AP_DISABLE_STATE - means the AP task still running AP DELETE command on this port */
        else if (apSm->state == AP_PORT_SM_AP_DISABLE_STATE) {
            replyMsg.returnCode = GT_NO_RESOURCE;
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is under execution, AP Enable is not valid!!!\n", msgParams->phyPortNum);
            replyMsg.returnCode = GT_CREATE_ERROR;
        }
        break;

    case MCD_IPC_PORT_AP_DISABLE_MSG:
        if (apSm->state != AP_PORT_SM_IDLE_STATE)
        {
            AP_PRINT_MCD_MAC(("%s MCD_IPC_PORT_AP_DISABLE_MSG port %d\n",__func__, portIndex));
            mvApPortDeleteMsg(portIndex);
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is NOT under execution, AP Disable is not valid!!!\n", msgParams->phyPortNum);
            replyMsg.returnCode = GT_NO_RESOURCE;
        }
        break;

    case MCD_IPC_PORT_AP_CFG_GET_MSG:
        replyMsg.readData.portApCfgGet.ifNum      = apSm->ifNum;
        replyMsg.readData.portApCfgGet.capability = apSm->capability;
        replyMsg.readData.portApCfgGet.options    = apSm->options;
        break;

    case MCD_IPC_PORT_AP_STATUS_MSG:

          /* return AP disabled only after both state machines reached idle state and application asked to disable AP on port */
        replyMsg.readData.portApStatusGet.state       = ((PORT_SM_IDLE_STATE == portSm->state) && (AP_PORT_SM_IDLE_STATE == apSm->state))?
                                                         0 : (AP_PORT_SM_IDLE_STATE == apSm->state)? 1: apSm->state;
        replyMsg.readData.portApStatusGet.status      = apSm->status;
        replyMsg.readData.portApStatusGet.laneNum     = AP_CTRL_LANE_GET(apSm->ifNum);
        replyMsg.readData.portApStatusGet.ARMSmStatus = apSm->ARMSmStatus;
        replyMsg.readData.portApStatusGet.hcdStatus   = apSm->hcdStatus;
        break;

    case MCD_IPC_PORT_AP_STATS_MSG:
        replyMsg.readData.portApStatsGet.txDisCnt          = apStats->txDisCnt;
        replyMsg.readData.portApStatsGet.abilityCnt        = apStats->abilityCnt;
        replyMsg.readData.portApStatsGet.abilitySuccessCnt = apStats->abilitySuccessCnt;
        replyMsg.readData.portApStatsGet.linkFailCnt       = apStats->linkFailCnt;
        replyMsg.readData.portApStatsGet.linkSuccessCnt    = apStats->linkSuccessCnt;
        replyMsg.readData.portApStatsGet.hcdResoultionTime = apStats->hcdResoultionTime;
        replyMsg.readData.portApStatsGet.linkUpTime        = apStats->linkUpTime;
        break;

    case MCD_IPC_PORT_AP_STATS_RESET_MSG:
        mvPortCtrlDbgStatsReset(msgParams->phyPortNum);
        break;

    case MCD_IPC_PORT_AP_INTROP_GET_MSG:
        apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
        replyMsg.readData.portApIntropGet.txDisDuration          = apIntrop->txDisDuration;
        replyMsg.readData.portApIntropGet.abilityDuration        = apIntrop->abilityDuration;
        replyMsg.readData.portApIntropGet.abilityMaxInterval     = apIntrop->abilityMaxInterval;
        replyMsg.readData.portApIntropGet.abilityFailMaxInterval = apIntrop->abilityFailMaxInterval;
        replyMsg.readData.portApIntropGet.apLinkDuration         = apIntrop->apLinkDuration;
        replyMsg.readData.portApIntropGet.apLinkMaxInterval      = apIntrop->apLinkMaxInterval;
        replyMsg.readData.portApIntropGet.pdLinkDuration         = apIntrop->pdLinkDuration;
        replyMsg.readData.portApIntropGet.pdLinkMaxInterval      = apIntrop->pdLinkMaxInterval;
        break;

    case MCD_IPC_PORT_AP_INTROP_SET_MSG:
        apIntropSetMsg  = (MCD_IPC_PORT_AP_INTROP_STRUCT*)&(msg->msgData);
        mvPortCtrlDbgIntropCfg(apIntropSetMsg);
        break;

    case MCD_IPC_PORT_AP_DEBUG_GET_MSG:
        mvPortCtrlLogInfoGet(&replyMsg.readData.logGet.fwBaseAddr,
                &replyMsg.readData.logGet.logBaseAddr,
                &replyMsg.readData.logGet.logPointerAddr,
                &replyMsg.readData.logGet.logCountAddr,
                &replyMsg.readData.logGet.logResetAddr);
        break;

    case MCD_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
        portRxCfgOverrideMsg = (MCD_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STRUCT*)&(msg->msgData);
        if (portRxCfgOverrideMsg->serdesSpeed == MCD_10_3125G)
        {
            i = MCD_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS;
        }
        else
        {
            i = MCD_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS;
        }
        portSm->apPortRxConfigOverride[i].bandLoopWidth = portRxCfgOverrideMsg->bandLoopWidth;
        portSm->apPortRxConfigOverride[i].dcGain = portRxCfgOverrideMsg->dcGain;
        portSm->apPortRxConfigOverride[i].lfHf = portRxCfgOverrideMsg->lfHf;
        portSm->apPortRxConfigOverride[i].squelch = portRxCfgOverrideMsg->squelch;
        portSm->apPortRxConfigOverride[i].fieldOverrideBmp = portRxCfgOverrideMsg->fieldOverrideBmp;
        portSm->apPortRxConfigOverride[i].etlMinDelay = portRxCfgOverrideMsg->etlMinDelay;
        portSm->apPortRxConfigOverride[i].etlMaxDelay = portRxCfgOverrideMsg->etlMaxDelay;
        break;

    case MCD_IPC_PORT_SERDES_TX_CONFIG_OVERRIDE_MSG:
        portTxCfgOverrideMsg = (MCD_IPC_PORT_SERDES_TX_CONFIG_OVERRIDE_STRUCT*)&(msg->msgData);
        if (portTxCfgOverrideMsg->serdesSpeed == MCD_10_3125G)
        {
            i = MCD_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS;
        }
        else
        {
            i = MCD_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS;
        }
        serdesTxApOverrideParams[portIndex][i].serdesTxParams = portTxCfgOverrideMsg->serdesTxParams;
        serdesTxApOverrideParams[portIndex][i].txOverrideBmp = portTxCfgOverrideMsg->txOverrideBmp;
        break;
    }
    mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);

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

    MCD_IPC_CTRL_MSG_STRUCT recvMsg;

    infoPtr->state = AP_PORT_DETECT_MSG_PROCESS_STATE;
    osMemSet(&recvMsg, 0, sizeof(recvMsg));
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




