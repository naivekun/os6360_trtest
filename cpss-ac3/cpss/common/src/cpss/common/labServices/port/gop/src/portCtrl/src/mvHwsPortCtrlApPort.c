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
* @file mvHwsPortCtrlApPort.c
*
* @brief Port Control AP Port State Machine
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <mvHwsPortCtrlInit.h>

static MV_PORT_CTRL_AP_CALLBACKS mvPortCtrlApCallbacks = {NULL,NULL};

static MV_HWS_PORT_CTRL_COMMON *infoPtr;

GT_STATUS mvPortCtrlApPortPendMsg(GT_U32 port, MV_HWS_IPC_CTRL_MSG_AP_STRUCT **msg,
                                  GT_U32 *msgType, GT_U32 action);
GT_STATUS mvPortCtrlApPortPendAdd(GT_U32 port, MV_HWS_IPC_CTRL_MSG_AP_STRUCT *msg);

void mvPortCtrlPortActiveExecTraining(GT_U32 targetPort,
                                      MV_HWS_PORT_CTRL_PORT_SM *portCtrlSmPtr);
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

extern MV_HWS_PORT_STANDARD mvHwsApConvertPortMode(MV_HWA_AP_PORT_MODE apPortMode);
extern GT_BOOL mvHwsApPortEnableCtrl;

static GT_STATUS mvPortCtrlApPortResetExec
(
    GT_U32 port,
    MV_HWS_PORT_CTRL_PORT_SM *info,
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT *msg
);

static void mvPortCtrlApPortFail
(
    char                           *errorMsg,
    GT_U32                          portIndex,
    GT_U32                         *msgType,
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT  *recvMsg,
    GT_U8                           status
);

/**
 *
 * mvPortCtrlApCallbacksInit
 *
 * @param apCallbacksPtr
 *
 * @return GT_STATUS
 */
GT_STATUS mvPortCtrlApRegisterCallbacks(MV_PORT_CTRL_AP_CALLBACKS *apCallbacksPtr)
{
    if (apCallbacksPtr != NULL) {
        mvPortCtrlApCallbacks = *apCallbacksPtr;
    }
    return GT_OK;
}

/**
* @internal mvPortCtrlApPortRoutine function
* @endinternal
*
* @brief   AP Port mng process execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvPortCtrlApPortRoutine(void *pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvHwsPortCtrlApPortMng.info);

    for (;;)
    {
        /*
        ** AP Port Mng State Machine Transitions Table
        ** +================+=============+=============+=============+
        ** + Event \ State  +  Active O1  +   Msg O2    +  Delay O3   +
        ** +================+=============+=============+=============+
        ** + Active         +     O1      +   Invalid   +    ==> O1   +
        ** +================+=============+=============+=============+
        ** + Msg            +   ==> O2    +     O2      +   Invalid   +
        ** +================+=============+=============+=============+
        ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
        ** +================+=============+=============+=============+
        **
        **
        ** Port SM State Machine Transitions
        ** +============================+=============+=============+=============+=============+=============+
        ** + Event \ State              + Idle O0     + Serdes      + Serdes      + MAC / PCS   + Active O4   +
        ** +                            +             + Config O1   + Training O2 + Config O3   +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Start Execute              +   ==> O1    +             +             +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Serdes Config in Progress  +             +     O1      +             +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Serdes Config Success      +             +    ==> O2   +             +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Serdes Config Failure      +             +    ==> O0   +             +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Serdes Train in Progress   +             +             +     O2      +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Serdes Train Success       +             +             +    ==> O3   +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Serdes Train Failure       +             +             +    ==> O0   +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Serdes Train end Wait Ack  +             +             +    ==> O3   +             +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + MAC/PCS Config in Progress +             +             +             +     O3      +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + MAC/PCS Config Success     +             +             +             +    ==> O4   +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + MAC/PCS Config Failure     +             +             +             +    ==> O0   +             +
        ** +============================+=============+=============+=============+=============+=============+
        ** + Delete Success             +     O0      +    ==> O0   +    ==> O0   +    ==> O0   +    ==> O0   +
        ** +============================+=============+=============+=============+=============+=============+
        */
        if (mvHwsPortCtrlApPortMng.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvHwsPortCtrlApPortMng.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Ap Port Ctrl, Port Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
        hwsOsTimerWkFuncPtr(1);
    }
}

/**
* @internal mvApPolarityCfg function
* @endinternal
*
* @brief   Configure polarity of serdes lanes of port
*
* @retval GT_OK                    - on success
* @retval otherwise                - error
*/
static GT_STATUS mvApPolarityCfg(GT_U8 portNum,  MV_HWS_PORT_STANDARD portMode, GT_U8 polarityVector)
{
    GT_STATUS               status;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8                   i;
    GT_BOOL                 polarityTx;
    GT_BOOL                 polarityRx;

    if (polarityVector)
    {
        if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, (GT_U32)portNum, portMode, &curPortParams))
        {
           return GT_BAD_PTR;
        }

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            polarityTx = polarityVector & (1 << (2 * i + 1));
            polarityRx = polarityVector & (1 << (2 * i));
            if (polarityTx || polarityRx)
            {
                status = mvHwsSerdesPolarityConfig(0, 0,
                                                   curPortParams.activeLanesList[i],
                                                   HWS_DEV_SERDES_TYPE(0),
                                                   polarityTx,
                                                   polarityRx);
                if (status != GT_OK)
                {
                    return status;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvPortCtrlApPortResetExec function
* @endinternal
*
* @brief   Port mng message processing - reset execution
*         - Execute port reset and return to Idle state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvPortCtrlApPortResetExec(GT_U32 port,
                                           MV_HWS_PORT_CTRL_PORT_SM *info,
                                           MV_HWS_IPC_CTRL_MSG_AP_STRUCT *msg)
{
    if(info->status == PORT_SM_DELETE_IN_PROGRESS)
    {
        return GT_OK;
    }

    /*TBD - Can be remove? Done on AP task*/
    if (mvHwsPortEnableSet(0/*dev*/, 0/*portGroup*/, (GT_U32)msg->phyPortNum,
                           (MV_HWS_PORT_STANDARD)msg->portMode, GT_FALSE) != GT_OK)
    {
        hwsOsPrintf("mvHwsPortEnableSet:%d-fail\n", msg->phyPortNum);
    }

    if (PORT_SM_SERDES_TRAINING_IN_PROGRESS == info->status)
    {
        /* Execute HWS Training check */
        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                 0, /* portGroup */
                                 (GT_U32)msg->phyPortNum,
                                 (MV_HWS_PORT_STANDARD)msg->portMode,
                                 TRxTuneStop,
                                 NULL) != GT_OK)
        {
            info->status = PORT_SM_SERDES_TRAINING_FAILURE;
            mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));
            /* still don't exit - try to delete port */
        }

        /* let training finish */
        hwsOsTimerWkFuncPtr(20);
    }

    info->status = PORT_SM_DELETE_IN_PROGRESS;
    mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));

    /* Execute HWS Port Reset */
    if (mvHwsPortReset(0, /* devNum */
                       0, /* portGroup */
                       (GT_U32)msg->phyPortNum,
                       (MV_HWS_PORT_STANDARD)msg->portMode,
                       PORT_POWER_DOWN) != GT_OK)
    {
        info->status = PORT_SM_DELETE_FAILURE;
        mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));

        return GT_FAIL;
    }

    return GT_OK;
}
#if (defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
GT_STATUS mvPortCtrlApPortEtlParam(GT_U32 portIndex, MV_HWS_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    MV_HWS_PORT_CTRL_PORT_SM            *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    GT_U16                              index;
    GT_BOOL                             ctleOverride = GT_FALSE;

    if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, (GT_U32)recvMsg->phyPortNum, (MV_HWS_PORT_STANDARD)recvMsg->portMode, &curPortParams))
    {
        return GT_FAIL;
    }

    /*Set Default values */
    portSm->apPortEtlParam.etlMode = GT_TRUE;
    portSm->apPortEtlParam.etlMinDelay = 0;
    portSm->apPortEtlParam.etlMaxDelay = 0;

    if (curPortParams.serdesSpeed == _10_3125G)
    {
        index = MV_HWS_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS;
        ctleOverride = GT_TRUE;
    }
    else if (curPortParams.serdesSpeed == _25_78125G)
    {
        index = MV_HWS_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS;
        ctleOverride = GT_TRUE;
    }

    if (ctleOverride == GT_TRUE)
    {
        if (portSm->apPortRxConfigOverride[index].fieldOverrideBmp & MV_HWS_MAN_TUNE_ETL_ENABLE_CNS)
        {
            portSm->apPortEtlParam.etlMode = (GT_BOOL)((portSm->apPortRxConfigOverride[index].etlMaxDelay >> MV_HWS_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT) & 1);
        }
        if (portSm->apPortRxConfigOverride[index].fieldOverrideBmp & MV_HWS_MAN_TUNE_ETL_MIN_DELAY_CNS)
        {
            /*bit[7]=1 means use user value and not the default*/
            portSm->apPortEtlParam.etlMinDelay = (GT_BOOL)((portSm->apPortRxConfigOverride[index].etlMinDelay & 0x7F) | (1<<MV_HWS_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT));
        }
        if (portSm->apPortRxConfigOverride[index].fieldOverrideBmp & MV_HWS_MAN_TUNE_ETL_MAX_DELAY_CNS)
        {
            /*bit[7]=1 means use user value and not the default*/
            portSm->apPortEtlParam.etlMaxDelay = (GT_BOOL)((portSm->apPortRxConfigOverride[index].etlMaxDelay & 0x7F) | (1<<MV_HWS_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT));
        }
    }
    return GT_OK;
}
#endif

