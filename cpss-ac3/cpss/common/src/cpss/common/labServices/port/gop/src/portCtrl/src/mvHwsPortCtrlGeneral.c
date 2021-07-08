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
* @file mvHwsPortCtrlGeneral.c
*
* @brief Port Control General State Machine
*
* @version   1
********************************************************************************
*/
#ifdef AP_GENERAL_TASK
#include <mvHwsPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>

static MV_HWS_PORT_CTRL_COMMON *infoPtr;

extern GT_STATUS mvHwsAvagoSerdesTxParametersOffsetSet
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U8  serdesLane,
    GT_U16 offsets,
    GT_U32 *baseAddrPtr,
    GT_U32 *sizePtr
) ;

/**
* @internal mvPortCtrlGenRoutine function
* @endinternal
*
* @brief   General process execution sequence
*/
void mvPortCtrlGenRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvPortCtrlGen.info);

    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+
        ** + Event \ State  +   Msg O1    +   Delay O2  +
        ** +================+=============+=============+
        ** + Msg            +     O1      +    ==> O1   +
        ** +================+=============+=============+
        ** + Delay          +   ==> O2    +   Invalid   +
        ** +================+=============+=============+
        */
        if (mvPortCtrlGen.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvPortCtrlGen.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Ctrl, General Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
    }
}

/* AP to CPSS Doorbell events test */
#if 1
#include <mvHwsPortCtrlDoorbellEvents.h>
#endif

