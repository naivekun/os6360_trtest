/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsIpcApi.c
*
* DESCRIPTION:
*           This file contains APIs for HWS IPC
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>

/**************************** Globals ****************************************************/
#ifndef MV_HWS_REDUCED_BUILD

/* IPC queue IDs pool */
GT_UINTPTR hwsIpcChannelHandlers[HWS_MAX_DEVICE_NUM];
GT_U32 hwsIpcQueueIdPool[HWS_MAX_DEVICE_NUM][MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM] = {{0},{0}};

/* the structure to gather HWS IPC statistics on Host*/
MV_HWS_IPC_STATISTICS_STRUCT mvHwsIpcStatistics[HWS_MAX_DEVICE_NUM];

/**************************** Pre-Declaration ********************************************/
GT_STATUS mvHwsSetIpcInfo(GT_U8 devNum, GT_U32 msgType, MV_HWS_IPC_CTRL_MSG_STRUCT *msgDataPtr,
                          GT_U32 msgLength);
GT_STATUS mvHwsIpcCtrlMsgTx(MV_HWS_IPC_CTRL_MSG_STRUCT *txCtrlMsg);
GT_STATUS mvHwsIpcReplyMsgRx(GT_U8 devNum, MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgId, GT_U32 queueId,
                             MV_HWS_IPC_REPLY_MSG_STRUCT *rxReplyData);

#if defined(CPU_BE)
#include <cpssCommon/cpssPresteraDefs.h>
static void prvMvHwsIpcToLE(MV_HWS_IPC_CTRL_MSG_STRUCT *requestMsg);
static void prvMvHwsIpcFromLE(MV_HWS_IPC_REPLY_MSG_STRUCT *replyMsg);
#endif

/**
* @internal mvHwsIpcDbInit function
* @endinternal
*
* @brief   HW Services Ipc database initialization
*
* @param[in] devNum                   - device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_VOID mvHwsIpcDbInit
(
    GT_U8 devNum
)
{
    hwsOsMemSetFuncPtr(hwsIpcQueueIdPool[devNum],0,MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM*sizeof(GT_U32));
    hwsOsMemSetFuncPtr(&(mvHwsIpcStatistics[devNum]),0,sizeof(mvHwsIpcStatistics[devNum]));
}

/**
* @internal mvHwsGetQueueId function
* @endinternal
*
* @brief   finds free queue for Host 2 HWS connection
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - pointer to queue ID
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_U32 mvHwsGetQueueId
(
    GT_U8 devNum,
    GT_U32 * queueId
)
{
    GT_U32   i;

    for (i = MV_HWS_IPC_TX_0_CH_ID; i < MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM; i++)
    {
        if (hwsIpcQueueIdPool[devNum][i] == MV_HWS_IPC_FREE_QUEUE)
        {
            *queueId = i;
            hwsIpcQueueIdPool[devNum][i] = MV_HWS_IPC_QUEUE_BUSY;
            return GT_OK;
        }
    }

    if (i == MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM)
    {
        mvHwsIpcStatistics[devNum].mvHwsIpcGenFailureCount++;
        hwsOsPrintf("No free Host2Hws TX message\n");
        return GT_NO_RESOURCE;
    }

    return GT_OK;
}

/**
* @internal mvHwsReturnQueueId function
* @endinternal
*
* @brief   returns queue Id to the pool
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - queue Id
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsReturnQueueId
(
    GT_U8 devNum,
    GT_U32 queueId
)
{
    if (queueId >= MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM)
    {
        mvHwsIpcStatistics[devNum].mvHwsIpcGenFailureCount++;
        hwsOsPrintf("mvHwsReturnQueueId queue ID %d doesn't exist \n", queueId);
        return GT_BAD_PARAM;
    }

    hwsIpcQueueIdPool[devNum][queueId] = MV_HWS_IPC_FREE_QUEUE;

    return GT_OK;

}

/**
* @internal mvHwsIpcCtrlMsgTx function
* @endinternal
*
* @brief   Send IPC message from Host to HW Services
*
* @param[in] txCtrlMsg                - pointer to the message
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcCtrlMsgTx
(
    MV_HWS_IPC_CTRL_MSG_STRUCT *txCtrlMsg
)
{
    /* Check that the message is valid: msg type and queueId are in the defined range */
    if ((txCtrlMsg->ctrlMsgType >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE ) ||
         (txCtrlMsg->msgQueueId >= MV_HWS_MAX_HWS2HOST_REPLY_QUEUE_NUM))
    {
        mvHwsIpcStatistics[txCtrlMsg->devNum].mvHwsPortIpcFailureCount[txCtrlMsg->msgData.portGeneral.phyPortNum]++;
        hwsOsPrintf ("mvHwsIpcCtrlMsgTx wrong parameter msg type %d queue ID %d",txCtrlMsg->ctrlMsgType,txCtrlMsg->msgQueueId);
        return GT_BAD_PARAM;
    }

    /* Update counter */
    mvHwsIpcStatistics[txCtrlMsg->devNum].mvHwsHostTxMsgCount[txCtrlMsg->ctrlMsgType]++;