#ifndef BC2_DEV_SUPPORT
GT_STATUS mvPortCtrlApPortCtleOverride(GT_U32 portIndex, MV_HWS_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleData[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U16                              fieldOverrideBmp, index, i;
    GT_BOOL                             ctleOverride = GT_FALSE;
    MV_HWS_PORT_CTRL_PORT_SM            *singlePortSm;

    if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, (GT_U32)recvMsg->phyPortNum, (MV_HWS_PORT_STANDARD)recvMsg->portMode, &curPortParams))
    {
        return GT_FAIL;
    }

    if (curPortParams.serdesSpeed == _10_3125G)
    {
        index = MV_HWS_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS;
        ctleOverride = GT_TRUE;
    }
    else if (curPortParams.serdesSpeed == _25_78125G)
    {
        index = MV_HWS_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS;
        ctleOverride = GT_TRUE;
    }

    if (ctleOverride == GT_TRUE)
    {
        if (mvHwsPortManualCtleConfigGet(0, 0, (GT_U32)recvMsg->phyPortNum, (MV_HWS_PORT_STANDARD)recvMsg->portMode, 0xFF, ctleData) != GT_OK)
        {
            return GT_FAIL;
        }
        for (i = 0; i < (curPortParams.numOfActLanes); i++)
        {
            singlePortSm = &(mvHwsPortCtrlApPortMng.apPortSm[(GT_U32)recvMsg->phyPortNum + i]);
            if (singlePortSm->apPortRxConfigOverride[index].fieldOverrideBmp != 0)
            {
                fieldOverrideBmp = singlePortSm->apPortRxConfigOverride[index].fieldOverrideBmp;
                if (fieldOverrideBmp & MV_HWS_MAN_TUNE_CTLE_CONFIG_SQLCH_CNS)
                {
                    ctleData[i].squelch = singlePortSm->apPortRxConfigOverride[index].squelch;
                    AP_PRINT_MAC(("ExecTrainingInProgress squelch %d \n",ctleData[i].squelch));
                }
                if (fieldOverrideBmp & MV_HWS_MAN_TUNE_CTLE_CONFIG_LF_CNS)
                {
                    ctleData[i].lowFrequency = (singlePortSm->apPortRxConfigOverride[index].lfHf & 0xF);
                    AP_PRINT_MAC(("ExecTrainingInProgress lowFrequency %d \n",ctleData[i].lowFrequency));
                }
                if (fieldOverrideBmp & MV_HWS_MAN_TUNE_CTLE_CONFIG_HF_CNS)
                {
                    ctleData[i].highFrequency = ((singlePortSm->apPortRxConfigOverride[index].lfHf >> MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_HF_SHIFT) & 0xF);
                    AP_PRINT_MAC(("ExecTrainingInProgress highFrequency %d \n",ctleData[i].highFrequency));
                }
                if (fieldOverrideBmp & MV_HWS_MAN_TUNE_CTLE_CONFIG_DCGAIN_CNS)
                {
                    ctleData[i].dcGain = singlePortSm->apPortRxConfigOverride[index].dcGain;
                    AP_PRINT_MAC(("ExecTrainingInProgress dcGain %d \n",ctleData[i].dcGain));
                }
                if (fieldOverrideBmp & MV_HWS_MAN_TUNE_CTLE_CONFIG_BANDWIDTH_CNS)
                {
                    ctleData[i].bandWidth = (singlePortSm->apPortRxConfigOverride[index].bandLoopWidth & 0xF);
                    AP_PRINT_MAC(("ExecTrainingInProgress bandWidth %d \n",ctleData[i].bandWidth));
                }
                if (fieldOverrideBmp & MV_HWS_MAN_TUNE_CTLE_CONFIG_LOOPBANDWIDTH_CNS)
                {
                    ctleData[i].loopBandwidth = ((singlePortSm->apPortRxConfigOverride[index].bandLoopWidth >> MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_LOOPWIDTH_SHIFT) & 0xF);
                    AP_PRINT_MAC(("ExecTrainingInProgress loopBandwidth %d \n",ctleData[i].loopBandwidth));
                }
                if (mvHwsPortManualCtleConfig(0, 0, (GT_U32)recvMsg->phyPortNum, (MV_HWS_PORT_STANDARD)recvMsg->portMode, i, &ctleData[i]) != GT_OK)
                {
                    return GT_FAIL;
                }
            }
        }
    }
    return GT_OK;

}
#endif
/*******************************************************************************
* mvPortCtrlApPortActiveExec
* mvPortCtrlApPortActiveExecTrainingInProgress
* mvPortCtrlApPortActiveExecTrainingEndWaitAck
*
* DESCRIPTION: AP Port active state execution
*
*           State Description
*           - Check if Port state is Training
*               In case not in pending, exit
*
*           - Training State
*
*               Check if there are active messages in the queue
*                  Note: messages are not released until execution ends
*                  the port state defines how the message will be executed
*
*               - Training Status = In progress
*                   Check training threshold
*                     Threshold execeeded, Set result to FALSE
*                     Threshold Not execeeded
*                       Check training result
*                         Training result return fail, Set result to FALSE
*                         Training result return OK,
*                           Training In progress, Continue
*                           Training ended successfully, Set training result to TRUE
*                           Training ended with failure, Set training result to FALSE
*                     Training stop in case  Training success / failure / threshold exceeded
*                       Training stop return fail, Set result to FALSE
*
*                       Training failure / Training threshold exceeded
*                         Execute port reset sequence
*                         Training timer reset
*                         Release message
*                         Notify AP Detection Process - failure
*
*                       Training success
*                         Check for system Ack ("PIZZA" configuration)
*                           Ack received
*                             System config timer reset
*                             Release message
*                             Notify AP Detection Process - success
*                           Ack Not received
*                             System config timer start
*
*               - Training Status = Wait for system Ack
*                   Check for system Ack ("PIZZA" configuration)
*                     Ack received
*                       System config timer reset
*                       Release message
*                       Notify AP Detection Process - success
*                     Ack Not received
*                       Check system config threshold
*                         System config threshold exceeded
*                           Execute port reset sequence
*                           Training timer reset
*                           Release message
*                           Notify AP Detection Process - failure
*                         System config threshold Not exceeded
*                           Continue
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*******************************************************************************/