/**
* @internal mvPortCtrlGenMsgProcess function
* @endinternal
*
* @brief   General message process state execution
*         - Read message from message queue
*         Message received
*         Add Log entry
*         Execute received message processing
*         - Message not received
*         Trigger state change
*/
void mvPortCtrlGenMsgProcess(void)
{
    GT_STATUS rcode;
    GT_U32 targetPort;
    MV_HWS_IPC_REPLY_MSG_STRUCT                         reply;
    MV_HWS_IPC_CTRL_MSG_STRUCT                          recvMsg;
#ifndef BV_DEV_SUPPORT
    MV_HWS_IPC_PORT_INFO_STRUCT                         *infoMsg;
    MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT       *tuneMsg;
    MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT          *tuneStopMsg;
#if !(defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
    MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT *manRxCfgMsg;
#endif
    MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT *manTxCfgMsg;
    MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT            *polMsg;
    MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT              *fecMsg;
    MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT               *txMsg;
    MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT            *lbSetMsg;
    MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT            *lbGetMsg;
#if !(defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
    MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT                 *ppmMsg;
#endif
    MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT                  *ifMsg;
    MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT   *pcsActiveMsg;
    MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT        *portFcStateSetMsg;
    MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT                  *paramsMsg;

    MV_HWS_PORT_INIT_PARAMS  portParams;
    GT_U32 i;
#endif
    infoPtr->state = PORT_GEN_MSG_PROCESS_STATE;

    /* Process messages from General message queue */
    rcode = mvPortCtrlProcessMsgRecv(M6_GENERAL, MV_PROCESS_MSG_RX_NO_DELAY, &recvMsg);
    if (rcode != GT_OK)
    {
        infoPtr->event = PORT_GEN_DELAY_EVENT;
    }
    else
    {
        /* Check that the message is valid: msg type and queueId are in the defined range */
        if ((recvMsg.ctrlMsgType >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE) ||
            ((recvMsg.msgQueueId  >= MV_HWS_MAX_HWS2HOST_REPLY_QUEUE_NUM) &&
            (recvMsg.msgQueueId  != MV_PORT_CTRL_NO_MSG_REPLY)))
        {
            reply.returnCode = GT_BAD_PARAM;
            /* Build and Send IPC reply */
            mvHwsIpcReplyMsg(recvMsg.msgQueueId, &reply);
            return;
        }

        reply.replyTo = recvMsg.ctrlMsgType;

        /* Decodes received IPC message */
        switch (recvMsg.ctrlMsgType)
        {
#ifndef BV_DEV_SUPPORT
        case MV_HWS_IPC_PORT_PARAMS_MSG:
        {
            targetPort = recvMsg.msgData.portParams.phyPortNum;
            paramsMsg  = &recvMsg.msgData.portParams;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));

            /* since some of data type don't match between structures (such as active Serdes)
            ** the copy should be param-by-param (instead of memcpy) */
            portParams.portStandard = paramsMsg->portStandard;
            portParams.portMacType = paramsMsg->portMacType;
            portParams.portMacNumber = paramsMsg->portMacNumber;
            portParams.portPcsType = paramsMsg->portPcsType;
            portParams.portPcsNumber = paramsMsg->portMacNumber;
            portParams.portFecMode = paramsMsg->portFecMode;
            portParams.serdesSpeed = paramsMsg->serdesSpeed;
            portParams.firstLaneNum = paramsMsg->firstLaneNum;
            portParams.numOfActLanes = (GT_U32)paramsMsg->numOfActLanes;

            /* copy Serdes list */
            for(i = 0; i< paramsMsg->numOfActLanes; i++)
            {
                portParams.activeLanesList[i] = (GT_U32)paramsMsg->activeLanesList[i];
            }
            portParams.serdesMediaType = paramsMsg->serdesMediaType;
            portParams.serdes10BitStatus = paramsMsg->serdes10BitStatus;

            reply.returnCode = hwsPortModeParamsSet(recvMsg.devNum,
                                                    paramsMsg->portGroup,
                                                    paramsMsg->phyPortNum,
                                                    &portParams);
            break;
        }
        /* Code not relevant for BobK CM3 due to space limit */
        case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
        {
            targetPort =  recvMsg.msgData.portAutoTuneSetExt.phyPortNum;
            tuneMsg    = &recvMsg.msgData.portAutoTuneSetExt;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));

            reply.returnCode = mvHwsPortAutoTuneSetExt(recvMsg.devNum,
                                                       tuneMsg->portGroup,
                                                       tuneMsg->phyPortNum,
                                                       tuneMsg->portMode,
                                                       tuneMsg->portTuningMode,
                                                       tuneMsg->optAlgoMask,
                                                       &reply.readData.portReplyGet.results);
            if (reply.returnCode != GT_OK)
            {
                mvPortCtrlLogAdd(PORT_GENERAL_LOG_RES(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType,
                                                      targetPort, LOG_RESULT_MASK, MV_GEN_TUNE_FAIL));
                mvPcPrintf("Error, Port Mng, Port-%d Rx Training Failed\n", targetPort);
            }
            else
            {
                mvPortCtrlLogAdd(PORT_GENERAL_LOG_RES(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType,
                                                      targetPort, LOG_RESULT_MASK, MV_GEN_TUNE_PASS));
            }
            break;
        }
        case MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG:
        {
            targetPort  =  recvMsg.msgData.portAutoTuneStop.phyPortNum;
            tuneStopMsg = &recvMsg.msgData.portAutoTuneStop;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortAutoTuneStop(recvMsg.devNum,
                                                     tuneStopMsg->portGroup,
                                                     tuneStopMsg->phyPortNum,
                                                     tuneStopMsg->portMode,
                                                     tuneStopMsg->stopRx,
                                                     tuneStopMsg->stopTx);
            break;
        }
        case MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG:
        {
            MV_HWS_AUTO_TUNE_STATUS rxTuneTmp;
            MV_HWS_AUTO_TUNE_STATUS txTuneTmp;

            targetPort =  recvMsg.msgData.portAutoTuneStateChk.phyPortNum;
            infoMsg    = &recvMsg.msgData.portAutoTuneStateChk;
            /* to avoid possible problems with types translation */
            rxTuneTmp  = (MV_HWS_AUTO_TUNE_STATUS)reply.readData.portAutoTuneStateChk.rxTune;
            txTuneTmp  = (MV_HWS_AUTO_TUNE_STATUS)reply.readData.portAutoTuneStateChk.txTune;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortAutoTuneStateCheck(recvMsg.devNum,
                                                           infoMsg->portGroup,
                                                           infoMsg->phyPortNum,
                                                           infoMsg->portMode,
                                                           &rxTuneTmp,
                                                           &txTuneTmp);
            break;
        }
        case MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG:
        {
#if !(defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
            manRxCfgMsg = &recvMsg.msgData.serdesManualRxConfig;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, 0x7F));
            reply.returnCode = mvHwsSerdesManualRxConfig(recvMsg.devNum,
                                                         manRxCfgMsg->portGroup,
                                                         manRxCfgMsg->serdesNum,
                                                         manRxCfgMsg->serdesType,
                                                         manRxCfgMsg->portTuningMode,
                                                         manRxCfgMsg->sqlch,
                                                         manRxCfgMsg->ffeRes,
                                                         manRxCfgMsg->ffeCap,
                                                         manRxCfgMsg->dfeEn,
                                                         manRxCfgMsg->alig);
