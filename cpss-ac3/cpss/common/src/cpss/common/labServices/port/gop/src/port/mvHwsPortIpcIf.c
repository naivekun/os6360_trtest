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
* mvHwsHost2HwsIfWraper.c
*
* DESCRIPTION: Port external interface
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 55 $
******************************************************************************/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortApInitIfPrv.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#ifdef _VISUALC
#pragma warning( disable : 4204)
#endif
/**************************** Globals ****************************************************/



/**************************** Pre-Declaration ********************************************/
extern GT_U32 txParamesOffsetsFwBaseAddr;
extern GT_U32 txParamesOffsetsFwSize;

/**
* @internal mvHwsIpcSendPortParams function
* @endinternal
*
* @brief   Sends to HWS request to set port parameters.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcSendPortParams
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
    GT_U32                   phyPortNum,
    MV_HWS_PORT_STANDARD     portMode
)
{
    GT_U8 i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT portParams;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* get port params */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        hwsOsPrintf("mvHwsIpcSendPortParams: portMode %d for port %d is not supported\n", portMode, phyPortNum);
        return GT_NOT_SUPPORTED;
    }

    /* fill IPC message data */
    portParams.portGroup = (GT_U16)portGroup;
    portParams.phyPortNum = (GT_U16)phyPortNum;
    portParams.portStandard = portMode;
    portParams.portMacType = curPortParams.portMacType;
    portParams.portMacNumber = curPortParams.portMacNumber;
    portParams.portPcsType = curPortParams.portPcsType;
    portParams.portPcsNumber = curPortParams.portPcsNumber;
    portParams.portFecMode = curPortParams.portFecMode;
    portParams.serdesSpeed = curPortParams.serdesSpeed;
    portParams.firstLaneNum = (GT_U8)curPortParams.firstLaneNum;
    portParams.numOfActLanes = (GT_U8)curPortParams.numOfActLanes;
    /* set active lane list*/
    for(i = 0; i< curPortParams.numOfActLanes; i++)
    {
        portParams.activeLanesList[i] = (GT_U8)curPortParams.activeLanesList[i];
    }
    portParams.serdesMediaType = curPortParams.serdesMediaType;
    portParams.serdes10BitStatus = curPortParams.serdes10BitStatus;

    /* Construct the msg */
    requestMsg.msgData.portParams = portParams;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_PARAMS_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortInitIpc function
* @endinternal
*
* @brief   Sends to HWS request to init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInitIpc
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
    GT_U32                   phyPortNum,
    MV_HWS_PORT_STANDARD     portMode,
    GT_BOOL                  lbPort,
    MV_HWS_REF_CLOCK_SUP_VAL refClock,
    MV_HWS_REF_CLOCK_SOURCE  refClockSource)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INIT_DATA_STRUCT portInit =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, lbPort, refClock, refClockSource};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portInit = portInit;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INIT_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_INIT_MSG));

    return (replyData.returnCode);
}
/**
* @internal mvHwsPortResetIpc function
* @endinternal
*
* @brief   Sends to HWS request to power down or reset physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      lbPort     - if true, init port without serdes activity
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortResetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_ACTION   action
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_RESET_DATA_STRUCT portReset = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, action};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portReset = portReset;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_RESET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_RESET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortAutoTuneStateCheckIpc function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portTuningMode - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStateCheckIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *rxTune,
    MV_HWS_AUTO_TUNE_STATUS *txTune
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portAutoTuneStateChk = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    requestMsg.msgData.portAutoTuneStateChk = portAutoTuneStateChk;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG));

    /* Update reply parameters */
    *rxTune = replyData.readData.portAutoTuneStateChk.rxTune;
    *txTune = replyData.readData.portAutoTuneStateChk.txTune;

    return (replyData.returnCode);
}
/**
* @internal mvHwsPortLinkStatusGetIpc function
* @endinternal
*
* @brief   Returns port link status.
*         Can run at any time.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLinkStatusGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              *linkStatus
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portLinkStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portLinkStatus = portLinkStatus;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG));

    /* Update reply parameters */
    *linkStatus = replyData.readData.portStatusGet.status;

   return (replyData.returnCode);
}
/**
* @internal mvHwsPortAutoTuneSetExtIpc function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSetExtIpc
(
    GT_U8                      devNum,
    GT_U32                     portGroup,
    GT_U32                     phyPortNum,
    MV_HWS_PORT_STANDARD       portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE portTuningMode,
    GT_U32                     optAlgoMask,
    void                       *results
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT portAutoTuneSetExt =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, portTuningMode, optAlgoMask};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    results = results; /* to avoid warning */

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portAutoTuneSetExt = portAutoTuneSetExt;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortAutoTuneStopIpc function
* @endinternal
*
* @brief   Send IPC message to stop Tx and Rx training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] stopRx                   - stop RX
* @param[in] stopTx                   - stop Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStopIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              stopRx,
    GT_BOOL              stopTx
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT portAutoTuneStop =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, stopRx, stopTx};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portAutoTuneStop = portAutoTuneStop;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsSerdesManualRxConfigIpc function
* @endinternal
*
* @brief   Send IPC message to configure SERDES Rx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*                                      portMode   - port standard metric
*                                      config params: sqlch,ffeRes,ffeCap,dfeEn,alig
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualRxConfigIpc
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    GT_U32                    sqlch,
    GT_U32                    ffeRes,
    GT_U32                    ffeCap,
    GT_BOOL                   dfeEn,
    GT_U32                    alig
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;

    MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT serdesManualRxConfig =
        {(GT_U16)portGroup, serdesNum, serdesType, portTuningMode, sqlch, ffeRes, ffeCap, dfeEn, alig};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.serdesManualRxConfig = serdesManualRxConfig;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsSerdesManualTxConfigIpc function
* @endinternal
*
* @brief   Send IPC message to configure SERDES Tx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*                                      portMode   - port standard metric
*                                      config params: txAmp,txAmpAdj,emph0,emph1,txAmpShft
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfigIpc
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_U32              txAmp,
    GT_BOOL             txAmpAdj,
    GT_U32              emph0,
    GT_U32              emph1,
    GT_BOOL             txAmpShft
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;

    MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT serdesManualTxConfig =
        {(GT_U16)portGroup, serdesNum, serdesType, txAmp, txAmpAdj, emph0, emph1, txAmpShft};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.serdesManualTxConfig = serdesManualTxConfig;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                               sizeof(MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT),
                                               MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortPolaritySetIpc function
* @endinternal
*
* @brief   Send message to set the port polarity of the Serdes lanes (Tx/Rx).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txInvMask                - bitmap of 32 bit, each bit represent Serdes
* @param[in] rxInvMask                - bitmap of 32 bit, each bit represent Serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPolaritySetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_32                txInvMask,
    GT_32                rxInvMask
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT portPolaritySet =
        {(GT_U16)portGroup,(GT_U16)phyPortNum,portMode,txInvMask,rxInvMask};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portPolaritySet = portPolaritySet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_POLARITY_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortFecConfigIpc function
* @endinternal
*
* @brief   Send message to configure FEC disable/enable on port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portFecType              - GT_TRUE for FEC enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecConfigIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_FEC_MODE portFecType
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT portFecConfig = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, portFecType};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portFecConfig = portFecConfig;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_FEC_CONFIG_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortFecConfigGetIpc function
* @endinternal
*
* @brief   Send message to get FEC status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] fecMode                  - (pointer to) FEC mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecConfigGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_FEC_MODE *fecMode
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portFecConfigGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portFecConfigGet = portFecConfigGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG));

    /* Update reply parameters */
    *fecMode = replyData.readData.portStatusGet.status;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortTxEnableIpc function
* @endinternal
*
* @brief   Turn of the port Tx according to selection.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - enable/disable port Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              enable
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT            requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT           replyData;
    MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT portTxEnableData = {(GT_U16)portGroup,(GT_U16)phyPortNum,portMode,enable};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portTxEnableData = portTxEnableData;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_TX_ENABLE_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortTxEnableGetIpc function