/*******************************************************************************
* mvPortCtrlApPortActiveExecTrainingInProgress
*
*******************************************************************************/
void mvPortCtrlApPortActiveExecTrainingInProgress(GT_U32 portIndex,
                                                  MV_HWS_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    GT_U32                         msgType;
    MV_HWS_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    GT_U32                         tuneResult = TUNE_NOT_COMPLITED;
    MV_HWS_AUTO_TUNE_STATUS_RES    tuneRes;
    MV_HWS_PORT_STANDARD           portMode;

    portMode = (MV_HWS_PORT_STANDARD)recvMsg->portMode;

    /* Check training threshold */
    if (!(mvPortCtrlThresholdCheck(portTimer->trainingThreshold)))
    {
        return;
    }

    portTimer->trainingCount++;

    /* Execute HWS Training check */
    AP_PRINT_MAC(("mvPortCtrlApPortActiveExecTrainingInProgress portIndex:%d  \n",portIndex));

    if (mvHwsPortAutoTuneSet(0, /* devNum */
                             0, /* portGroup */
                             (GT_U32)recvMsg->phyPortNum,
                             portMode,
                             TRxTuneStatusNonBlocking,
                             &tuneRes) == GT_OK)
    /* Execute HWS Training check */
    {
        if (tuneRes.txTune == TUNE_PASS)
        {
            /* Training success */
            tuneResult = TUNE_PASS;
            AP_PRINT_MAC(("\nmvPortCtrlApPortActiveExecTrainingInProgress portIndex:%d  TUNE_PASS***\n",portIndex));
        }
        else if (tuneRes.txTune == TUNE_FAIL)
        {
            /* Training failure */
            tuneResult = TUNE_FAIL;
            AP_PRINT_MAC(("mvPortCtrlApPortActiveExecTrainingInProgress portIndex:%d  TUNE_FAIL***\n",portIndex));
        }
    }

    mvPortCtrlDbgO_CoreStatusDump(portIndex, portSm->state);

    if ((tuneResult == TUNE_NOT_COMPLITED) &&
        (portTimer->trainingCount < portTimer->trainingMaxInterval))
    {
        /* Training timer restart */
        mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
        return;
    }

    /* Clear training timer */
    portTimer->trainingCount = 0;
    /* Training timer reset */
    mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));

    /* Training timer expire and training not complited */
    if (tuneResult == TUNE_NOT_COMPLITED)
    {
        /* Training failure */
        tuneResult = TUNE_FAIL;
    }

    /* Training success / failure / threshold exceeded */
    /* =============================================== */
    /*For avago serdes stop training is done only failure / threshold exceeded cases (this action causes link down)*/
#ifdef BC2_DEV_SUPPORT
    /* Execute HWS Training check */
    if (mvHwsPortAutoTuneSet(0, /* devNum */
                             0, /* portGroup */
                             (GT_U32)recvMsg->phyPortNum,
                             portMode,
                             TRxTuneStop,
                             NULL) != GT_OK)
    {
        /* Training failure */
        tuneResult = TUNE_FAIL;
    }
#else
    if (tuneResult == TUNE_FAIL)
    {
        mvHwsPortAutoTuneSet(0, /* devNum */
                             0, /* portGroup */
                             (GT_U32)recvMsg->phyPortNum,
                             portMode,
                             TRxTuneStop,
                             NULL);
    }
#endif

    /* Training failure / Training threshold exceeded */
    if (tuneResult == TUNE_FAIL)
    {
        /* let training finish */
        hwsOsTimerWkFuncPtr(20);

        mvPortCtrlApPortFail("Error, Port-%d TRX training Failed\n",
                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_FAILURE);
        return;
    }

    /* TRX-Training success */
    else if (tuneResult == TUNE_PASS)
    {
        portSm->state  = PORT_SM_ACTIVE_STATE;
        portSm->status = PORT_SM_SERDES_TRAINING_SUCCESS;
        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

#ifdef BC2_DEV_SUPPORT /* for AC3 doorbell interrupt not supported yet - must enable port here unconditionally. NOTE: BC2_DEV_SUPPORT common for AC3 and BC2 */
        if (Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
        {
            /* Release message */
            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
        }
        else /*BC2*/
        {
            /* Check System Config */
            if (mvHwsApPortEnableCtrl)
            {
                portSm->status = PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK;
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                if ((portTimer->sysCfState == PORT_SM_SERDES_SYSTEM_DISABLE) ||
                    ((portTimer->sysCfState  == PORT_SM_SERDES_SYSTEM_ENABLE) &&
                     (portTimer->sysCfStatus == PORT_SM_SERDES_SYSTEM_VALID) &&
                     ((portMode == portTimer->sysCfMode) || (portMode == _25GBase_KR_S && portTimer->sysCfMode ==_25GBase_KR) || (portMode == _25GBase_CR_S && portTimer->sysCfMode ==_25GBase_CR))))
                {
                    portSm->status = PORT_SM_SERDES_TRAINING_END_SYS_ACK_RCV;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                    /* Ready to get traffic */
                    if (mvHwsPortEnableSet(0, 0/*portGroup*/, recvMsg->phyPortNum, portMode, GT_TRUE) != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port-%d enable Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_PORT_ENABLE_FAIL);
                    }
                    else /*Done*/
                    {
                        /* Release message */
                        mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                    }
                }
                else
                {
                    /* System config timer start */
                    mvPortCtrlThresholdSet(PORT_CTRL_SYSTEM_CONFIG_DURATION, &(portTimer->sysCfgThreshold));
                }
            }
            else /*Done*/
            {
                /* Release message */
                mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
            }
        }
#else/* #if BC2_DEV_SUPPORT */

        /* Check System Config */
        if (mvHwsApPortEnableCtrl)
        {
            portSm->status = PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK;
            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

            if ((portTimer->sysCfState == PORT_SM_SERDES_SYSTEM_DISABLE) ||
                ((portTimer->sysCfState  == PORT_SM_SERDES_SYSTEM_ENABLE) &&
                 (portTimer->sysCfStatus == PORT_SM_SERDES_SYSTEM_VALID) &&
                 ((portMode == portTimer->sysCfMode) || (portMode == _25GBase_KR_S && portTimer->sysCfMode ==_25GBase_KR) || (portMode == _25GBase_CR_S && portTimer->sysCfMode ==_25GBase_CR))))
            {
                portSm->status = PORT_SM_SERDES_TRAINING_END_SYS_ACK_RCV;
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                /* Ready to get traffic */
                if (mvHwsPortEnableSet(0, 0/*portGroup*/, recvMsg->phyPortNum, portMode, GT_TRUE) != GT_OK)
                {
                    mvPortCtrlApPortFail("Error, Port-%d enable Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_PORT_ENABLE_FAIL);
                    return;
                }
            }
            else
            {
                /* System config timer start */
                mvPortCtrlThresholdSet(PORT_CTRL_SYSTEM_CONFIG_DURATION, &(portTimer->sysCfgThreshold));
                return;
            }
        }
        if (mvPortCtrlApPortCtleOverride(portIndex,recvMsg)!= GT_OK)
        {
             mvPortCtrlApPortFail("Error, Port-%d ctle config Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_CTLE_ETL_CONFIG_FAIL);
             return;
        }
#if defined(BOBK_DEV_SUPPORT)

        /* Start continuous Adaptive RX training after finish the EnhanceTuneLite */
        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                 0, /* portGroup */
                                 (GT_U32)recvMsg->phyPortNum,
                                 portMode,
                                 RxTrainingAdative,
                                 NULL) != GT_OK)
        {
            mvPortCtrlApPortFail("Error, Port-%d Adaptive training start Failed\n",
                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL);

        }
        else /*Done*/
        {
            /* Release message */
            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
        }
#else
        if (mvPortCtrlApPortEtlParam(portIndex,recvMsg)!= GT_OK)
        {
             mvPortCtrlApPortFail("Error, Port-%d ctle config Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_CTLE_ETL_CONFIG_FAIL);
             return;
        }

        if(portSm->apPortEtlParam.etlMode == GT_TRUE)
        {
            portSm->status = PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS;
            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

            /* Use Training timer variables for ETL as well - reset it */
            portTimer->trainingCount = 0;
            mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
            /*reset ETL DB*/
            mvHwsPortEnhanceTuneLiteByPhaseInitDb(0,MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET((GT_U32)recvMsg->phyPortNum, MV_PORT_CTRL_AP_PORT_NUM_BASE));

        }
        else
        {
            /* Start continuous Adaptive RX training after finish the EnhanceTuneLite */
            if (mvHwsPortAutoTuneSet(0, /* devNum */
                                     0, /* portGroup */
                                     (GT_U32)recvMsg->phyPortNum,
                                     portMode,
                                     RxTrainingAdative,
                                     NULL) != GT_OK)
            {
                mvPortCtrlApPortFail("Error, Port-%d Adaptive training start Failed\n",
                                     portIndex, &msgType, recvMsg, PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL);
            }
            else /*Done*/
            {
                /* Release message */
                mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
            }
        }
#endif /* #if BOBK_DEV_SUPPORT */
#endif /* #else */
    }
}