#if defined(CPU_BE)
    /* convert request to LE */
    prvMvHwsIpcToLE(txCtrlMsg);
#endif

    /* Send msg to Service CPU*/
    CHECK_STATUS(prvCpssGenericSrvCpuIpcMessageSend(
            hwsIpcChannelHandlers[txCtrlMsg->devNum],
            (txCtrlMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG)
                ? MV_HWS_IPC_HIGH_PRI_QUEUE
                : MV_HWS_IPC_LOW_PRI_QUEUE,
            (GT_U8*)txCtrlMsg,
            sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT)
    ));

    return GT_OK;
}

/**
* @internal mvHwsSetIpcInfo function
* @endinternal
*
* @brief   writes IPC data to message structure
*
* @param[in] devNum                   - system device number
* @param[in] msgDataPtr               - pointer to message data
* @param[in] msgLength                - message length
* @param[in] msgType                  - message type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSetIpcInfo
(
    GT_U8                        devNum,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgType,
    MV_HWS_IPC_CTRL_MSG_STRUCT   *msgDataPtr,
    GT_U32                       msgLength
)
{
    GT_U32 queueId;

    msgDataPtr->devNum      = devNum;
    msgDataPtr->msgLength   = (GT_U8)msgLength;
    msgDataPtr->ctrlMsgType = (GT_U8)msgType;
    CHECK_STATUS(mvHwsGetQueueId(devNum, &queueId));
    msgDataPtr->msgQueueId  = (GT_U8)queueId;

    return GT_OK;
}

/**
* @internal mvHwsIpcReplyMsgRx function
* @endinternal
*
* @brief   Gets reply from HW Services to the Host
*
* @param[in] devNum                   - system device number
* @param[in] queueId                  - queue ID
* @param[in] msgId                    - message ID
* @param[in] rxReplyData              - pointer to message
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_TIMEOUT               - timeout to receive reply message
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS mvHwsIpcReplyMsgRx
(
    GT_U8                        devNum,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgId,
    GT_U32                       queueId,
    MV_HWS_IPC_REPLY_MSG_STRUCT  *rxReplyData
)
{
    GT_STATUS rcode;
    GT_U32 replyTimeout = 200;

    /* Wait for message from Service CPU */
    do
    {
        rcode = prvCpssGenericSrvCpuIpcMessageRecv(hwsIpcChannelHandlers[devNum], queueId, (GT_U8*)rxReplyData, NULL);
        if (rcode != GT_NO_MORE)
        {
            break;
        }
        else
        {
            hwsOsExactDelayPtr(devNum, 0, 1);
            replyTimeout--;
        }
    } while (replyTimeout > 0);

#if defined(CPU_BE)
    /* convert reply from LE to BE */
    prvMvHwsIpcFromLE(rxReplyData);