* @endinternal
*
* @brief   Retrieve the status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              serdesTxStatus[]
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portTxEnableGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portTxEnableGet = portTxEnableGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        serdesTxStatus[i] = replyData.readData.portSerdesTxEnableGet.status[i];
    }

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortSignalDetectGetIpc function
* @endinternal
*
* @brief   Retrieve the status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSignalDetectGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              signalDetect[]
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portSignalDetectGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portSignalDetectGet = portSignalDetectGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        signalDetect[i] = replyData.readData.portSerdesSignalDetectGet.status[i];
    }

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortCdrLockStatusGetIpc function
* @endinternal
*
* @brief   Send message to get the CDR lock status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCdrLockStatusGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              cdrLockStatus[]
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portCdrLockStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portCdrLockStatus = portCdrLockStatus;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        cdrLockStatus[i] = replyData.readData.portSerdesCdrLockStatusGet.status[i];
    }

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortLoopbackSetIpc function
* @endinternal
*
* @brief   Send message to activates the port loopback modes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*                                      lpType     - loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackSetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_UNIT          lpPlace,
    MV_HWS_PORT_LB_TYPE  lbType
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;
    MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT portLoopbackSet =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, lpPlace, lbType};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portLoopbackSet = portLoopbackSet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_LOOPBACK_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortFlowControlStateSetIpc function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlowControlStateSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;
    MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT portFcStateSet =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, fcState};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portFcStateSet = portFcStateSet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_FC_STATE_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortLoopbackStatusGetIpc function
* @endinternal
*
* @brief   Send IPC message to retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackStatusGetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_UNIT             lpPlace,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;
    MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT portLoopbackGet =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, lpPlace};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portLoopbackGet = portLoopbackGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG));

    *lbType = replyData.readData.portLoopbackStatusGet.lbType;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortPPMSetIpc function
* @endinternal
*
* @brief   Send IPC message to increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPPM                  - limited to +/- 3 taps
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMSetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PPM_VALUE     portPPM
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT portPPMSet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, portPPM};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portPPMSet = portPPMSet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_PPM_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortPPMGetIpc function
* @endinternal
*
* @brief   Send message to check the entire line configuration
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] portPPM                  - current PPM
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PPM_VALUE     *portPPM
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portPPMGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portPPMGet = portPPMGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_PPM_GET_MSG));

    *portPPM = replyData.readData.portPpmGet.portPpm;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortInterfaceGetIpc function
* @endinternal
*
* @brief   Send message to gets Interface mode and speed of a specified port.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number (or CPU port)
*
* @param[out] portModePtr              - interface mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInterfaceGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD *portModePtr
)
{
#if 0
    MV_HWS_IPC_CTRL_MSG_STRUCT         requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT        replyData;
    MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT portInterfaceGet = {(GT_U16)portGroup, (GT_U16)phyPortNum};

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portInterfaceGet = portInterfaceGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_IF_GET_MSG));

    *portModePtr = replyData.readData.portIfGet.portIf;

    return (replyData.returnCode);
#else
    /* avoid warnings */
    devNum      = devNum;
    portGroup   = portGroup;
    phyPortNum  = phyPortNum;
    portModePtr = portModePtr;

    /* this functionality cannot be ran in internal cpu since it requires port
        elements info for all port modes which doesn't exist in internal CPU */
    return GT_NOT_SUPPORTED;
#endif
}

/**
* @internal mvHwsPortPcsActiveStatusGetIpc function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0 if PCS unit is under RESET.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPcsActiveStatusGetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT                          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT                         replyData;
    MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT   portPcsActiveStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /*construct the msg*/
    requestMsg.msgData.portPcsActiveStatus = portPcsActiveStatus;

    /* send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply (devNum,&requestMsg,&replyData,
                                                 sizeof(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT),
                                                 MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG));

    *numOfLanes = replyData.readData.portReplyGet.results;

    return (replyData.returnCode);
}

GT_STATUS mvHwsNoopIpc
(
    GT_U8                           devNum,
    GT_U32                          data
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.noop.data = data;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_NOOP_DATA_STRUCT),
                                                MV_HWS_IPC_NOOP_MSG));

    hwsOsPrintf("IPC NOOP rc=%d data=%d\n",replyData.returnCode,replyData.readData.noopReply.data);
    return (replyData.returnCode);
}



/**
* @internal mvHwsPortApStartIpc function
* @endinternal
*
* @brief   Send message to configure AP port parameters and start execution
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStartIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apCfg
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT     requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT    replyData;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT apCfgIpc;
    MV_HWS_AP_CFG                  *localApCfg = (MV_HWS_AP_CFG*)apCfg;
    GT_U16 phyPcsNum = mvHwsPortLaneMuxingGetMacFromSerdes(devNum,(GT_U8)portGroup,phyPortNum,(GT_U16)localApCfg->apLaneNum);

    apCfgIpc.portGroup          = (GT_U16)portGroup;
    apCfgIpc.phyPortNum         = (GT_U16)phyPortNum;
    apCfgIpc.advMode            = localApCfg->modesVector;
    apCfgIpc.options            = (GT_U16)localApCfg->fcPause       |
                                  (GT_U16)localApCfg->fcAsmDir << 1 |
                                  (GT_U16)localApCfg->fecSup   << 4 |
                                  (GT_U16)localApCfg->fecReq   << 5 |
                                  (GT_U16)localApCfg->nonceDis << 9 |
                                  (GT_U16)localApCfg->ctleBiasValue << 14,
    apCfgIpc.laneNum            = (GT_U8)localApCfg->apLaneNum;
    apCfgIpc.pcsNum             = (GT_U8)phyPcsNum;                                     /* pcs num OF THE NEGOTIATION lane*/
    apCfgIpc.macNum             = (GT_U8)phyPortNum;
    apCfgIpc.polarityVector     = (GT_U8)localApCfg->polarityVector;
    apCfgIpc.refClockSrcParams  = (GT_U8)localApCfg->refClockCfg.isValid |              /* valid serdes number */
                                  (GT_U8)localApCfg->refClockCfg.cpllOutFreq << 1 |     /* cpll out frequency*/