/*******************************************************************************
* mvPortCtrlApPortActiveExecTrainingNotReady
*
*******************************************************************************/
void mvPortCtrlApPortActiveExecTrainingNotReady(GT_U32 portIndex,
                                                MV_HWS_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    GT_U32                         msgType;
    MV_HWS_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MV_HWS_AUTO_TUNE_STATUS_RES    tuneRes;

    tuneRes.txTune = TUNE_FAIL;
    if (portTimer->trainingCount >= portTimer->trainingMaxInterval)
    {
        /* for Avago this call does nothing */
        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                 0, /* portGroup */
                                 (GT_U32)recvMsg->phyPortNum,
                                 (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                 TRxTuneStop,
                                 NULL) != GT_OK)
        {
            /* Training failure */
            tuneRes.txTune = TUNE_FAIL;
        }
    }
    else
    {
        portTimer->trainingCount++;

        /* Execute HWS Training start */
        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                 0, /* portGroup */
                                 (GT_U32)recvMsg->phyPortNum,
                                 (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                 TRxTuneStart,
                                 &tuneRes) != GT_OK)
        {
            tuneRes.txTune = TUNE_FAIL;
        }
    }

    if (tuneRes.txTune == TUNE_NOT_READY)
    {
        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
    }
    else if ((tuneRes.txTune == TUNE_PASS) || (tuneRes.txTune == TUNE_READY))
    {
        portSm->status = PORT_SM_SERDES_TRAINING_IN_PROGRESS;
        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
        /* Training timer start */
        portTimer->trainingCount = 0;
        mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
    }
    else
    {
        mvPortCtrlApPortFail("Error, Port Mng ExecTrainingNotReady, Port-%d training starting Failed\n",
                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_FAILURE);
         /* Training timer reset */
        portTimer->trainingCount = 0;
        mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
    }
}

/*******************************************************************************
* mvPortCtrlApPortActiveExecTrainingEndWaitAck
*
*******************************************************************************/
void mvPortCtrlApPortActiveExecTrainingEndWaitAck(GT_U32 portIndex,
                                                  MV_HWS_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    GT_U32                         msgType;
    MV_HWS_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MV_HWS_PORT_STANDARD           portMode;

    portMode = (MV_HWS_PORT_STANDARD)recvMsg->portMode;

    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

    /* Check System Config */
    if ((portTimer->sysCfState == PORT_SM_SERDES_SYSTEM_DISABLE) ||
        ((portTimer->sysCfState  == PORT_SM_SERDES_SYSTEM_ENABLE) &&
         (portTimer->sysCfStatus == PORT_SM_SERDES_SYSTEM_VALID) &&
         ((portMode == portTimer->sysCfMode) || (portMode == _25GBase_KR_S && portTimer->sysCfMode ==_25GBase_KR) || (portMode == _25GBase_CR_S && portTimer->sysCfMode ==_25GBase_CR))))
    {
        portSm->status = PORT_SM_SERDES_TRAINING_END_SYS_ACK_RCV;
        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

        /* Ready to get traffic */
        if (mvHwsPortEnableSet(0, 0/*portGroup*/, recvMsg->phyPortNum, portMode, GT_TRUE) != GT_OK)
        {
            mvPortCtrlApPortFail("Error, Port Mng, ExecTrainingEndWaitAck - Port-%d enable Failed\n",
                                 portIndex, &msgType, recvMsg, PORT_SM_PORT_ENABLE_FAIL);
        }
        else
        {
#if BC2_DEV_SUPPORT
            /* Done - Release message */
            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
#else
            /*Done for those sppeds*/
            if ((portMode == _1000Base_X) || (portMode == _10GBase_KX4))
            {
                mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                return;
            }

            if (mvPortCtrlApPortCtleOverride(portIndex,recvMsg)!= GT_OK)
            {
                 mvPortCtrlApPortFail("Error, Port-%d ctle config Failed\n",
                                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_CTLE_ETL_CONFIG_FAIL);
                 return;
            }
#if defined(BOBK_DEV_SUPPORT)

            /* Start continuous Adaptive RX training after finish the EnhanceTuneLite */
            if (mvHwsPortAutoTuneSet(0, /* devNum */
                                     0, /* portGroup */
                                     (GT_U32)recvMsg->phyPortNum,
                                     portMode,
                                     RxTrainingAdative,
                                     NULL) != GT_OK)
            {
                mvPortCtrlApPortFail("Error, Port-%d Adaptive training start Failed\n",
                                     portIndex, &msgType, recvMsg, PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL);

            }
            else
            {
                /* Release message */
                mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
            }
#else
            if (mvPortCtrlApPortEtlParam(portIndex,recvMsg)!= GT_OK)
            {
                 mvPortCtrlApPortFail("Error, Port-%d ctle config Failed\n",
                                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_CTLE_ETL_CONFIG_FAIL);
                 return;
            }

            if(portSm->apPortEtlParam.etlMode == GT_TRUE)
            {
                portSm->status = PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS;
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                /* Use Training timer variables for ETL as well - reset it */
                portTimer->trainingCount = 0;
                mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
                /*reset ETL DB*/
                mvHwsPortEnhanceTuneLiteByPhaseInitDb(0,MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET((GT_U32)recvMsg->phyPortNum, MV_PORT_CTRL_AP_PORT_NUM_BASE));

            }
            else
            {
                /* Start continuous Adaptive RX training after finish the EnhanceTuneLite */
                if (mvHwsPortAutoTuneSet(0, /* devNum */
                                         0, /* portGroup */
                                         (GT_U32)recvMsg->phyPortNum,
                                         portMode,
                                         RxTrainingAdative,
                                         NULL) != GT_OK)
                {
                    mvPortCtrlApPortFail("Error, Port-%d Adaptive training start Failed\n",
                                         portIndex, &msgType, recvMsg, PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL);
                }
                else
                {
                    /* Release message */
                    mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                }
            }
#endif /* #if BOBK_DEV_SUPPORT */
#endif /* #else */

        }
    }
    else
    {
        /* Check system config threshold */
        if (mvPortCtrlThresholdCheck((GT_U32)portTimer->sysCfgThreshold))
        {
            mvPortCtrlApPortFail("Error, Port Mng, ExecTrainingEndWaitAck, Port-%d TRX training Failed\n",
                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_FAILURE);

            /* System config timer reset */
            mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->sysCfgThreshold));
        }
    }
}

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/*******************************************************************************
* mvPortCtrlApPortActiveExecTrainingEndWaitAck
*
*******************************************************************************/
void mvPortCtrlApPortActiveExecETLInProgress(GT_U32 portIndex,
                                             MV_HWS_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    GT_U32                                  msgType;
    MV_HWS_PORT_CTRL_PORT_SM                *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER          *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MV_HWS_PORT_STANDARD                    portMode;
    MV_HWS_AUTO_TUNE_STATUS                 rxTune;
    MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS   *trainPhase;

    /* Check training threshold */
    if (!(mvPortCtrlThresholdCheck(portTimer->trainingThreshold)))
    {
        return;
    }

    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

    portMode = (MV_HWS_PORT_STANDARD)recvMsg->portMode;

    if (portTimer->trainingCount >= PORT_CTRL_ETL_INTERVAL)
    {
        mvPortCtrlApPortFail("Error, Port Mng mvPortCtrlApPortActiveExecETLInProgress, Port-%d ETL time expired\n",
                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL);
        /* ETL timer reset */
        mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
        portTimer->trainingCount = 0;
        return;
    }
    else
    {
        portTimer->trainingCount++;

        trainPhase = &(hwsDeviceSpecInfo[0].portEnhanceTuneLiteDbPtr
                       [MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(recvMsg->phyPortNum, MV_PORT_CTRL_AP_PORT_NUM_BASE)].phaseEnhanceTuneLiteParams);

        /*  get the PCAL RX-Training status. the PCAL RX-Training runs only in EnhanceTuneLite phase-1 */
        if (trainPhase->phase == 1)
        {
            if (mvHwsPortAutoTuneStateCheck(0, /* devNum */
                                            0, /* portGroup */
                                            (GT_U32)recvMsg->phyPortNum,
                                            portMode,
                                            &rxTune,
                                            NULL /* TRX-Training PASS */) != GT_OK)
            {
                mvPortCtrlApPortFail("Error, Port-%d mvHwsPortAutoTuneStateCheck Failed\n",
                                     portIndex, &msgType, recvMsg, PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL);

                /* ETL timer reset */
                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                portTimer->trainingCount = 0;
                return;
            }

            /*
                if PCAL RX-Training is not completed and timing of trainingInterval still not expired
                --> Restart the threshold of ETL timer and continue
            */
            if ((rxTune == TUNE_NOT_COMPLITED) && (portTimer->trainingCount < PORT_CTRL_ETL_INTERVAL))
            {
                mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
                return;
            }

            /*
                In case of PCAL RX-Training FAIL or trainingInterval timer is expired and Training is not completed
                --> Delet the port and Reset the threshold of Training timer
            */
            if (rxTune == TUNE_NOT_COMPLITED)
            {
                mvPortCtrlApPortFail("Error, Port-%d EnhanceTuneLite phase-1: PCAL TUNE_FAIL\n",
                                     portIndex, &msgType, recvMsg, PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL);

                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                portTimer->trainingCount = 0;
                return;
            }
        }

        /* here, after running the EnhanceTuneLite phase-1 PCAL RX-Training PASS */

       if (trainPhase->phase == 0)
       {   /* Initialize the EnhanceTubeLite state before execute */
           trainPhase->phase = 1;
       }
       else if (trainPhase->phaseFinished == GT_TRUE)
       {
            /*
                For the first phase, run mvHwsAvagoSerdesEnhanceTuneLitePhase1.
                In second phase, if all PCAL RX-Trainings PASS, run the mvHwsAvagoSerdesEnhanceTuneLitePhase2.
            */
           trainPhase->phase++;
       }

       /* run pCAL with shifted sampling point to find best sampling point */
       if (mvHwsPortEnhanceTuneLiteSetByPhase(0, /* devNum */
                                              0, /* portGroup */
                                              (GT_U32)recvMsg->phyPortNum,
                                              portMode,
                                              trainPhase->phase,
                                              &(trainPhase->phaseFinished),
                                              portSm->apPortEtlParam.etlMinDelay,
                                              portSm->apPortEtlParam.etlMaxDelay) != GT_OK)
       {
           mvPortCtrlApPortFail("Error, Port-%d mvHwsPortEnhanceTuneLiteSetByPhase Failed\n",
                                portIndex, &msgType, recvMsg, PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL);

           /* Training timer reset */
           mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
           portTimer->trainingCount = 0;
           return;
       }

       if (trainPhase->phase == 2 && trainPhase->phaseFinished == GT_TRUE)
       {
           portSm->status = PORT_SM_SERDES_ENHANCE_TUNE_LITE_SUCCESS;
           mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

           /* Training timer reset */
           portTimer->trainingCount = 0;
           mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
           /* Start continuous Adaptive RX training after finish the EnhanceTuneLite */
           if (mvHwsPortAutoTuneSet(0, /* devNum */
                                    0, /* portGroup */
                                    (GT_U32)recvMsg->phyPortNum,
                                    portMode,
                                    RxTrainingAdative,
                                    NULL) != GT_OK)
           {
               mvPortCtrlApPortFail("Error, Port-%d Adaptive training start Failed\n",
                                    portIndex, &msgType, recvMsg, PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL);

               /* Training timer reset */
               mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
               portTimer->trainingCount = 0;

           }
           else /*Done*/
           {
               /* Training timer reset */
               mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
               portTimer->trainingCount = 0;
               /* Release message */
               mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
           }
       }
       else
       {
           /*
             after executing the mvHwsAvagoSerdesEnhanceTuneLitePhase1, return for checking the PCAL Rx-Training status.
             If the RX-Training is PASS, return to execute the mvHwsAvagoSerdesEnhanceTuneLitePhase2.
           */
           mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
       }
    }
}
#endif