#else
            reply.returnCode = GT_NOT_SUPPORTED;
#endif
            break;
        }
        case MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG:
        {
            manTxCfgMsg = &recvMsg.msgData.serdesManualTxConfig;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, 0x7F));
            reply.returnCode = mvHwsSerdesManualTxConfig(recvMsg.devNum,
                                                         manTxCfgMsg->portGroup,
                                                         manTxCfgMsg->serdesNum,
                                                         manTxCfgMsg->serdesType,
                                                         manTxCfgMsg->txAmp,
                                                         manTxCfgMsg->txAmpAdj,
                                                         manTxCfgMsg->emph0,
                                                         manTxCfgMsg->emph1,
                                                         manTxCfgMsg->txAmpShft, GT_TRUE);
            break;
        }
        case MV_HWS_IPC_PORT_POLARITY_SET_MSG:
        {
            targetPort =  recvMsg.msgData.portPolaritySet.phyPortNum;
            polMsg     = &recvMsg.msgData.portPolaritySet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortPolaritySet(recvMsg.devNum,
                                                    polMsg->portGroup,
                                                    polMsg->phyPortNum,
                                                    polMsg->portMode,
                                                    polMsg->txInvMask,
                                                    polMsg->rxInvMask);
            break;
        }
        case MV_HWS_IPC_PORT_FEC_CONFIG_MSG:
        {
            targetPort =  recvMsg.msgData.portFecConfig.phyPortNum;
            fecMsg     = &recvMsg.msgData.portFecConfig;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortFecCofig(recvMsg.devNum,
                                                 fecMsg->portGroup,
                                                 fecMsg->phyPortNum,
                                                 fecMsg->portMode,
                                                 fecMsg->portFecType);
            break;
        }
        case MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG:
        {
            targetPort =  recvMsg.msgData.portFecConfigGet.phyPortNum;
            infoMsg    = &recvMsg.msgData.portFecConfigGet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortFecCofigGet(recvMsg.devNum,
                                                    infoMsg->portGroup,
                                                    infoMsg->phyPortNum,
                                                    infoMsg->portMode,
                                                    (MV_HWS_PORT_FEC_MODE *)&reply.readData.portStatusGet.status);
            break;
        }
        case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
        {
            targetPort =  recvMsg.msgData.portLinkStatus.phyPortNum;
            infoMsg    = &recvMsg.msgData.portLinkStatus;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortLinkStatusGet(recvMsg.devNum,
                                                      infoMsg->portGroup,
                                                      infoMsg->phyPortNum,
                                                      infoMsg->portMode,
                                                      (GT_BOOL *)&reply.readData.portStatusGet.status);
            if (reply.returnCode == GT_OK)
            {
                if (reply.readData.portStatusGet.status == GT_FALSE)
                {
                    mvPortCtrlLogAdd(PORT_GENERAL_LOG_RES(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType,
                                                          targetPort, LOG_RESULT_MASK, MV_GEN_LINK_DOWN));
                    mvPcPrintf("Port-%d Link Down\n", targetPort);
                }
                else
                {
                    mvPortCtrlLogAdd(PORT_GENERAL_LOG_RES(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType,
                                                          targetPort, LOG_RESULT_MASK, MV_GEN_LINK_UP));
                    mvPcPrintf("Port-%d Link Up\n", targetPort);
                }
            }
            else
            {
                mvPcPrintf("Error, Port Mng, Port-%d Link Get Failed\n", targetPort);
            }
            break;
        }
        case MV_HWS_IPC_PORT_TX_ENABLE_MSG:
        {
            targetPort =  recvMsg.msgData.portTxEnableData.phyPortNum;
            txMsg      = &recvMsg.msgData.portTxEnableData;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortTxEnable(recvMsg.devNum,
                                                 txMsg->portGroup,
                                                 txMsg->phyPortNum,
                                                 txMsg->portMode,
                                                 txMsg->enable);
            break;
        }
        case  MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG:
        {
            targetPort =  recvMsg.msgData.portTxEnableGet.phyPortNum;
            infoMsg    = &recvMsg.msgData.portTxEnableGet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortTxEnableGet(recvMsg.devNum,
                                                    infoMsg->portGroup,
                                                    infoMsg->phyPortNum,
                                                    infoMsg->portMode,
                                                    (GT_BOOL *)&reply.readData.portSerdesTxEnableGet.status[0]);
            break;
        }
        case MV_HWS_IPC_PORT_LOOPBACK_SET_MSG:
        {
            targetPort =  recvMsg.msgData.portLoopbackSet.phyPortNum;
            lbSetMsg   = &recvMsg.msgData.portLoopbackSet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortLoopbackSet(recvMsg.devNum,
                                                    lbSetMsg->portGroup,
                                                    lbSetMsg->phyPortNum,
                                                    lbSetMsg->portMode,
                                                    lbSetMsg->lpPlace,
                                                    lbSetMsg->lbType);
            break;
        }
        case  MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG:
        {
            targetPort =  recvMsg.msgData.portLoopbackGet.phyPortNum;
            lbGetMsg   = &recvMsg.msgData.portLoopbackGet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortLoopbackStatusGet(recvMsg.devNum,
                                                          lbGetMsg->portGroup,
                                                          lbGetMsg->phyPortNum,
                                                          lbGetMsg->portMode,
                                                          lbGetMsg->lpPlace,
                                                          (MV_HWS_PORT_LB_TYPE *)&reply.readData.portLoopbackStatusGet.lbType);
            break;
        }
        case MV_HWS_IPC_PORT_PPM_SET_MSG:
        {
#if !(defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
            targetPort =  recvMsg.msgData.portPPMSet.phyPortNum;
            ppmMsg     = &recvMsg.msgData.portPPMSet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortPPMSet(recvMsg.devNum,
                                               ppmMsg->portGroup,
                                               ppmMsg->phyPortNum,
                                               ppmMsg->portMode,
                                               ppmMsg->portPPM);
#else
            reply.returnCode = GT_NOT_SUPPORTED;
#endif
            break;
        }
        case  MV_HWS_IPC_PORT_PPM_GET_MSG:
        {
#if !(defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
            targetPort =  recvMsg.msgData.portPPMGet.phyPortNum;
            infoMsg    = &recvMsg.msgData.portPPMGet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortPPMGet(recvMsg.devNum,
                                               infoMsg->portGroup,
                                               infoMsg->phyPortNum,
                                               infoMsg->portMode,
                                               (MV_HWS_PPM_VALUE *)&reply.readData.portPpmGet.portPpm);
#else
            reply.returnCode = GT_NOT_SUPPORTED;
#endif
            break;
        }
        case  MV_HWS_IPC_PORT_IF_GET_MSG:
        {
            targetPort =  recvMsg.msgData.portInterfaceGet.phyPortNum;
            ifMsg      = &recvMsg.msgData.portInterfaceGet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortInterfaceGet(recvMsg.devNum,
                                                     ifMsg->portGroup,
                                                     ifMsg->phyPortNum,
                                                     (MV_HWS_PORT_STANDARD *)&reply.readData.portIfGet.portIf);
            break;
        }
        case MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG:
        {
            targetPort =  recvMsg.msgData.portSignalDetectGet.phyPortNum;
            infoMsg    = &recvMsg.msgData.portSignalDetectGet;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortSignalDetectGet(recvMsg.devNum,
                                                        infoMsg->portGroup,
                                                        infoMsg->phyPortNum,
                                                        infoMsg->portMode,
                                                        (GT_BOOL *)&reply.readData.portSerdesSignalDetectGet.status[0]);
            break;
        }
        case MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG:
        {
#if !(defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
            targetPort =  recvMsg.msgData.portCdrLockStatus.phyPortNum;
            infoMsg    = &recvMsg.msgData.portCdrLockStatus;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortCdrLockStatusGet(recvMsg.devNum,
                                                         infoMsg->portGroup,
                                                         infoMsg->phyPortNum,
                                                         infoMsg->portMode,
                                                         (GT_BOOL *)&reply.readData.portSerdesCdrLockStatusGet.status[0]);
#else
            reply.returnCode = GT_NOT_SUPPORTED;
#endif
            break;
        }
        case MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG:
        {
            targetPort   =  recvMsg.msgData.portPcsActiveStatus.phyPortNum;
            pcsActiveMsg = &recvMsg.msgData.portPcsActiveStatus;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortPcsActiveStatusGet(recvMsg.devNum,
                                                           pcsActiveMsg->portGroup,
                                                           pcsActiveMsg->phyPortNum,
                                                           pcsActiveMsg->portMode,
                                                           &reply.readData.portReplyGet.results);
            break;
        }
        case MV_HWS_IPC_PORT_FC_STATE_SET_MSG:
        {
            portFcStateSetMsg   = &recvMsg.msgData.portFcStateSet;
            targetPort =  portFcStateSetMsg->phyPortNum;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(PORT_GEN_MSG_PROCESS_STATE, recvMsg.ctrlMsgType, targetPort));
            reply.returnCode = mvHwsPortFlowControlStateSet(recvMsg.devNum,
                                                            portFcStateSetMsg->portGroup,
                                                            portFcStateSetMsg->phyPortNum,
                                                            portFcStateSetMsg->portMode,
                                                            portFcStateSetMsg->fcState);
            break;
        }