#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
                                  (GT_U8)localApCfg->refClockCfg.refClockSource << 4 |  /* reference clock source*/
                                  (GT_U8)localApCfg->refClockCfg.refClockFreq << 5;     /* reference clock freq*/
#else
                                  (GT_U8)localApCfg->refClockCfg.refClockSource << 4;    /* reference clock source*/
#endif

    /** [10:10]consortium (RS-FEC) ability (F1)
    ** [11:11]consortium (BASE-R FEC) ability (F2)
    ** [12:12]consortium (RS-FEC) request (F3)
    ** [13:13]consortium (BASE-R FEC) request (F4)*/
    AP_CTRL_RS_FEC_REQ_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_RS)? 1:0);
    AP_CTRL_FC_FEC_REQ_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_FC)? 1:0);
    AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_RS)? 1:0);
    AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_FC)? 1:0);

    if(hwsDeviceSpecInfo[devNum].devType != Bobcat3 && hwsDeviceSpecInfo[devNum].devType != Aldrin && hwsDeviceSpecInfo[devNum].devType != Aldrin2 && hwsDeviceSpecInfo[devNum].devType != Pipe)
    {
        apCfgIpc.pcsNum = (GT_U8)phyPortNum;
    }
    if((AP_CTRL_ADV_50G_CONSORTIUM_GET(apCfgIpc.advMode)) &&
    (AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(apCfgIpc.options)) &&
    ((phyPortNum % 4) != 0))
    {
        /* only slot 0 in MSPG can work with 50G and FC-FEC mode */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    AP_CTRL_FEC_ADVANCED_REQ_SET(apCfgIpc.options, (localApCfg->fecAdvanceReq>>FEC_ADVANCE_BASE_R_SHIFT));

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apConfig = apCfgIpc;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AP_ENABLE_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStopIpc function
* @endinternal
*
* @brief   Send message to stop AP port execution
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStopIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT     requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT    replyData;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT apPortCfg = {(GT_U16)portGroup, (GT_U16)phyPortNum, 0, 0, 0, 0, 0, 0, 0};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apConfig = apPortCfg;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AP_DISABLE_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApSysAckIpc function
* @endinternal
*
* @brief   Send message to notify AP state machine that port resource allocation
*         was executed by the Host and it can continue execution
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSysAckIpc
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_PORT_INFO_STRUCT apPortSysCfg = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* Construct the msg */
    requestMsg.msgData.apSysConfig = apPortSysCfg;

    /* Send message to HWS and don't wait for the reply */
    CHECK_STATUS(mvHwsIpcSendMsgNoReply(devNum,&requestMsg,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG));

    return GT_OK;
}