/*******************************************************************************
* mvPortCtrlApPortActiveExec
*
*******************************************************************************/
void mvPortCtrlApPortActiveExec(GT_U32 portIndex)
{
    GT_U32                        msgType;
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT *recvMsg = NULL;
    MV_HWS_PORT_CTRL_PORT_SM      *portSm  = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_AP_SM_INFO  *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);

    if ((portSm->state == PORT_SM_SERDES_TRAINING_STATE) ||
        (portSm->state == PORT_SM_ACTIVE_STATE &&
         (portSm->status == PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK || portSm->status == PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS)))
    {
        /* Check if there are active messages for execution */
        if (mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PROC) != GT_OK)
        {
            /* No messages to process */
            return;
        }

        if (AP_PORT_DELETE_IN_PROGRESS == apSm->status)
        {
            AP_PRINT_MAC(("mvPortCtrlApPortActiveExec portIndex:%d deleted\n",portIndex));

            if (PORT_SM_SERDES_TRAINING_IN_PROGRESS == portSm->status)
            {
                    /* Execute HWS Training check */
                    mvHwsPortAutoTuneSet(0, /* devNum */
                                         0, /* portGroup */
                                         (GT_U32)recvMsg->phyPortNum,
                                         (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                         TRxTuneStop,
                                         NULL);
                    portSm->status = PORT_SM_SERDES_TRAINING_FAILURE;
                    mvPortCtrlLogAdd(PORT_SM_LOG(portSm->state, portSm->status, portIndex));
                    /* let training finish */
                    hwsOsTimerWkFuncPtr(20);
            }

            /* Clear training timer */
            portTimer->trainingCount = 0;
            /* Training timer reset */
            mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));

            /* Release message */
            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);

            return;
        }

        switch (portSm->status)
        {
            case PORT_SM_SERDES_TRAINING_IN_PROGRESS:
                mvPortCtrlApPortActiveExecTrainingInProgress(portIndex, recvMsg);
                break;
            case PORT_SM_SERDES_TRAINING_NOT_READY:
                mvPortCtrlApPortActiveExecTrainingNotReady(portIndex, recvMsg);
                break;
            case PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK:
                mvPortCtrlApPortActiveExecTrainingEndWaitAck(portIndex, recvMsg);
                break;
#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
           case PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS:
                mvPortCtrlApPortActiveExecETLInProgress(portIndex, recvMsg);
                break;
#endif
            default:
                return;

        }
    }
    else
    {
        return;
    }
}

/**
* @internal mvPortCtrlApPortActive function
* @endinternal
*
* @brief   AP Port Active state execution
*         State Description
*         - loop over all AP Port currently executed
*         - Trigger state change
*/
void mvPortCtrlApPortActive(void)
{
    GT_U32 port;
    GT_U8 maxApPortNum = 0;

#ifdef BC2_DEV_SUPPORT
    if (BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_BC2_MAX_AP_PORT_NUM;
    }
    else if (Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
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

    infoPtr->state = AP_PORT_MNG_ACTIVE_PORT_STATE;

    for (port = 0; port < maxApPortNum; port++)
    {
        mvPortCtrlApPortActiveExec(port);
    }
    infoPtr->event = AP_PORT_MNG_DELAY_EVENT;
}

/**
* @internal mvPortCtrlApPortFail function
* @endinternal
*
* @brief   Execute fail sequence for different failure cases in mvPortCtrlApPortPendExec
*
* @param[in] errorMsg                 - string describing failure
* @param[in] portIndex                - index of port in local AP DB
* @param[in] msgType                  - type of message - actually used just to pass to mvPortCtrlApPortPendMsg
* @param[in] recvMsg                  - message treated now by task
* @param[in] status                   - new  of port treatment
*                                       None.
*/
static void mvPortCtrlApPortFail
(
    char                           *errorMsg,
    GT_U32                          portIndex,
    GT_U32                         *msgType,
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT  *recvMsg,
    GT_U8                           status
)
{
    MV_HWS_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_PORT_CTRL_AP_INIT       *portInit  = &(mvHwsPortCtrlApPortMng.apInitStatus);

    mvPcPrintf(errorMsg, recvMsg->phyPortNum);
    AP_PRINT_MAC(("mvPortCtrlApPortFail: port:%d %s\n",portIndex,errorMsg));

    portSm->status = status;
    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
    mvPortCtrlApPortResetExec((GT_U32)recvMsg->phyPortNum, portSm, recvMsg);

    /* Mark port init failure */
    *portInit |= MV_PORT_CTRL_NUM(portIndex);

    /* Release message */
    mvPortCtrlApPortPendMsg(portIndex, &recvMsg, msgType, PORT_MSG_FREE);

    return;
}

/**
* @internal mvPortCtrlApPortFcSet function
* @endinternal
*
* @brief   Execute Flow Control relosultion configuration in mvPortCtrlApPortPendExec
*
* @param[in] portIndex                - index of port in local AP DB
* @param[in] msgType                  - type of message - actually used just to pass to mvPortCtrlApPortPendMsg
* @param[in] recvMsg                  - message treated now by task
*                                      status - new status of port treatment
*                                       None.
*/
static void mvPortCtrlApPortFcSet
(
    GT_U32                          portIndex,
    MV_HWS_PORT_STANDARD            portMode,
    GT_U32                         *msgType,
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT  *recvMsg
)
{
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState;
    GT_U32                          fcRxPauseEn, fcTxPauseEn;
    MV_HWS_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_AP_SM_INFO              *apSm      = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, PORT_SM_FC_STATE_SET_IN_PROGRESS, recvMsg->phyPortNum));
    fcRxPauseEn = AP_ST_HCD_FC_RX_RES_GET(apSm->hcdStatus);
    fcTxPauseEn = AP_ST_HCD_FC_TX_RES_GET(apSm->hcdStatus);
    if (fcRxPauseEn && fcTxPauseEn)
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_RX_TX_E;
    }
    else if (fcRxPauseEn)
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_RX_ONLY_E;
    }
    else if (fcTxPauseEn)
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_TX_ONLY_E;
    }
    else
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_DISABLE_E;
    }

    if (mvHwsPortFlowControlStateSet(0, /* devNum */
                                     0, /* portGroup */
                                     (GT_U32)recvMsg->phyPortNum,
                                     portMode,
                                     fcState) != GT_OK)
    {
        mvPortCtrlApPortFail("Error, Port Mng, Port-%d FC state set Failed\n",
                             portIndex, msgType, recvMsg, PORT_SM_FC_STATE_SET_FAILURE);
        return;
    }

    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, PORT_SM_FC_STATE_SET_SUCCESS, recvMsg->phyPortNum));

    return;
}