#endif
        case MV_HWS_IPC_NOOP_MSG:
        {
/* AP to CPSS Doorbell events test */
#if 1
            if (recvMsg.msgData.noop.data == 8)
                recvMsg.msgData.noop.data = apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_IPC_E);
            if (recvMsg.msgData.noop.data == 10)
                recvMsg.msgData.noop.data = apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(30));
#endif
            reply.readData.noopReply.data = recvMsg.msgData.noop.data+2;
            reply.returnCode = GT_OK;
            break;
        }
        case MV_HWS_IPC_HWS_LOG_GET_MSG:
            mvHwsLogInfoGet(&reply.readData.logGet.fwBaseAddr,
                &reply.readData.logGet.logBaseAddr,
                &reply.readData.logGet.logPointerAddr,
                &reply.readData.logGet.logCountAddr,
                &reply.readData.logGet.logResetAddr);
            reply.returnCode = GT_OK;
            break;

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
            case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
            reply.returnCode = mvHwsAvagoSerdesTxParametersOffsetSet(recvMsg.devNum,
                  recvMsg.msgData.portSerdesTxParametersOffsetConfig.portGroup,
                  recvMsg.msgData.portSerdesTxParametersOffsetConfig.phyPortNum,
                  recvMsg.msgData.portSerdesTxParametersOffsetConfig.serdesSpeed,
                  recvMsg.msgData.portSerdesTxParametersOffsetConfig.serdesLane,
                  recvMsg.msgData.portSerdesTxParametersOffsetConfig.offsets,
                 &reply.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetBaseAddr,
                 &reply.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetSize);
            break;
#endif /* (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)) */
        default:
            reply.returnCode = GT_NOT_SUPPORTED;
        }

        /* Build and Send IPC reply */
        mvHwsIpcReplyMsg(recvMsg.msgQueueId, &reply);
    }
}

/**
* @internal mvPortCtrlGenDelay function
* @endinternal
*
* @brief   General message delay state execution
*         - Execute delay
*         - Trigger state change
*/
void mvPortCtrlGenDelay(void)
{
    infoPtr->state = PORT_GEN_DELAY_STATE;
    mvPortCtrlProcessDelay(infoPtr->delayDuration);
    infoPtr->event = PORT_GEN_MSG_EVENT;
}
#endif