/**
* @internal mvHwsPortApCfgGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port configuration parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApCfgGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apCfg
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT       requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT      replyData;
    MV_HWS_IPC_PORT_REPLY_AP_CFG_GET *apCfgIpc;
    MV_HWS_AP_CFG                    *apCfgReply = (MV_HWS_AP_CFG*)apCfg;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT   apPortCfg = {(GT_U16)portGroup, (GT_U16)phyPortNum, 0, 0, 0, 0, 0, 0, 0};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apConfig = apPortCfg;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AP_CFG_GET_MSG));

    apCfgIpc = &(replyData.readData.portApCfgGet);
    apCfgReply->fecAdvanceReq = 0;
    apCfgReply->fecAdvanceAbil = 0;
    /* Update reply parameters */
    apCfgReply->apLaneNum   = AP_CTRL_LANE_GET(apCfgIpc->ifNum);
    apCfgReply->modesVector = apCfgIpc->capability;
    apCfgReply->fcPause     = AP_CTRL_FC_PAUSE_GET(apCfgIpc->options);
    apCfgReply->fcAsmDir    = AP_CTRL_FC_ASM_GET(apCfgIpc->options);
    apCfgReply->fecSup      = AP_CTRL_FEC_ABIL_GET(apCfgIpc->options);
    apCfgReply->fecReq      = AP_CTRL_FEC_REQ_GET(apCfgIpc->options);
    apCfgReply->fecAdvanceReq |= AP_CTRL_FEC_ADVANCED_REQ_GET(apCfgIpc->options) << FEC_ADVANCE_BASE_R_SHIFT;
    /*apCfgReply->fecAdvanceAbil = */
    apCfgReply->fecAdvanceReq  |= (AP_CTRL_RS_FEC_REQ_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->fecAdvanceReq  |= (AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->fecAdvanceAbil |= (AP_CTRL_RS_FEC_ABIL_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->fecAdvanceAbil |= (AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->nonceDis    = AP_CTRL_LB_EN_GET(apCfgIpc->options);
    apCfgReply->refClockCfg.refClockFreq   = MHz_156;
    apCfgReply->refClockCfg.refClockSource = PRIMARY_LINE_SRC;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStatusGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port status parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apStatus                 - AP Status parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatusGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apStatus
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET *apStatusIpc;
    MV_HWS_AP_PORT_STATUS               *apStatusReply = (MV_HWS_AP_PORT_STATUS*)apStatus;
    MV_HWS_IPC_PORT_INFO_STRUCT         apPortStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apStatusGet = apPortStatus;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_STATUS_MSG));

    apStatusIpc = &(replyData.readData.portApStatusGet);

    /* Update reply parameters */
    apStatusReply->apLaneNum                = apStatusIpc->laneNum;
    apStatusReply->smState                  = apStatusIpc->state;
    apStatusReply->smStatus                 = apStatusIpc->status;
    apStatusReply->arbStatus                = apStatusIpc->ARMSmStatus;
    apStatusReply->hcdResult.hcdFound       = AP_ST_HCD_FOUND_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdLinkStatus  = AP_ST_HCD_LINK_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdResult      = AP_ST_HCD_TYPE_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdFecEn       = (AP_ST_HCD_FEC_RES_GET(apStatusIpc->hcdStatus)
                                               != AP_ST_HCD_FEC_RES_NONE) ? 1 : 0;
    apStatusReply->hcdResult.hcdFecType     = AP_ST_HCD_FEC_RES_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdFcRxPauseEn = AP_ST_HCD_FC_RX_RES_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdFcTxPauseEn = AP_ST_HCD_FC_TX_RES_GET(apStatusIpc->hcdStatus);

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStatsGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port stats parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apStats                  - AP Stats parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatsGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apStats
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT         requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT        replyData;
    MV_HWS_IPC_PORT_REPLY_AP_STATS_GET *apStatsIpc;
    MV_HWS_AP_PORT_STATS               *apStatsReply = (MV_HWS_AP_PORT_STATS*)apStats;
    MV_HWS_IPC_PORT_INFO_STRUCT        apPortStats = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apStatsGet = apPortStats;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_STATS_MSG));

    apStatsIpc = &(replyData.readData.portApStatsGet);

    /* Update reply parameters */
    apStatsReply->txDisCnt          = apStatsIpc->txDisCnt;
    apStatsReply->abilityCnt        = apStatsIpc->abilityCnt;
    apStatsReply->abilitySuccessCnt = apStatsIpc->abilitySuccessCnt;
    apStatsReply->linkFailCnt       = apStatsIpc->linkFailCnt;
    apStatsReply->linkSuccessCnt    = apStatsIpc->linkSuccessCnt;
    apStatsReply->hcdResoultionTime = apStatsIpc->hcdResoultionTime;
    apStatsReply->linkUpTime        = apStatsIpc->linkUpTime;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStatsResetIpc function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatsResetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT apPortStats = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apStatsReset = apPortStats;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_STATS_RESET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApIntropSetIpc function