/**
* @internal mvPortCtrlApPortPendExec function
* @endinternal
*
* @brief   AP Port message state execution
*         State Description
*         - Check if there are new pending messages for execution
*         In case no message is pending, exit
*         - Message is pending, and Message type is RESET
*         Execute port reset sequence
*         - Message is pending, and Message type is INIT
*         Port state == Idle
*         Extract pending message for execution
*         Execute port Init
*         Execute SERDES Configuration
*         Execute MAC/PCS Configuration
*         Execute training
*         Training configuration
*         Training start
*         Training timeout timer
*         Note: current execution use current port init API which execute
*         SERDES + MAC/PCS Configuration. once available it will be seperated
*         to two API's and MAC/PCS Configuration will be executed after training ends
*/
void mvPortCtrlApPortPendExec(GT_U32 portIndex)
{
    GT_U32                         msgType;
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT  *recvMsg   = NULL;
    MV_HWS_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MV_HWS_PORT_CTRL_AP_INIT       *portInit  = &(mvHwsPortCtrlApPortMng.apInitStatus);
    MV_HWS_AP_SM_INFO              *apSm      = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
#ifndef BV_DEV_SUPPORT
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
#endif
    MV_HWS_PORT_STANDARD            portMode;
    MV_HWS_AUTO_TUNE_STATUS_RES     tuneRes;
    GT_STATUS                       rc        = GT_OK;
    MV_HWS_PORT_FEC_MODE            portFecType;
    MV_HWS_PORT_PCS_TYPE            pcsType;
    GT_U32                          curLanesList[HWS_MAX_SERDES_NUM];
    GT_U8                           trainingStatus;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    /* Check if there are new pending messages for execution */
    if (mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PEEK) != GT_OK)
    {
        /* No messages to process */
        return;
    }

    /* Handle reset message in any port execution state */
    if ((msgType == MV_HWS_IPC_PORT_RESET_MSG) ||
        (msgType == MV_HWS_IPC_PORT_RESET_EXT_MSG))
    {
        AP_PRINT_MAC(("mvPortCtrlApPortPendExec portIndex:%d  rcv IPC_PORT_RESET port_state %d port_status %d ap status %d ap_state %d\n",portIndex, portSm->state,portSm->status, apSm->status, apSm->state));
        if (mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PROC) == GT_OK)
        {
            mvPortCtrlApPortResetExec((GT_U32)recvMsg->phyPortNum, portSm, recvMsg);
            if(portSm->status != PORT_SM_DELETE_FAILURE)
            {
                portSm->state = PORT_SM_IDLE_STATE;
                portSm->status = PORT_SM_DELETE_SUCCESS;
                mvPortCtrlLogAdd(PORT_SM_LOG(portSm->state, portSm->status, portIndex));
            }

            /* Clear port init failure */
            *portInit &= ~(MV_PORT_CTRL_NUM(portIndex));

            /* Release message */
            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
            return;
        }
    }
    /* Handle init nessage in IDLE state, any other state Ignore */
    else if (msgType == MV_HWS_IPC_PORT_INIT_MSG)
    {
        if (portSm->state == PORT_SM_IDLE_STATE)
        {
            if(AP_PORT_DELETE_IN_PROGRESS == apSm->status)
            {
                AP_PRINT_MAC(("mvPortCtrlApPortPendExec portIndex:%d  rcv IPC_PORT_INIT delete msg\n",portIndex));
                portSm->status = PORT_SM_DELETE_IN_PROGRESS;
                mvPortCtrlLogAdd(PORT_SM_LOG(portSm->state, portSm->status, portIndex));
                /* Clear port init failure */
                *portInit &= ~(MV_PORT_CTRL_NUM(portIndex));

                /* Release message */
                mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                return;
            }
            if (mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PROC) == GT_OK)
            {
                /* Clear port init failure */
                *portInit &= ~(MV_PORT_CTRL_NUM(portIndex));
                /* Clear training counter before init */
                portTimer->trainingCount = 0;
                portSm->status = PORT_SM_START_EXECUTE;
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                /* Execute HWS Port Init */
                portMode = (MV_HWS_PORT_STANDARD)recvMsg->portMode;

                if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, (GT_U32)recvMsg->phyPortNum, portMode, &curPortParams))
                {
                    mvPortCtrlApPortFail("Error, Port Mng, Port-%d hwsPortModeParamsGet return NULL\n",
                                         portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                    return;
                }
                pcsType = curPortParams.portPcsType;
                portFecType = (AP_ST_HCD_FEC_RES_NONE == AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus))
                    ? FEC_OFF : FC_FEC;