#endif
    /* Free the queue */
    CHECK_STATUS(mvHwsReturnQueueId(devNum, queueId));

    if (rcode != GT_OK)
    {
        hwsOsPrintf("Failed to read from channel %d\n", queueId);
        return (rcode == GT_NO_MORE) ? GT_TIMEOUT : rcode;
    }

    if (rxReplyData->replyTo != (GT_U32)msgId)
    {
        mvHwsIpcStatistics[devNum].mvHwsIpcGenFailureCount++;
        hwsOsPrintf ("mvHwsIpceplyMsgRx wrong msg ID %d Expetced %d queue ID %d\n",rxReplyData->replyTo, msgId,queueId);
        return GT_BAD_PARAM;
    }

    /* Updte statistics*/
    mvHwsIpcStatistics[devNum].mvHwsHostRxMsgCount[msgId]++;

    return GT_OK;
}


/**
* @internal mvHwsIpcSendRequestAndGetReply function
* @endinternal
*
* @brief   Gets reply from HW Services to the Host
*
* @param[in] requestMsg               - pointer to request message
* @param[in] replyData                - pointer to reply message
* @param[in] msgId                    - message ID
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_TIMEOUT               - timeout to receive reply message
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS mvHwsIpcSendRequestAndGetReply
(
    GT_U8                                   devNum,
    MV_HWS_IPC_CTRL_MSG_STRUCT              *requestMsg,
    MV_HWS_IPC_REPLY_MSG_STRUCT             *replyData,
    GT_U32                                  msgLength,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE            msgId
)
{
#ifdef ASIC_SIMULATION
    (GT_VOID)devNum;
    (GT_VOID)*requestMsg;
    (GT_VOID)*replyData;
    (GT_VOID)msgLength;
    (GT_VOID)msgId;
#else
    /* Set IPC info */
    CHECK_STATUS(mvHwsSetIpcInfo (devNum,msgId,requestMsg, msgLength));

    /* Send IPC message */
    CHECK_STATUS(mvHwsIpcCtrlMsgTx (requestMsg));

    /* Wait for reply */
    CHECK_STATUS(mvHwsIpcReplyMsgRx (requestMsg->devNum,msgId,requestMsg->msgQueueId,replyData));
#endif
    return GT_OK;
}

/**
* @internal mvHwsIpcSendMsgNoReply function
* @endinternal
*
* @brief   Send a message to HW Services
*         No reply expected
* @param[in] requestMsg               - pointer to request message
* @param[in] msgId                    - message ID
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcSendMsgNoReply
(
    GT_U8                                   devNum,
    MV_HWS_IPC_CTRL_MSG_STRUCT              *requestMsg,
    GT_U32                                  msgLength,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE            msgId
)
{
#ifdef ASIC_SIMULATION
    (GT_VOID)devNum;
    (GT_VOID)*requestMsg;
    (GT_VOID)msgLength;
    (GT_VOID)msgId;
#else
    /* Set IPC info */
    requestMsg->devNum      = devNum;
    requestMsg->msgLength   = (GT_U8)msgLength;
    requestMsg->ctrlMsgType = (GT_U8)msgId;
    requestMsg->msgQueueId  = 0;

    /* Send IPC message */
    CHECK_STATUS(mvHwsIpcCtrlMsgTx (requestMsg));
#endif
    return GT_OK;
}

#if defined(CPU_BE)

#define PRV_TO_LE_FUNC(_type) \
    static void PRV_TO_LE_ ## _type(_type *r)
PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AP_DATA_STRUCT)
{
    r->portGroup  = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->advMode    = CPSS_32BIT_LE(r->advMode);
    r->options    = CPSS_16BIT_LE(r->options);
}
PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_INFO_STRUCT)
{
    r->portGroup  = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode   = CPSS_32BIT_LE(r->portMode);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AP_INTROP_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->attrBitMask = CPSS_16BIT_LE(r->attrBitMask);
    r->txDisDuration = CPSS_16BIT_LE(r->txDisDuration);
    r->abilityDuration = CPSS_16BIT_LE(r->abilityDuration);
    r->abilityMaxInterval = CPSS_16BIT_LE(r->abilityMaxInterval);
    r->abilityFailMaxInterval = CPSS_16BIT_LE(r->abilityFailMaxInterval);
    r->apLinkDuration = CPSS_16BIT_LE(r->apLinkDuration);
    r->apLinkMaxInterval = CPSS_16BIT_LE(r->apLinkMaxInterval);
    r->pdLinkDuration = CPSS_16BIT_LE(r->pdLinkDuration);
    r->pdLinkMaxInterval = CPSS_16BIT_LE(r->pdLinkMaxInterval);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT)
{
    r->state = CPSS_32BIT_LE(r->state);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_VOS_OVERRIDE_PARAMS)
{
    GT_U32 i;
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    for (i = 0; i < 8; i++)
        r->vosOverrideParamsBufferPtr[i] = CPSS_32BIT_LE(r->vosOverrideParamsBufferPtr[i]);
    r->vosOverrideParamsBufferIdx = CPSS_32BIT_LE(r->vosOverrideParamsBufferIdx);
    r->vosOverrideParamsBufferLength = CPSS_32BIT_LE(r->vosOverrideParamsBufferLength);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->vosOverrideMode = CPSS_32BIT_LE(r->vosOverrideMode);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_NOOP_DATA_STRUCT)
{
    r->data = CPSS_32BIT_LE(r->data);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_INIT_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->lbPort = CPSS_32BIT_LE(r->lbPort);
    r->refClock = CPSS_32BIT_LE(r->refClock);
    r->refClockSource = CPSS_32BIT_LE(r->refClockSource);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_RESET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->action = CPSS_32BIT_LE(r->action);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->portTuningMode = CPSS_32BIT_LE(r->portTuningMode);
    r->optAlgoMask = CPSS_32BIT_LE(r->optAlgoMask);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->stopRx = CPSS_32BIT_LE(r->stopRx);
    r->stopTx = CPSS_32BIT_LE(r->stopTx);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->txInvMask = (GT_32)(CPSS_32BIT_LE(r->txInvMask));
    r->rxInvMask = (GT_32)(CPSS_32BIT_LE(r->rxInvMask));
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->portFecType = CPSS_32BIT_LE(r->portFecType);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->enable = CPSS_32BIT_LE(r->enable);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->lpPlace = CPSS_32BIT_LE(r->lpPlace);
    r->lbType = CPSS_32BIT_LE(r->lbType);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->lpPlace = CPSS_32BIT_LE(r->lpPlace);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->portPPM = CPSS_32BIT_LE(r->portPPM);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->serdesNum = CPSS_32BIT_LE(r->serdesNum);
    r->serdesType = CPSS_32BIT_LE(r->serdesType);
    r->portTuningMode = CPSS_32BIT_LE(r->portTuningMode);
    r->sqlch = CPSS_32BIT_LE(r->sqlch);
    r->ffeRes = CPSS_32BIT_LE(r->ffeRes);
    r->ffeCap = CPSS_32BIT_LE(r->ffeCap);
    r->dfeEn = CPSS_32BIT_LE(r->dfeEn);
    r->alig = CPSS_32BIT_LE(r->alig);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->serdesNum = CPSS_32BIT_LE(r->serdesNum);
    r->serdesType = CPSS_32BIT_LE(r->serdesType);
    r->txAmp = CPSS_32BIT_LE(r->txAmp);
    r->txAmpAdj = CPSS_32BIT_LE(r->txAmpAdj);
    r->emph0 = CPSS_32BIT_LE(r->emph0);
    r->emph1 = CPSS_32BIT_LE(r->emph1);
    r->txAmpShft = CPSS_32BIT_LE(r->txAmpShft);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->fcState = CPSS_32BIT_LE(r->fcState);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT)
{
    r->portEnableCtrl = CPSS_32BIT_LE(r->portEnableCtrl);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->serdesSpeed = CPSS_32BIT_LE(r->serdesSpeed);
    r->serdesLane = r->serdesLane /*one byte length */;
    r->offsets = CPSS_16BIT_LE(r->offsets);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC)
{
    r->portGroup                      = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum                     = CPSS_16BIT_LE(r->phyPortNum);
    r->rxCfg.serdesSpeed              = CPSS_32BIT_LE(r->rxCfg.serdesSpeed);
    r->rxCfg.ctleParams.dcGain        = CPSS_32BIT_LE(r->rxCfg.ctleParams.dcGain);
    r->rxCfg.ctleParams.lowFrequency  = CPSS_32BIT_LE(r->rxCfg.ctleParams.lowFrequency);
    r->rxCfg.ctleParams.highFrequency = CPSS_32BIT_LE(r->rxCfg.ctleParams.highFrequency);
    r->rxCfg.ctleParams.bandWidth     = CPSS_32BIT_LE(r->rxCfg.ctleParams.bandWidth);
    r->rxCfg.ctleParams.loopBandwidth = CPSS_32BIT_LE(r->rxCfg.ctleParams.loopBandwidth);
    r->rxCfg.ctleParams.squelch       = CPSS_32BIT_LE(r->rxCfg.ctleParams.squelch);
    r->rxCfg.etlParams.etlMinDelay  = r->rxCfg.etlParams.etlMinDelay;
    r->rxCfg.etlParams.etlMaxDelay  = r->rxCfg.etlParams.etlMaxDelay;
    r->rxCfg.etlParams.etlEnableOverride = r->rxCfg.etlParams.etlEnableOverride;
    r->rxCfg.fieldOverrideBmp         = CPSS_16BIT_LE(r->rxCfg.fieldOverrideBmp);
}