* @endinternal
*
* @brief   Set AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApIntropSetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apIntrop
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_AP_PORT_INTROP       *localApIntropCfg = (MV_HWS_AP_PORT_INTROP*)apIntrop;

    MV_HWS_IPC_PORT_AP_INTROP_STRUCT apIntropIpc =
    {
        (GT_U16)portGroup,
        (GT_U16)phyPortNum,
        NON_SUP_MODE,
        localApIntropCfg->attrBitMask,
        localApIntropCfg->txDisDuration,
        localApIntropCfg->abilityDuration,
        localApIntropCfg->abilityMaxInterval,
        localApIntropCfg->abilityFailMaxInterval,
        localApIntropCfg->apLinkDuration,
        localApIntropCfg->apLinkMaxInterval,
        localApIntropCfg->pdLinkDuration,
        localApIntropCfg->pdLinkMaxInterval
    };

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apIntropSet = apIntropIpc;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_INTROP_STRUCT),
                                                MV_HWS_IPC_PORT_AP_INTROP_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApIntropGetIpc function
* @endinternal
*
* @brief   Return AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApIntropGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apIntrop
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET *apIntropIpc;
    MV_HWS_AP_PORT_INTROP               *apIntropReply = (MV_HWS_AP_PORT_INTROP*)apIntrop;
    MV_HWS_IPC_PORT_INFO_STRUCT         apPortIntrop = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apIntropGet = apPortIntrop;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_INTROP_GET_MSG));

    apIntropIpc = &(replyData.readData.portApIntropGet);

    /* Update reply parameters */
    apIntropReply->txDisDuration          = apIntropIpc->txDisDuration;
    apIntropReply->abilityDuration        = apIntropIpc->abilityDuration;
    apIntropReply->abilityMaxInterval     = apIntropIpc->abilityMaxInterval;
    apIntropReply->abilityFailMaxInterval = apIntropIpc->abilityFailMaxInterval;
    apIntropReply->apLinkDuration         = apIntropIpc->apLinkDuration;
    apIntropReply->apLinkMaxInterval      = apIntropIpc->apLinkMaxInterval;
    apIntropReply->pdLinkDuration         = apIntropIpc->pdLinkDuration;
    apIntropReply->pdLinkMaxInterval      = apIntropIpc->pdLinkMaxInterval;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApDebugGetIpc function
* @endinternal
*
* @brief   Return AP debug information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apDebug                  - AP debug parameters
*
* @param[out] apDebug                  - AP debug parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApDebugGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apDebug
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT   requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT  replyData;
    MV_HWS_IPC_LOG_GET           *apDebugIpc;
    MV_HWS_FW_LOG                *apDebugReply = (MV_HWS_FW_LOG*)apDebug;
    MV_HWS_IPC_PORT_INFO_STRUCT  apPortDebug = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.logGet = apPortDebug;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG));

    apDebugIpc = &(replyData.readData.logGet);

    /* Update reply parameters */
    apDebugReply->fwBaseAddr       = apDebugIpc->fwBaseAddr;
    apDebugReply->fwLogBaseAddr    = apDebugIpc->logBaseAddr;
    apDebugReply->fwLogCountAddr   = apDebugIpc->logCountAddr;
    apDebugReply->fwLogPointerAddr = apDebugIpc->logPointerAddr;
    apDebugReply->fwLogResetAddr   = apDebugIpc->logResetAddr;

    return (replyData.returnCode);
}

/**
* @internal mvHwsLogGetIpc function
* @endinternal
*
* @brief   Return FW Hws log information
*
* @param[in] devNum                   - system device number
* @param[in] hwsLogPtr                - hws log parameters pointer
*
* @param[out] hwsLogPtr                - hws log parameters pointer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLogGetIpc
(
    GT_U8  devNum,
    GT_U32 *hwsLogPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT    requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT   replyData;
    MV_HWS_IPC_LOG_GET            *ipcReplay;
    MV_HWS_FW_LOG                 *apDebugReply = (MV_HWS_FW_LOG*)hwsLogPtr;
    MV_HWS_IPC_PORT_INFO_STRUCT   hwsPortDebug = {0, 0, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.logGet = hwsPortDebug;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_HWS_LOG_GET_MSG));

    ipcReplay = &(replyData.readData.logGet);

    /* Update reply parameters */
    apDebugReply->fwBaseAddr       = ipcReplay->fwBaseAddr;
    apDebugReply->fwLogBaseAddr    = ipcReplay->logBaseAddr;
    apDebugReply->fwLogCountAddr   = ipcReplay->logCountAddr;
    apDebugReply->fwLogPointerAddr = ipcReplay->logPointerAddr;
    apDebugReply->fwLogResetAddr   = ipcReplay->logResetAddr;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortVosOVerrideParamsSetIpc function
* @endinternal
*
* @brief   Set the VOS Override params for the service CPU in the DB. This API allows
*         the overall VOS params to be sent in small buffers in order to keep
*         low memory consumption in the IPC.
* @param[in] devNum                   - system device number
* @param[in] vosOverrideParamsBufPtr  - (pointer to) VOS Override params to set
* @param[in] vosOverrideParamsBufIdx  - relative index of the params buffer in the
*                                      overall VOS Override params DB.
* @param[in] vosOverrideParamsBufLength - length of the buffer array.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOVerrideParamsSetIpc
(
    GT_U8  devNum,
    GT_U32 *vosOverrideParamsBufPtr,
    GT_U32  vosOverrideParamsBufIdx,
    GT_U32  vosOverrideParamsBufLength
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_VOS_OVERRIDE_PARAMS      vosOverrideParams;

    /* Null pointer check */
    if (vosOverrideParamsBufPtr == NULL)
    {
        hwsOsPrintf("mvHwsPortVosOVerrideParamsSetIpc: vosOverrideParamsBufPtr is null for idx %d \n",
            vosOverrideParamsBufIdx);
        return GT_BAD_PARAM;
    }

    /* initialize replyData, requestMsg and vosOverrideParams */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&requestMsg, 0, sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&vosOverrideParams, 0, sizeof(MV_HWS_IPC_VOS_OVERRIDE_PARAMS));

    /* Setting the data */
    vosOverrideParams.vosOverrideParamsBufferIdx = vosOverrideParamsBufIdx;
    vosOverrideParams.vosOverrideParamsBufferLength = vosOverrideParamsBufLength;
    hwsOsMemCopyFuncPtr(&(vosOverrideParams.vosOverrideParamsBufferPtr[0]),
                       vosOverrideParamsBufPtr, (vosOverrideParamsBufLength*(sizeof(GT_U32))));
    requestMsg.msgData.vosOverrideParams = vosOverrideParams;

    /* Sending the data */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_VOS_OVERRIDE_PARAMS),
                                                MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortVosOverrideModeSetIpc function
* @endinternal
*
* @brief   Set the VOS Override mode for the service CPU.
*
* @param[in] devNum                   - system device number
* @param[in] vosOverride              - Vos override mode, GT_TRUE - override enable, else override disabled.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideModeSetIpc
(
    GT_U8  devNum,
    GT_BOOL vosOverride
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT    vosOverrideModeSet;

    /* initialize replyData, requestMsg and vosOverrideParams */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&requestMsg, 0, sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&vosOverrideModeSet, 0, sizeof(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT));

    /* Setting the data */
    vosOverrideModeSet.vosOverrideMode = vosOverride;
    requestMsg.msgData.vosOverrideModeSet = vosOverrideModeSet;

    /* Sending the data */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT),
                                                MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApAvagoGuiSetIpc function
* @endinternal
*
* @brief   Set AP state machine state when Avago GUI is enabled
*         Avago GUI access Avago Firmware as SBUS command level
*         Therefore it is required to stop the periodic behiviour of AP state
*         machine when Avago GUI is enabled
* @param[in] devNum                   - system device number
* @param[in] state                    - Avago GUI state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApAvagoGuiSetIpc
(
    GT_U8 devNum,
    GT_U8 state
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.avagoGuiSet.state = (GT_U32)state;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT),
                                                MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortAvagoCfgAddrGetIpc function
* @endinternal
*
* @brief   Return Avago Serdes Configuration structure address
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @param[out] avagoCfgAddr             - Avago Serdes Configuration structure address
* @param[out] avagoCfgSize             - Avago Serdes Configuration structure size
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoCfgAddrGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *avagoCfgAddr,
    GT_U32 *avagoCfgSize
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT   requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT  replyData;
    MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET *avagoAddrIpc;
    MV_HWS_IPC_PORT_INFO_STRUCT  avagoAddr = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.avagoAddrGet = avagoAddr;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG));

    avagoAddrIpc = &(replyData.readData.portAvagoAaplGet);

    /* Update reply parameters */
    *avagoCfgAddr = avagoAddrIpc->addr;
    *avagoCfgSize = avagoAddrIpc->size;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortAvagoAaplInitIpc function
* @endinternal
*
* @brief   Init Avago Serdes Configuration structure pointers parameters, on service CPU, with NULL values.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoAaplInitIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT avagoAaplInitIpc = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.avagoAaplInit = avagoAaplInitIpc;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApEnablePortCtrlSetIpc function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApEnablePortCtrlSetIpc
(
    GT_U8        devNum,
    GT_BOOL      srvCpuEnable
)
{

    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.portEnableCtrlSet.portEnableCtrl = (GT_U32)srvCpuEnable;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApEnablePortCtrlGetIpc function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApEnablePortCtrlGetIpc
(
    GT_U8        devNum,
    GT_BOOL      *srvCpuEnablePtr
)
{

    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portInfo = {0, 0, NON_SUP_MODE};
    MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT *portEnableCtrlIpc;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

     /* Construct the msg */
    requestMsg.msgData.avagoAddrGet = portInfo;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG));

    portEnableCtrlIpc = &(replyData.readData.portEnableCtrlGet);

    *srvCpuEnablePtr = (GT_BOOL)portEnableCtrlIpc->portEnableCtrl;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApSerdesTxParametersOffsetSetIpc function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - serdes TX values offsets
* @param[in] serdesSpeed              port speed assicoated with the offsets
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesTxParametersOffsetSetIpc
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    GT_U32        serdesNumber,
    GT_U16        offsets,
    MV_HWS_SERDES_SPEED serdesSpeed
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC portSerdesTxParametersOffseConfig =
            {(GT_U16)portGroup, (GT_U16)phyPortNum, (GT_U32)serdesSpeed, (GT_U8)serdesNumber, offsets};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.portSerdesTxParametersOffsetConfig = portSerdesTxParametersOffseConfig;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC),
                                                MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG));

#ifndef ASIC_SIMULATION
    /* those parameters will abe host to read tx-offsets database directly from FW memory */
    txParamesOffsetsFwBaseAddr = replyData.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetBaseAddr;
    txParamesOffsetsFwSize = replyData.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetSize;
#endif

    return (replyData.returnCode);
}


/**
* @internal mvHwsPortApSerdesRxParametersManualSetIpc function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesRxParametersManualSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE *rxCfgPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC rxCfgOverride;

    if (rxCfgPtr == NULL) {
        return GT_FAIL;
    }
    rxCfgOverride.portGroup = (GT_U16)portGroup;
    rxCfgOverride.phyPortNum = (GT_U16)apPortNum;
    rxCfgOverride.rxCfg.ctleParams.bandWidth     = rxCfgPtr->ctleParams.bandWidth;
    rxCfgOverride.rxCfg.ctleParams.loopBandwidth = rxCfgPtr->ctleParams.loopBandwidth;
    rxCfgOverride.rxCfg.ctleParams.dcGain        = rxCfgPtr->ctleParams.dcGain;
    rxCfgOverride.rxCfg.ctleParams.highFrequency = rxCfgPtr->ctleParams.highFrequency;
    rxCfgOverride.rxCfg.ctleParams.lowFrequency  = rxCfgPtr->ctleParams.lowFrequency;
    rxCfgOverride.rxCfg.ctleParams.squelch       = rxCfgPtr->ctleParams.squelch;
    rxCfgOverride.rxCfg.serdesSpeed              = rxCfgPtr->serdesSpeed;
    rxCfgOverride.rxCfg.etlParams.etlMinDelay    = rxCfgPtr->etlParams.etlMinDelay;
    rxCfgOverride.rxCfg.etlParams.etlMaxDelay    = rxCfgPtr->etlParams.etlMaxDelay;
    rxCfgOverride.rxCfg.etlParams.etlEnableOverride = rxCfgPtr->etlParams.etlEnableOverride;
    rxCfgOverride.rxCfg.fieldOverrideBmp         = rxCfgPtr->fieldOverrideBmp;



    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.portSerdesRxConfigOverride = rxCfgOverride;
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC),
                                                MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG));

    return (replyData.returnCode);
}