#ifdef BC2_DEV_SUPPORT
                if (Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
                {
                    portSm->state  = PORT_SM_SERDES_CONFIG_STATE;
                    portSm->status = PORT_SM_SERDES_CONFIG_IN_PROGRESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    portSm->state  = PORT_SM_MAC_PCS_CONFIG_STATE;
                    portSm->status = PORT_SM_MAC_PCS_CONFIG_IN_PROGRESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                    osMemSet(&portInitInParam, 0,sizeof(portInitInParam));
                    portInitInParam.lbPort = (GT_BOOL)recvMsg->action;
                    portInitInParam.refClock = (MV_HWS_REF_CLOCK_SUP_VAL)recvMsg->refClock;
                    portInitInParam.refClockSource = (MV_HWS_REF_CLOCK_SOURCE)recvMsg->refClockSource;

                    rc = mvHwsPortInit(0, /* devNum */
                                       0, /* portGroup */
                                       (GT_U32)recvMsg->phyPortNum,
                                       portMode,
                                       &portInitInParam);

                    if (rc != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d Init Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                        return;
                    }

                    portSm->status = PORT_SM_SERDES_CONFIG_SUCCESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    portSm->status = PORT_SM_MAC_PCS_CONFIG_SUCCESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    portSm->status = PORT_SM_SERDES_POLARITY_SET_IN_PROGRESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                    rc = mvApPolarityCfg(apSm->portNum, portMode, apSm->polarityVector);
                    if (rc != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d Set polarity Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_POLARITY_SET_FAILURE);
                        return;
                    }

                    portSm->status = PORT_SM_SERDES_POLARITY_SET_SUCCESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    if ((portMode != _1000Base_X) && (portMode != _10GBase_KX4))
                    {
                        portSm->state  = PORT_SM_SERDES_TRAINING_STATE;
                        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                        portSm->status = PORT_SM_SERDES_TRAINING_IN_PROGRESS;
                        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                        AP_PRINT_MAC(("mvPortCtrlApPortPendExec:call mvHwsPortAutoTuneSet  port:%d \n",portIndex));
                        /* Execute HWS Training config */
                        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                                 0, /* portGroup */
                                                 (GT_U32)recvMsg->phyPortNum,
                                                 portMode,
                                                 TRxTuneCfg,
                                                 0) != GT_OK)
                        {
                            mvPortCtrlApPortFail("Error, Port Mng CtrlApPortPendExec, Port-%d training config Failed\n",
                                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_CONFIG_FAILURE);
                            return;
                        }

                        tuneRes.txTune = TUNE_NOT_COMPLITED;

                        /* Execute HWS Training start */
                        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                                 0, /* portGroup */
                                                 (GT_U32)recvMsg->phyPortNum,
                                                 portMode,
                                                 TRxTuneStart,
                                                 &tuneRes) != GT_OK)
                        {
                            tuneRes.txTune = TUNE_FAIL;
                        }

                        if (tuneRes.txTune == TUNE_NOT_READY)
                        {
                            portSm->status = PORT_SM_SERDES_TRAINING_NOT_READY;
                            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                            /* Training timer start to prevent infinite loop in serdes not ready state */
                            mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
                        }
                        else if ((tuneRes.txTune == TUNE_PASS) || (tuneRes.txTune == TUNE_READY))
                        {
                            /* Training timer start */
                            mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
                        }
                        else
                        {
                            mvPortCtrlApPortFail("Error, Port Mng, Port-%d training starting Failed\n",
                                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_FAILURE);
                            return;
                        }
                    }
                    else
                    {
                        mvPortCtrlApPortFcSet(portIndex, portMode, &msgType, recvMsg);
                        portSm->state  = PORT_SM_ACTIVE_STATE;
                        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                        mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                    }
                }
                else
                {
#endif /* BC2_DEV_SUPPORT */
                    portSm->state  = PORT_SM_SERDES_CONFIG_STATE;
                    portSm->status = PORT_SM_SERDES_CONFIG_IN_PROGRESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    /* rebuild active lanes list according to current configuration (redundancy) */
                    if (mvHwsRebuildActiveLaneList(0, 0, (GT_U32)recvMsg->phyPortNum, portMode, curLanesList) != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d mvHwsRebuildActiveLaneList Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_CONFIG_FAILURE);
                        return;
                    }

                    if (mvPortCtrlApCallbacks.apHcdFoundClbk != NULL)  {
                        mvPortCtrlApCallbacks.apHcdFoundClbk(0, (GT_U32)recvMsg->phyPortNum, portMode);
                    }

                    /* SERDES Tx Disable - disable signal on lane where managed negotiatio */
                    mvHwsSerdesTxEnable(0, 0, AP_CTRL_LANE_GET(apSm->ifNum), HWS_DEV_SERDES_TYPE(0/*devNum*/), GT_FALSE);
                    osMemSet(&portInitInParam, 0,sizeof(portInitInParam));
                    portInitInParam.lbPort = (GT_BOOL)recvMsg->action;
                    portInitInParam.refClock = (MV_HWS_REF_CLOCK_SUP_VAL)recvMsg->refClock;
                    portInitInParam.refClockSource = (MV_HWS_REF_CLOCK_SOURCE)recvMsg->refClockSource;
                    rc = mvHwsApPortInit(0, /* devNum */
                                         0, /* portGroup */
                                         (GT_U32)recvMsg->phyPortNum,
                                         portMode,
                                         &portInitInParam);

                    if (rc != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d Init Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                        return;
                    }

                    portSm->status = PORT_SM_SERDES_CONFIG_SUCCESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    portSm->status = PORT_SM_SERDES_POLARITY_SET_IN_PROGRESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                    rc = mvApPolarityCfg((GT_U32)recvMsg->phyPortNum, portMode, apSm->polarityVector);
                    if (rc != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d Set polarity Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_SERDES_POLARITY_SET_FAILURE);
                        return;
                    }
                    portSm->status = PORT_SM_SERDES_POLARITY_SET_SUCCESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    /* Enable the Tx signal, the signal was disabled during Serdes init */
                    if (mvHwsPortTxEnable(0, 0, (GT_U32)recvMsg->phyPortNum, portMode, GT_TRUE) != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d mvHwsPortTxEnable Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                        return;
                    }

                    if ((portMode != _1000Base_X) && (portMode != _10GBase_KX4))
                    {
                        portSm->state  = PORT_SM_SERDES_TRAINING_STATE;
                        portSm->status = PORT_SM_SERDES_TRAINING_IN_PROGRESS;
                        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                        AP_PRINT_MAC(("mvPortCtrlApPortPendExec:call mvHwsPortAutoTuneSet  port:%d \n",portIndex));
                        /* Execute HWS Training config */
                        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                                 0, /* portGroup */
                                                 (GT_U32)recvMsg->phyPortNum,
                                                 portMode,
                                                 TRxTuneCfg,
                                                 0) != GT_OK)
                        {
                            mvPortCtrlApPortFail("Error, Port Mng CtrlApPortPendExec, Port-%d training config Failed\n",
                                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_CONFIG_FAILURE);
                            return;
                        }

                        tuneRes.txTune = TUNE_NOT_COMPLITED;

                        /* Execute HWS Training start */
                        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                                 0, /* portGroup */
                                                 (GT_U32)recvMsg->phyPortNum,
                                                 portMode,
                                                 TRxTuneStart,
                                                 &tuneRes) != GT_OK)
                        {
                            tuneRes.txTune = TUNE_FAIL;
                        }

                        mvPortCtrlDbgO_CoreStatusDump(portIndex, portSm->state);

                        if (tuneRes.txTune == TUNE_NOT_READY)
                        {
                            portSm->status = PORT_SM_SERDES_TRAINING_NOT_READY;
                            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                            /* Training timer start to prevent infinite loop in serdes not ready state */
                            mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
                        }
                        else if ((tuneRes.txTune == TUNE_PASS) || (tuneRes.txTune == TUNE_READY))
                        {
                            /* Training timer start */
                            mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
                        }
                        else
                        {
                            mvPortCtrlApPortFail("Error, Port Mng, Port-%d training starting Failed\n",
                                                 portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_FAILURE);
                            return;
                        }

                        if (_100GBase_KR4 == portMode || _100GBase_CR4 == portMode)
                        { /* for 100G RS FEC is mandatory not subject of negotiation */
                            curPortParams.portFecMode = portFecType = RS_FEC;
                            hwsPortModeParamsSetFec(0,0,(GT_U32)recvMsg->phyPortNum, portMode, portFecType); /* update the elements database since curPortParams is only a buffer*/
                            pcsType = CGPCS;
                        }
                        else
                        {
                            switch (AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus))
                            {
                                case AP_ST_HCD_FEC_RES_NONE:
                                    portFecType = FEC_OFF;
                                    pcsType = ((_50GBase_KR2_C == portMode) || (_50GBase_CR2_C == portMode)) ? CGPCS : MMPCS;
                                    break;
                                case AP_ST_HCD_FEC_RES_FC:
                                    portFecType = FC_FEC;
                                    pcsType = MMPCS;
                                    break;
                                case AP_ST_HCD_FEC_RES_RS:
                                    portFecType = RS_FEC;
                                    pcsType = CGPCS;
                                    break;
                                default:
                                    mvPortCtrlApPortFail("Error, Port Mng, Port-%d not supported FEC resolution\n",
                                                         portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                                    return;
                            }

                            if (((_25GBase_KR == portMode) || (_25GBase_KR_C == portMode) || (_50GBase_KR2_C == portMode)
                                 || (_25GBase_CR_C == portMode) || (_50GBase_CR2_C == portMode) || (_25GBase_CR == portMode)) && (curPortParams.portFecMode != portFecType))
                            {
                                curPortParams.portFecMode = portFecType;
                                hwsPortModeParamsSetFec(0,0,(GT_U32)recvMsg->phyPortNum, portMode, portFecType); /* update the elements database since curPortParams is only a buffer */
                            }
                        }
                    }

                    trainingStatus = portSm->status;
                    portSm->state  = PORT_SM_MAC_PCS_CONFIG_STATE;
                    portSm->status = PORT_SM_MAC_PCS_CONFIG_IN_PROGRESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    /* Configure MAC/PCS */
                    if (mvHwsPortModeCfg(0, 0, recvMsg->phyPortNum, portMode) != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d mvHwsPortModeCfg Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                        return;
                    }

                    /* Un-Reset the port */
                    if (mvHwsPortStartCfg(0, 0, recvMsg->phyPortNum, portMode) != GT_OK)
                    {
                        mvPortCtrlApPortFail("Error, Port Mng, Port-%d mvHwsPortStartCfg Failed\n",
                                             portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                        return;
                    }

                    portSm->status = PORT_SM_MAC_PCS_CONFIG_SUCCESS;
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    mvPortCtrlApPortFcSet(portIndex, portMode, &msgType, recvMsg);

                    if ((portMode != _1000Base_X) && (portMode != _10GBase_KX4))
                    {
                        if (((portMode != _25GBase_KR) && (portMode != _25GBase_KR_C) && (portMode != _50GBase_KR2_C)
                             && (portMode != _25GBase_CR_C) && (portMode != _50GBase_CR2_C) && (portMode != _25GBase_CR)) ||
                            (portFecType != RS_FEC))
                        { /* this call not relevant for 25G RS FEC - it already configured during port init */
                            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, PORT_SM_FEC_CONFIG_IN_PROGRESS, recvMsg->phyPortNum));
                            if (mvHwsPcsFecConfig(0, /* devNum */
                                                  0, /* portGroup */
                                                  curPortParams.portPcsNumber,
                                                  pcsType,
                                                  portFecType) != GT_OK)
                            {
                                mvPortCtrlApPortFail("Error, Port Mng, Port-%d FEC configuration Failed\n",
                                                     portIndex, &msgType, recvMsg, PORT_SM_FEC_CONFIG_FAILURE);
                                return;
                            }
                            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, PORT_SM_FEC_CONFIG_SUCCESS, recvMsg->phyPortNum));
                        }
                        /* restore state and status to TRAINIG for right state machine */
                        AP_PRINT_MAC(("FC and fec config done restore to training state port:%d \n",portIndex));
                        portSm->state  = PORT_SM_SERDES_TRAINING_STATE;
                        portSm->status = trainingStatus;
                        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                    }
                    else
                    {
                        portSm->state  = PORT_SM_ACTIVE_STATE;
                        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                        /* Check System Config */
                        if (mvHwsApPortEnableCtrl)
                        {
                            portSm->status = PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK;
                            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                            if ((portTimer->sysCfState == PORT_SM_SERDES_SYSTEM_DISABLE) ||
                                ((portTimer->sysCfState  == PORT_SM_SERDES_SYSTEM_ENABLE) &&
                                 (portTimer->sysCfStatus == PORT_SM_SERDES_SYSTEM_VALID) &&
                                 (portMode == portTimer->sysCfMode)))
                            {
                                portSm->status = PORT_SM_SERDES_TRAINING_END_SYS_ACK_RCV;
                                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                                /* Ready to get traffic */
                                rc = mvHwsPortEnableSet(0, 0/*portGroup*/, recvMsg->phyPortNum, portMode, GT_TRUE);
                                if (rc != GT_OK)
                                {
                                    mvPortCtrlApPortFail("Error, Port Mng, Port-%d enable Failed\n",
                                                         portIndex, &msgType, recvMsg, PORT_SM_PORT_ENABLE_FAIL);
                                }
                                /* Release message */
                                mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                            }
                            else
                            {
                                /* System config timer start */
                                mvPortCtrlThresholdSet(PORT_CTRL_SYSTEM_CONFIG_DURATION_1000BASEX_10GBase_KX4, &(portTimer->sysCfgThreshold));
                            }
                        }
                        else
                        {
                            /* Release message */
                            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                        } /*if(mvHwsApPortEnableCtrl)*/
                    } /*if ((portMode != _1000Base_X) && (portMode != _10GBase_KX4))*/