/**
* @internal prvMvHwsIpcToLE function
* @endinternal
*
* @brief   Convert request message to Little Endian
*         Required when CPSS runs on CPU other than Little endian
* @param[in] requestMsg               - pointer to request message
*
* @param[out] requestMsg               - pointer to request message
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void prvMvHwsIpcToLE
(
    MV_HWS_IPC_CTRL_MSG_STRUCT              *requestMsg
)
{
#define PRV_TO_LE(_type,_member) \
    PRV_TO_LE_ ## _type(&(requestMsg->msgData._member)); break

    switch ((MV_HWS_IPC_CTRL_MSG_DEF_TYPE)(requestMsg->ctrlMsgType))
    {
        case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
        case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
        case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AP_DATA_STRUCT, apConfig);
        case MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG:
        case MV_HWS_IPC_PORT_AP_STATUS_MSG:
        case MV_HWS_IPC_PORT_AP_STATS_MSG:
        case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
        case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
        case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
        case MV_HWS_IPC_PORT_PARAMS_MSG:
        case MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG:
        case MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
        case MV_HWS_IPC_HWS_LOG_GET_MSG:
        case MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG:
        case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
        case MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG:
        case MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG:
        case MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG:
        case MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG:
        case MV_HWS_IPC_PORT_PPM_GET_MSG:
        case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_INFO_STRUCT, portLinkStatus);
        case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AP_INTROP_STRUCT, apIntropSet);
        case MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT, avagoGuiSet);
        case MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
            PRV_TO_LE(MV_HWS_IPC_VOS_OVERRIDE_PARAMS, vosOverrideParams);
        case MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT, vosOverrideModeSet);
#if 0
        case MV_HWS_IPC_EXECUTE_CMDLINE_MSG:
            /*MV_HWS_IPC_PORT_INIT_DATA_STRUCT*/
            /* don't convert */
            break;
#endif
        case MV_HWS_IPC_NOOP_MSG:
            PRV_TO_LE(MV_HWS_IPC_NOOP_DATA_STRUCT, noop);
        case MV_HWS_IPC_PORT_INIT_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_INIT_DATA_STRUCT, portInit);
        case MV_HWS_IPC_PORT_RESET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_RESET_DATA_STRUCT, portReset);
        case MV_HWS_IPC_PORT_RESET_EXT_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_INIT_DATA_STRUCT, portInit);
        case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT, portAutoTuneSetExt);
        case MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT, portAutoTuneStop);
        case MV_HWS_IPC_PORT_SERDES_RESET_MSG:
            /*TODO Unused ?*/
            break;
        case MV_HWS_IPC_PORT_POLARITY_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT, portPolaritySet);
        case MV_HWS_IPC_PORT_FEC_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT, portFecConfig);
        case MV_HWS_IPC_PORT_TX_ENABLE_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT, portTxEnableData);
        case MV_HWS_IPC_PORT_LOOPBACK_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT, portLoopbackSet);
        case MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT, portLoopbackGet);
        case MV_HWS_IPC_PORT_PPM_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT, portPPMSet);
        case MV_HWS_IPC_PORT_IF_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT, portInterfaceGet);
        case MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT, serdesManualRxConfig);
        case MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT, serdesManualTxConfig);
        case MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT, portPcsActiveStatus);
        case MV_HWS_IPC_PORT_FC_STATE_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT, portFcStateSet);
        case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT, portEnableCtrlSet);
        case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC, portSerdesTxParametersOffsetConfig);
        case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC, portSerdesRxConfigOverride);

        case MV_HWS_IPC_LAST_CTRL_MSG_TYPE:
            break;
    }
}