#ifdef BC2_DEV_SUPPORT
                }
#endif /* BC2_DEV_SUPPORT */
            } /*(mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PROC) == GT_OK)*/
        } /*if (portSm->state == PORT_SM_IDLE_STATE)*/
    } /*else if (msgType == MV_HWS_IPC_PORT_INIT_MSG)*/
}

/**
* @internal mvPortCtrlApPortMsg function
* @endinternal
*
* @brief   AP Port message state execution
*         State Description
*         - loop over all AP Port pending queues, and trigger queue execution
*         - Trigger state change
*/
void mvPortCtrlApPortMsg(void)
{
    GT_U32 port;
    GT_U8 maxApPortNum = 0;

#ifdef  BC2_DEV_SUPPORT
    if (BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_BC2_MAX_AP_PORT_NUM;
    }
    else if (Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
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

    infoPtr->state = AP_PORT_MNG_MSG_PROCESS_STATE;

    for (port = 0; port < maxApPortNum; port++)
    {
        mvPortCtrlApPortPendExec(port);
    }

    infoPtr->event = AP_PORT_MNG_ACTIVE_PORT_EVENT;
}

/**
* @internal mvPortCtrlApPortDelay function
* @endinternal
*
* @brief   AP Port delay state execution
*         State Description
*         - Execute delay
*         - Trigger state change
*/
void mvPortCtrlApPortDelay(void)
{
    infoPtr->state = AP_PORT_MNG_PORT_DELAY_STATE;
    mvPortCtrlProcessDelay(mvHwsPortCtrlApPortMng.info.delayDuration);
    infoPtr->event = AP_PORT_MNG_MSG_PROCESS_EVENT;
}

/**
* @internal mvPortCtrlApPortPendAdd function
* @endinternal
*
* @brief   The functions add an IPC message to the port pending tasks
*         message queue
*         It validate if there is free space in the queue, and in case
*         valid it adds the message to the the queue
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlApPortPendAdd(GT_U32 port, MV_HWS_IPC_CTRL_MSG_AP_STRUCT *msg)
{
    GT_U8 nextPendTaskAddIndex;
    MV_HWS_PORT_CTRL_PEND_MSG_TBL *portCtrlSmPendMsgPtr;

    portCtrlSmPendMsgPtr = &(mvHwsPortCtrlApPortMng.tasks[port]);

    if (((portCtrlSmPendMsgPtr->pendTasksAddIndex + 1) != portCtrlSmPendMsgPtr->pendTasksProcIndex) &&
        (portCtrlSmPendMsgPtr->pendTasksCount < MV_PORT_CTRL_PORT_PEND_MSG_NUM))
    {
        nextPendTaskAddIndex = portCtrlSmPendMsgPtr->pendTasksAddIndex + 1;
        /* Roll over */
        if (nextPendTaskAddIndex >= MV_PORT_CTRL_PORT_PEND_MSG_NUM)
        {
            nextPendTaskAddIndex = 0;
        }
        mvPortCtrlSyncLock();
        osMemCpy(&(portCtrlSmPendMsgPtr->pendTasks[nextPendTaskAddIndex]), msg, sizeof(MV_HWS_IPC_CTRL_MSG_AP_STRUCT));
        portCtrlSmPendMsgPtr->pendTasksCount++;
        portCtrlSmPendMsgPtr->pendTasksAddIndex = nextPendTaskAddIndex;
        mvPortCtrlSyncUnlock();
    }
    else
    {
        mvPcPrintf("Error, AP Port Mng, Port Index-%d Add message failed!\n", port);
        return GT_ERROR;
    }

    return GT_OK;
}

/**
* @internal mvPortCtrlApPortPendMsg function
* @endinternal
*
* @brief   The functions return IPC message waiting in the port pending tasks
*         message queue for execution
*         The message is not release to the queue until it is executed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlApPortPendMsg(GT_U32 port, MV_HWS_IPC_CTRL_MSG_AP_STRUCT **msg,
                                  GT_U32 *msgType, GT_U32 action)
{
    GT_U8 nextPendTasksIndex;
    MV_HWS_PORT_CTRL_PEND_MSG_TBL *portCtrlSmPendMsgPtr;

    portCtrlSmPendMsgPtr = &(mvHwsPortCtrlApPortMng.tasks[port]);

    if ((portCtrlSmPendMsgPtr->pendTasksProcIndex != portCtrlSmPendMsgPtr->pendTasksAddIndex) ||
        ((portCtrlSmPendMsgPtr->pendTasksProcIndex == portCtrlSmPendMsgPtr->pendTasksAddIndex) &&
         (portCtrlSmPendMsgPtr->pendTasksCount == MV_PORT_CTRL_PORT_PEND_MSG_NUM)))
    {
        nextPendTasksIndex = portCtrlSmPendMsgPtr->pendTasksProcIndex + 1;
        /* Roll over */
        if (nextPendTasksIndex >= MV_PORT_CTRL_PORT_PEND_MSG_NUM)
        {
            nextPendTasksIndex = 0;
        }
        /* Return message type */
        if (action == PORT_MSG_PEEK)
        {
            *msgType = (GT_U32)portCtrlSmPendMsgPtr->pendTasks[nextPendTasksIndex].ctrlMsgType;
        }
        /* Return message pointer */
        else if (action == PORT_MSG_PROC)
        {
            *msg = &(portCtrlSmPendMsgPtr->pendTasks[nextPendTasksIndex]);
        }
        /* Release message */
        else if (action == PORT_MSG_FREE)
        {
            mvPortCtrlSyncLock();
            portCtrlSmPendMsgPtr->pendTasksProcIndex = nextPendTasksIndex;
            portCtrlSmPendMsgPtr->pendTasksCount--;
            mvPortCtrlSyncUnlock();
        }

        return GT_OK;
    }

    return GT_ERROR;
}

/**
* @internal mvPortCtrlApPortMsgSend function
* @endinternal
*
* @brief   The functions build and send IPC message to AP Port init queue
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlApPortMsgSend(GT_U8 portIndex, GT_U8 msgType, GT_U8 queueId,
                                  GT_U8 portNum, GT_U8 portMode, GT_U8 action,
                                  GT_U8 refClk, GT_U8 refClkSrc)
{
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT apIpcMsg;

    apIpcMsg.ctrlMsgType    = msgType;
    apIpcMsg.msgQueueId     = queueId;
    apIpcMsg.phyPortNum     = portNum;
    apIpcMsg.portMode       = portMode;
    apIpcMsg.action         = action;
    apIpcMsg.refClock       = refClk;
    apIpcMsg.refClockSource = refClkSrc;

    return mvPortCtrlApPortPendAdd(portIndex, &apIpcMsg);

}