#define PRV_FROM_LE_FUNC(_type) \
    static void PRV_FROM_LE_ ## _type(_type *r)

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_CFG_GET)
{
    r->ifNum = CPSS_32BIT_LE(r->ifNum);
    r->capability = CPSS_32BIT_LE(r->capability);
    r->options = CPSS_16BIT_LE(r->options);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET)
{
    r->state = CPSS_16BIT_LE(r->state);
    r->status = CPSS_16BIT_LE(r->status);
    r->laneNum = CPSS_16BIT_LE(r->laneNum);
    r->ARMSmStatus = CPSS_16BIT_LE(r->ARMSmStatus);
    r->hcdStatus = CPSS_32BIT_LE(r->hcdStatus);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_STATS_GET)
{
    r->txDisCnt = CPSS_16BIT_LE(r->txDisCnt);
    r->abilityCnt = CPSS_16BIT_LE(r->abilityCnt);
    r->abilitySuccessCnt = CPSS_16BIT_LE(r->abilitySuccessCnt);
    r->linkFailCnt = CPSS_16BIT_LE(r->linkFailCnt);
    r->linkSuccessCnt = CPSS_16BIT_LE(r->linkSuccessCnt);
    r->hcdResoultionTime = CPSS_32BIT_LE(r->hcdResoultionTime);
    r->linkUpTime = CPSS_32BIT_LE(r->linkUpTime);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET)
{
    r->txDisDuration = CPSS_16BIT_LE(r->txDisDuration);
    r->abilityDuration = CPSS_16BIT_LE(r->abilityDuration);
    r->abilityMaxInterval = CPSS_16BIT_LE(r->abilityMaxInterval);
    r->abilityFailMaxInterval = CPSS_16BIT_LE(r->abilityFailMaxInterval);
    r->apLinkDuration = CPSS_16BIT_LE(r->apLinkDuration);
    r->apLinkMaxInterval = CPSS_16BIT_LE(r->apLinkMaxInterval);
    r->pdLinkDuration = CPSS_16BIT_LE(r->pdLinkDuration);
    r->pdLinkMaxInterval = CPSS_16BIT_LE(r->pdLinkMaxInterval);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_LOG_GET)
{
    r->fwBaseAddr = CPSS_32BIT_LE(r->fwBaseAddr);
    r->logBaseAddr = CPSS_32BIT_LE(r->logBaseAddr);
    r->logCountAddr = CPSS_32BIT_LE(r->logCountAddr);
    r->logPointerAddr = CPSS_32BIT_LE(r->logPointerAddr);
    r->logResetAddr = CPSS_32BIT_LE(r->logResetAddr);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET)
{
    r->addr = CPSS_32BIT_LE(r->addr);
    r->size = CPSS_32BIT_LE(r->size);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_NOOP_REPLY)
{
    r->data = CPSS_32BIT_LE(r->data);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AUTO_TUNE_STATE_CHK)
{
    r->rxTune = CPSS_32BIT_LE(r->rxTune);
    r->txTune = CPSS_32BIT_LE(r->txTune);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_STATUS_GET)
{
    r->status = CPSS_32BIT_LE(r->status);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET)
{
    GT_U32 i;
    for (i = 0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
        r->status[i] = CPSS_32BIT_LE(r->status[i]);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_LOOPBACK_STATUS_GET)
{
    r->lbType = CPSS_32BIT_LE(r->lbType);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_PPM_GET)
{
    r->portPpm = CPSS_32BIT_LE(r->portPpm);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_IF_GET)
{
    r->portIf = CPSS_32BIT_LE(r->portIf);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_RESULT)
{
    r->results = CPSS_32BIT_LE(r->results);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT)
{
    r->portEnableCtrl = CPSS_32BIT_LE(r->portEnableCtrl);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_TX_OFFSETS_INFO_STRUCT)
{
    r->txOffsetBaseAddr = CPSS_32BIT_LE(r->txOffsetBaseAddr);
    r->txOffsetSize = CPSS_32BIT_LE(r->txOffsetSize);
}

/**
* @internal prvMvHwsIpcFromLE function
* @endinternal
*
* @brief   Convert request message to Little Endian
*         Required when CPSS runs on CPU other than Little endian
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void prvMvHwsIpcFromLE
(
    MV_HWS_IPC_REPLY_MSG_STRUCT              *replyMsg
)
{
    replyMsg->returnCode = CPSS_32BIT_LE(replyMsg->returnCode);
    replyMsg->replyTo = CPSS_32BIT_LE(replyMsg->replyTo);
#define PRV_FROM_LE(_type,_member) \
    PRV_FROM_LE_ ## _type(&(replyMsg->readData._member)); break
    switch ((MV_HWS_IPC_CTRL_MSG_DEF_TYPE)(replyMsg->replyTo))
    {
        case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
        case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
        case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
        case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
        case MV_HWS_IPC_PORT_PARAMS_MSG:
        case MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG:
        case MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
        case MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
        case MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG:
#if 0
        case MV_HWS_IPC_EXECUTE_CMDLINE_MSG:
#endif
        case MV_HWS_IPC_PORT_INIT_MSG:
        case MV_HWS_IPC_PORT_RESET_MSG:
        case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
        case MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG:
        case MV_HWS_IPC_PORT_POLARITY_SET_MSG:
        case MV_HWS_IPC_PORT_FEC_CONFIG_MSG:
        case MV_HWS_IPC_PORT_TX_ENABLE_MSG:
        case MV_HWS_IPC_PORT_LOOPBACK_SET_MSG:
        case MV_HWS_IPC_PORT_PPM_SET_MSG:
        case MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG:
        case MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG:
        case MV_HWS_IPC_PORT_FC_STATE_SET_MSG:
        case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
        case MV_HWS_IPC_PORT_RESET_EXT_MSG:
        case MV_HWS_IPC_PORT_SERDES_RESET_MSG:
        case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
            /* just returnCode */
            break;
        case MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG:
            /* No reply for this message */
            break;
        case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_CFG_GET, portApCfgGet);
        case MV_HWS_IPC_PORT_AP_STATUS_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET, portApStatusGet);
        case MV_HWS_IPC_PORT_AP_STATS_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_STATS_GET, portApStatsGet);
        case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET, portApIntropGet);
        case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
        case MV_HWS_IPC_HWS_LOG_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_LOG_GET, logGet);
        case MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET, portAvagoAaplGet);
        case MV_HWS_IPC_NOOP_MSG:
            PRV_FROM_LE(MV_HWS_IPC_NOOP_REPLY, noopReply);
        case MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AUTO_TUNE_STATE_CHK, portAutoTuneStateChk);
        case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
        case MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_STATUS_GET, portStatusGet);
        case MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG:
        case MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG:
        case MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET, portSerdesTxEnableGet);
        case MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_LOOPBACK_STATUS_GET, portLoopbackStatusGet);
        case MV_HWS_IPC_PORT_PPM_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_PPM_GET, portPpmGet);
        case MV_HWS_IPC_PORT_IF_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_IF_GET, portIfGet);
        case MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_RESULT, portReplyGet);
        case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT, portEnableCtrlGet);
        case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_TX_OFFSETS_INFO_STRUCT, portSerdesTxParametersOffsetInfoCfg);
        case MV_HWS_IPC_LAST_CTRL_MSG_TYPE:
            break;
    }
}
#endif /* defined(CPU_BE) */

#endif /* MV_HWS_REDUCED_BUILD */


