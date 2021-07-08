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
* @file mcdFwPortCtrlApPort.c
*
* @brief Port Control AP Port State Machine
*
* @version   1
********************************************************************************
*/
#include <mcdFwPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>              /* from port */
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>              /* from port */
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>  /* from port */

static MCD_PORT_CTRL_COMMON *infoPtr;

GT_STATUS mvPortCtrlApPortPendMsg(GT_U32 port, MCD_IPC_CTRL_MSG_AP_STRUCT **msg,
                                  GT_U32 *msgType, GT_U32 action);
GT_STATUS mvPortCtrlApPortPendAdd(GT_U32 port, MCD_IPC_CTRL_MSG_AP_STRUCT *msg);

void mvPortCtrlPortActiveExecTraining(GT_U32 targetPort,
                                      MCD_PORT_CTRL_PORT_SM *portCtrlSmPtr);
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
extern MCD_PORT_CTRL_AP_CALLBACKS mvPortCtrlApCallbacks;
extern MV_HWS_PORT_STANDARD mvHwsApConvertPortMode(MV_HWA_AP_PORT_MODE apPortMode);

/* These extern requires for MCD_STAT_PORT_SM_STATUS_INSERT and MCD_STAT_PORT_SM_STATE_INSERT */
extern void  mcdStatPortSmStateInsert(GT_U8 portIndex, GT_U16 state);
extern void  mcdStatPortSmStatusInsert(GT_U8 portIndex, GT_U16 status);

static void mvPortCtrlApPortFail
(
    char                           *errorMsg,
    GT_U32                          portIndex,
    GT_U32                         *msgType,
    MCD_IPC_CTRL_MSG_AP_STRUCT  *recvMsg,
    GT_U8                           status
);

/**
* @internal mvPortCtrlApPortRoutine function
* @endinternal
*
* @brief   AP Port mng process execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvPortCtrlApPortRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvHwsPortCtrlApPortMng.info);

    for( ;; )
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

/** mvApPolarityCfg
*******************************************************************************/
GT_STATUS mvApPolarityCfg(GT_U8 portNum,  MV_HWS_PORT_STANDARD portMode, GT_U8 polarityVector)
{
    GT_STATUS status = GT_OK;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U8                           i;
    GT_BOOL                         polarityTx = GT_FALSE;
    GT_BOOL                         polarityRx = GT_FALSE;

    if(polarityVector)
    {
        if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, (GT_U32)portNum, portMode, &curPortParams))
        {
            return GT_BAD_PTR;
        }

        for(i = 0; i < curPortParams.numOfActLanes; i++)
        {
            polarityTx = polarityVector & (1 << (2*i+1));
            polarityRx = polarityVector & (1 << (2*i));
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(curPortParams.activeLanesList[i], 0, 19, 0));
            if (polarityTx || polarityRx)
            {
                status = mvHwsSerdesPolarityConfig(0, 0, curPortParams.activeLanesList[i], HWS_DEV_SERDES_TYPE(0), polarityTx, polarityRx);
                if (status != GT_OK)
                {
                    return status;
                }
            }
        }
    }
    return status;
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
GT_STATUS mvPortCtrlApPortResetExec(GT_U32 port,
                                    MCD_PORT_CTRL_PORT_SM *info,
                                    MCD_IPC_CTRL_MSG_AP_STRUCT *msg)

{
    MV_HWS_PORT_STANDARD portMode;
    MV_HWA_AP_PORT_MODE  apPortMode;
    MCD_AP_SM_INFO              *apSm      = &(mvHwsPortCtrlApPortDetect.apPortSm[port].info);

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
        ms_sleep(20);
    }

     if (mvPortCtrlApCallbacks.apLinkFallingClbk != NULL)
    {
        apPortMode = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);
        portMode   = mvHwsApConvertPortMode(apPortMode);
        AP_PRINT_MCD2_MAC(("mvApPortDeleteMsg port:%d  calling apLinkFallingClbk\n",port));
        mvPortCtrlApCallbacks.apLinkFallingClbk(0 /*devNum*/, (GT_U32)msg->phyPortNum, portMode);
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

    info->state = PORT_SM_IDLE_STATE;
    info->status = PORT_SM_DELETE_SUCCESS;
    mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));

    return GT_OK;
}

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
                                                  MCD_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    GT_U32                         msgType;
    MCD_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MCD_AP_SM_STATS             *apStats   = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    GT_U32                         tuneResult = TUNE_NOT_COMPLITED;
    MV_HWS_AUTO_TUNE_STATUS_RES    tuneRes;
    MV_HWS_PORT_INIT_PARAMS  curPortParams;

    GT_BOOL etlMode = GT_TRUE;
    MCD_U8   etlMinDelay = 0;/*bit[7]=0 means use default*/
    MCD_U8   etlMaxDelay = 0;/*bit[7]=0 means use default*/
    MV_HWS_AUTO_TUNE_STATUS     rxTune;
    MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS    *trainPhase;

    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA  ctleData[MCD_MAX_LANES_NUM_PER_PORT];
    MCD_U16                           fieldOverrideBmp, index, i;
    MCD_BOOL                         ctleOverride = MCD_FALSE;
    MCD_PORT_CTRL_PORT_SM            *singlePortSm;

    /* Check training threshold */
    if (!(mvPortCtrlThresholdCheck(portTimer->trainingThreshold)))
    {
        return;
    }

    portTimer->trainingCount++;

    /* Execute HWS Training check */
    AP_PRINT_MCD2_MAC(("mvPortCtrlApPortActiveExecTrainingInProgress portIndex:%d  \n",portIndex));
    if (portSm->status == PORT_SM_SERDES_TRAINING_IN_PROGRESS)
    {
        if (mvHwsPortAutoTuneSet(0, /* devNum */
                                 0, /* portGroup */
                                 (GT_U32)recvMsg->phyPortNum,
                                 (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                 TRxTuneStatusNonBlocking,
                                 &tuneRes) == GT_OK)
        /* Execute HWS Training check */
        {
            if (tuneRes.txTune == TUNE_PASS)
            {
                /* Training success */
                tuneResult = TUNE_PASS;
                AP_PRINT_MCD_MAC(("mvPortCtrlApPortActiveExecTrainingInProgress portIndex:%d  TUNE_PASS***\n",portIndex));
            }
            else if (tuneRes.txTune == TUNE_FAIL)
            {
                /* Training failure */
                tuneResult = TUNE_FAIL;
                AP_PRINT_MCD_MAC(("mvPortCtrlApPortActiveExecTrainingInProgress portIndex:%d  TUNE_FAIL***\n",portIndex));
            }
        }


        if ((tuneResult == TUNE_NOT_COMPLITED) &&
            (portTimer->trainingCount < portTimer->trainingMaxInterval))
        {
            /* Training timer restart */
            mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
            return;
        }

        /* Training timer expire and training not complited */
        if (tuneResult == TUNE_NOT_COMPLITED)
        {
            /* Training failure */
            tuneResult = TUNE_FAIL;
        }

        /* Training success / failure / threshold exceeded */
        /* =============================================== */
        if(tuneResult == TUNE_FAIL)
        {
            mvHwsPortAutoTuneSet(0, /* devNum */
                                 0, /* portGroup */
                                 (GT_U32)recvMsg->phyPortNum,
                                 (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                 TRxTuneStop,
                                 NULL);
        }

        /* Training failure / Training threshold exceeded */
        if (tuneResult == TUNE_FAIL)
        {
                    /* let training finish */
            ms_sleep(20);
            mvPortCtrlApPortFail("Error, Port-%d TRX training Failed\n",
                                    portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_FAILURE);
            /* Training timer reset */
            mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
            return;
        }
    /* Training success */
        else if (tuneResult == TUNE_PASS)
        {

            if (portSm->state != PORT_SM_ACTIVE_STATE)
            {
                portSm->state = PORT_SM_ACTIVE_STATE;
                MCD_STAT_PORT_SM_STATE_INSERT(portIndex,PORT_SM_ACTIVE_STATE);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
            }

            /* link check can start to take place, and user requested manual
            rx parameters can be set. At this point TRX training has finished
            and link should be up */
            if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0,(GT_U32)recvMsg->phyPortNum,(MV_HWS_PORT_STANDARD)recvMsg->portMode, &curPortParams))
            {
                mvPortCtrlApPortFail("Error, Port Mng, Port-%d hwsPortModeParamsGet return NULL\n",
                                     portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                return;
            }
            if (curPortParams.serdesSpeed == (MV_HWS_SERDES_SPEED)MCD_10_3125G)
            {
                index = MCD_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS;
                ctleOverride = MCD_TRUE;
            }
            else if (curPortParams.serdesSpeed == (MV_HWS_SERDES_SPEED)MCD_25_78125G)
            {
                index = MCD_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS;
                ctleOverride = MCD_TRUE;
            }

            /* call project callback for additional operations
              rrr On TRX training end
              1.    Configure external serdes clock mux and data mux at host side
              2.    Check if RX PLL to be ready
              3.    Switch clock (int 30 then int 0x5) at host side
              4.    Wait 10 msec overall
              5.    Un_reset RF_RESET at host sides
            */
            if (mvPortCtrlApCallbacks.apTrxTrainingEndClbk != NULL)
            {
                apStats->trxClbkStartTime = mvPortCtrlCurrentTs();
                mvPortCtrlApCallbacks.apTrxTrainingEndClbk(0 /*devNum*/, recvMsg->phyPortNum, (MV_HWS_PORT_STANDARD)recvMsg->portMode);
                apStats->trxClbkStopTime = mvPortCtrlCurrentTs();
            }
            if (ctleOverride == MCD_TRUE)
            {
                if (mvHwsPortManualCtleConfigGet(0, 0, (GT_U32)recvMsg->phyPortNum, (MV_HWS_PORT_STANDARD)recvMsg->portMode, 0xFF, ctleData) != GT_OK)
                {
                    mvPortCtrlApPortFail("Error, Port Mng, Port-%d hwsPortModeParamsGet return NULL\n",
                                         portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                    mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                    return;
                }

                if (portSm->apPortRxConfigOverride[index].fieldOverrideBmp & MCD_MAN_TUNE_ETL_ENABLE_CNS)
                {
                    etlMode = (GT_BOOL)((portSm->apPortRxConfigOverride[index].etlMaxDelay >> MCD_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT) & 1);
                }
                if (portSm->apPortRxConfigOverride[index].fieldOverrideBmp & MCD_MAN_TUNE_ETL_MIN_DELAY_CNS)
                {
                    /*bit[7]=1 means use user value and not the default*/
                    etlMinDelay = (GT_BOOL)((portSm->apPortRxConfigOverride[index].etlMinDelay & 0x7F) | (1<<MCD_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT));
                }
                if (portSm->apPortRxConfigOverride[index].fieldOverrideBmp & MCD_MAN_TUNE_ETL_MAX_DELAY_CNS)
                {
                    /*bit[7]=1 means use user value and not the default*/
                    etlMaxDelay = (GT_BOOL)((portSm->apPortRxConfigOverride[index].etlMaxDelay & 0x7F) | (1<<MCD_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT));
                }

                for (i = 0; i < (curPortParams.numOfActLanes); i++)
                {
                    singlePortSm = &(mvHwsPortCtrlApPortMng.apPortSm[(GT_U32)recvMsg->phyPortNum + i]);
                    if (singlePortSm->apPortRxConfigOverride[index].fieldOverrideBmp != 0)
                    {
                        fieldOverrideBmp = singlePortSm->apPortRxConfigOverride[index].fieldOverrideBmp;
                        if (fieldOverrideBmp & MCD_MAN_TUNE_CTLE_CONFIG_SQLCH_CNS)
                        {
                            ctleData[i].squelch = singlePortSm->apPortRxConfigOverride[index].squelch;
                            /*AP_PRINT_MAC(("ExecTrainingInProgress squelch %d \n",ctleData[i].squelch));*/
                        }
                        if (fieldOverrideBmp & MCD_MAN_TUNE_CTLE_CONFIG_LF_CNS)
                        {
                            ctleData[i].lowFrequency = (singlePortSm->apPortRxConfigOverride[index].lfHf & 0xF);
                            /*AP_PRINT_MAC(("ExecTrainingInProgress lowFrequency %d \n",ctleData[i].lowFrequency));*/
                        }
                        if (fieldOverrideBmp & MCD_MAN_TUNE_CTLE_CONFIG_HF_CNS)
                        {
                            ctleData[i].highFrequency = ((singlePortSm->apPortRxConfigOverride[index].lfHf >> MCD_MAN_TUNE_CTLE_CONFIG_OVERRIDE_HF_SHIFT) & 0xF);
                            /*AP_PRINT_MAC(("ExecTrainingInProgress highFrequency %d \n",ctleData[i].highFrequency));*/
                        }
                        if (fieldOverrideBmp & MCD_MAN_TUNE_CTLE_CONFIG_DCGAIN_CNS)
                        {
                            ctleData[i].dcGain = singlePortSm->apPortRxConfigOverride[index].dcGain;
                           /* AP_PRINT_MAC(("ExecTrainingInProgress dcGain %d \n",ctleData[i].dcGain));*/
                        }
                        if (fieldOverrideBmp & MCD_MAN_TUNE_CTLE_CONFIG_BANDWIDTH_CNS)
                        {
                            ctleData[i].bandWidth = (singlePortSm->apPortRxConfigOverride[index].bandLoopWidth & 0xF);
                            /*AP_PRINT_MAC(("ExecTrainingInProgress bandWidth %d \n",ctleData[i].bandWidth));*/
                        }
                        if (fieldOverrideBmp & MCD_MAN_TUNE_CTLE_CONFIG_LOOPBANDWIDTH_CNS)
                        {
                            ctleData[i].loopBandwidth = ((singlePortSm->apPortRxConfigOverride[index].bandLoopWidth >> MCD_MAN_TUNE_CTLE_CONFIG_OVERRIDE_LOOPWIDTH_SHIFT) & 0xF);
                            /*AP_PRINT_MAC(("ExecTrainingInProgress loopBandwidth %d \n",ctleData[i].loopBandwidth));*/
                        }
                        if (mvHwsPortManualCtleConfig(0, 0, (GT_U32)recvMsg->phyPortNum, (MV_HWS_PORT_STANDARD)recvMsg->portMode, i, &ctleData[i]) != GT_OK)
                        {
                            mvPortCtrlApPortFail("Error, Port Mng, Port-%d hwsPortModeParamsGet return NULL\n",
                                                 portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
                            mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                            return;
                        }
                    }
                }
            }
        }
    }
        /*
           after TRX-Training PASS,
           run the EnhanceTuneLite algorithm
        */
        tuneResult = TUNE_PASS;
        if (etlMode == GT_TRUE)
        {
            if (portSm->status != PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS)
            {
                portSm->status = PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS);
                mvHwsPortEnhanceTuneLiteByPhaseInitDb(0,(GT_U32)recvMsg->phyPortNum);
                /* log the start of enhanceTuneLight, we don't want to keep on logging
                   this entry, better to save the log space. Logging just the start and end */
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
            }

            trainPhase = &(hwsDeviceSpecInfo[0].portEnhanceTuneLiteDbPtr[recvMsg->phyPortNum].phaseEnhanceTuneLiteParams);

             /*
               get the PCAL RX-Training status.
               the PCAL RX-Training runs only in EnhanceTuneLite phase-1
            */

            if (trainPhase->phase == 1)
            {
                if (mvHwsPortAutoTuneStateCheck(0, /* devNum */
                                                0, /* portGroup */
                                                (GT_U32)recvMsg->phyPortNum,
                                                (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                                &rxTune,
                                                NULL /* TRX-Training PASS */) != GT_OK)
                {
                    mvPortCtrlApPortFail("Error, Port-%d mvHwsPortAutoTuneStateCheck Failed\n",
                                         portIndex, &msgType, recvMsg, PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL);

                    /* Training timer reset */
                    mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                    return;
                }


                /*
                   if PCAL RX-Training is not completed and timing of trainingInterval still not expired
                   --> Restart the threshold of Training timer and continue
                */
                if ((rxTune == TUNE_NOT_COMPLITED) && (portTimer->trainingCount < PORT_CTRL_TRAINING_INTERVAL))
                {
                    mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
                    return;
                }

                 /*
                   In case of PCAL RX-Training FAIL or
                   trainingInterval timer is expired and Training is not completed
                   --> set TRX-Training as FAIL and Reset the threshold of Training timer and
                       run TRX-Training flow again
                */
                if ((rxTune == TUNE_FAIL) || (tuneResult == TUNE_NOT_COMPLITED))
                {
                    tuneResult = TUNE_FAIL;

                    mvPortCtrlApPortFail("Error, Port-%d EnhanceTuneLite phase-1: PCAL TUNE_FAIL\n",
                                         portIndex, &msgType, recvMsg, PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL);

                    mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                    return;
                }
            }

            /*
               here, after running the EnhanceTuneLite phase-1
               --> PCAL RX-Training PASS
            */

            if (trainPhase->phase == 0)
            {   /* Initialize the EnhanceTubeLite state before execute */
                trainPhase->phase = 1;
            }
            else if (trainPhase->phaseFinished == GT_TRUE)
            {
                /*
                   For the first phase, run mcdSerdesEnhanceTuneLitePhase1.
                   In second phase, if all PCAL RX-Trainings PASS, run the mcdSerdesEnhanceTuneLitePhase2.
                */
                trainPhase->phase++;
            }

            /* run pCAL with shifted sampling point to find best sampling point */
            if (mvHwsPortEnhanceTuneLiteSetByPhase(0, /* devNum */
                                                   0, /* portGroup */
                                                   (GT_U32)recvMsg->phyPortNum,
                                                   (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                                   trainPhase->phase,
                                                   &(trainPhase->phaseFinished),
                                                   etlMinDelay/*bit[7]=0 means use default*/,
                                                   etlMaxDelay/*bit[7]=0 means use default*/) != GT_OK)
            {
                mvPortCtrlApPortFail("Error, Port-%d mvHwsPortEnhanceTuneLiteSetByPhase Failed\n",
                                     portIndex, &msgType, recvMsg, PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL);

                /* Training timer reset */
                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                return;
            }

            /*
               EnhanceTubeLite succeeded.
               continue by running Rx Adative-PCAL
            */
            if (trainPhase->phase == 2 && trainPhase->phaseFinished == GT_TRUE)
            {
                portSm->status = PORT_SM_SERDES_TRAINING_SUCCESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_SUCCESS);

                /* Clear training timer */
                portTimer->trainingCount = 0;
                /* Training timer reset */
                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                /* Start continuous Adaptive RX training after finish the EnhanceTuneLite */
                if (mvHwsPortAutoTuneSet(0, /* devNum */
                                         0, /* portGroup */
                                         (GT_U32)recvMsg->phyPortNum,
                                         (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                         RxTrainingAdative,
                                         NULL) != GT_OK)
                {
                    mvPortCtrlApPortFail("Error, Port-%d Adaptive training start Failed\n",
                                         portIndex, &msgType, recvMsg, PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL);

                    /* Training timer reset */
                    mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                    return;
                }
            }
            else
            {
                /*
                   after executing the mcdSerdesEnhanceTuneLitePhase1, return for checking the PCAL Rx-Training status.
                   If the RX-Training is PASS, return to execute the mcdSerdesEnhanceTuneLitePhase2.
                */
                return;
            }

             /*
               after executing Rx Adative-PCAL
            */
            /* Clear training timer */
            portTimer->trainingCount = 0;
            /* Training timer reset */
            mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
        }
        else /* mode == GT_FALSE */
        {
            /* Clear training timer */
            portTimer->trainingCount = 0;
            /* Training timer reset */
            mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
            /* Start continuous Adaptive RX training after finish the EnhanceTuneLite */
            if (mvHwsPortAutoTuneSet(0, /* devNum */
                                     0, /* portGroup */
                                     (GT_U32)recvMsg->phyPortNum,
                                     (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                     RxTrainingAdative,
                                     NULL) != GT_OK)
            {
                mvPortCtrlApPortFail("Error, Port-%d Adaptive training start Failed\n",
                                     portIndex, &msgType, recvMsg, PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL);

                /* Training timer reset */
                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));
                return;
            }
        }


        portSm->status = PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK;
        MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK);
        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

        /* Check System Config */
        if ((portTimer->sysCfState == PORT_SM_SERDES_SYSTEM_DISABLE) ||
           ((portTimer->sysCfState  == PORT_SM_SERDES_SYSTEM_ENABLE) &&
            (portTimer->sysCfStatus == PORT_SM_SERDES_SYSTEM_VALID)))
        {
            portSm->state  = PORT_SM_ACTIVE_STATE;
            MCD_STAT_PORT_SM_STATE_INSERT(portIndex,PORT_SM_ACTIVE_STATE);
            portSm->status = PORT_SM_SERDES_TRAINING_SUCCESS;
            MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_SUCCESS);
            mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
            /* Training timer reset */
            mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->trainingThreshold));

            /* Release message */
            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
        }
        else
        {
            /* System config timer start */
            mvPortCtrlThresholdSet(PORT_CTRL_SYSTEM_CONFIG_DURATION, &(portTimer->sysCfgThreshold));
        }
}


/*******************************************************************************
* mvPortCtrlApPortActiveExecTrainingNotReady
*
*******************************************************************************/
void mvPortCtrlApPortActiveExecTrainingNotReady(GT_U32 portIndex,
                                                  MCD_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    GT_U32                         msgType;
    MCD_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MV_HWS_AUTO_TUNE_STATUS_RES    tuneRes;

    tuneRes.txTune = TUNE_FAIL;
    if(portTimer->trainingCount >= portTimer->trainingMaxInterval)
    {
        portTimer->trainingCount=0;
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

   if(tuneRes.txTune == TUNE_NOT_READY)
   {
        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
   }
   else if ((tuneRes.txTune == TUNE_PASS) || (tuneRes.txTune == TUNE_READY))
   {
        portSm->status = PORT_SM_SERDES_TRAINING_IN_PROGRESS;
        MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_IN_PROGRESS);
        mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
        /* Training timer start */
        portTimer->trainingCount=0;
        mvPortCtrlThresholdSet(PORT_CTRL_TRAINING_DURATION, &(portTimer->trainingThreshold));
   }
   else
   {
      mvPortCtrlApPortFail("Error, Port Mng ExecTrainingNotReady, Port-%d training starting Failed\n",
                          portIndex, &msgType, recvMsg, PORT_SM_SERDES_TRAINING_FAILURE);
    }
}

/*******************************************************************************
* mvPortCtrlApPortActiveExecTrainingEndWaitAck
*
*******************************************************************************/
void mvPortCtrlApPortActiveExecTrainingEndWaitAck(GT_U32 portIndex,
                                                  MCD_IPC_CTRL_MSG_AP_STRUCT *recvMsg)
{
    GT_U32                         msgType;
    MCD_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);

    /* Check System Config */
    if ((portTimer->sysCfState == PORT_SM_SERDES_SYSTEM_DISABLE) ||
       ((portTimer->sysCfState  == PORT_SM_SERDES_SYSTEM_ENABLE) &&
        (portTimer->sysCfStatus == PORT_SM_SERDES_SYSTEM_VALID)))
    {
        portSm->state  = PORT_SM_ACTIVE_STATE;
        MCD_STAT_PORT_SM_STATE_INSERT(portIndex,PORT_SM_ACTIVE_STATE);
        portSm->status = PORT_SM_SERDES_TRAINING_SUCCESS;
        MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_SUCCESS);

        /* System config timer reset */
        mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->sysCfgThreshold));

        /* Release message */
        mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);

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

/*******************************************************************************
* mvPortCtrlApPortActiveExec
*
*******************************************************************************/
void mvPortCtrlApPortActiveExec(GT_U32 portIndex)
{
    GT_U32                        msgType;
    MCD_IPC_CTRL_MSG_AP_STRUCT *recvMsg = NULL;
    MCD_PORT_CTRL_PORT_SM      *portSm  = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MCD_AP_SM_INFO              *apSm      = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    if ((portSm->state == PORT_SM_SERDES_TRAINING_STATE) ||
        ((portSm->state == PORT_SM_ACTIVE_STATE) && (portSm->status == PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK ||
                                                     portSm->status == PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS)))
    {
        /* Check if there are active messages for execution */
        if (mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PROC) != GT_OK)
        {
            /* No messages to process */
            return;
        }

            if(AP_PORT_DELETE_IN_PROGRESS == apSm->status)
            {
                AP_PRINT_MCD2_MAC(("mvPortCtrlApPortActiveExec portIndex:%d deleted\n",portIndex));

                            /* Execute HWS Training check */
            mvHwsPortAutoTuneSet(0, /* devNum */
                                 0, /* portGroup */
                                 (GT_U32)recvMsg->phyPortNum,
                                 (MV_HWS_PORT_STANDARD)recvMsg->portMode,
                                 TRxTuneStop,
                                 NULL);
            portSm->status = PORT_SM_SERDES_TRAINING_FAILURE;
            MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_FAILURE);
            mvPortCtrlLogAdd(PORT_SM_LOG(portSm->state, portSm->status, portIndex));
            /* let training finish */
            hwsOsTimerWkFuncPtr(20);
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
        case PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS:
            mvPortCtrlApPortActiveExecTrainingInProgress(portIndex, recvMsg);
            break;
        case PORT_SM_SERDES_TRAINING_NOT_READY:
            mvPortCtrlApPortActiveExecTrainingNotReady(portIndex, recvMsg);
            break;
        case PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK:
            mvPortCtrlApPortActiveExecTrainingEndWaitAck(portIndex, recvMsg);
            break;
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
    MCD_IPC_CTRL_MSG_AP_STRUCT  *recvMsg,
    GT_U8                           status
)
{
    MCD_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_PORT_CTRL_AP_INIT       *portInit  = &(mvHwsPortCtrlApPortMng.apInitStatus);

    mvPcPrintf(errorMsg, recvMsg->phyPortNum);
    AP_PRINT_MCD_MAC(("mvPortCtrlApPortFail: port:%d %s\n",portIndex,errorMsg));
    portSm->status = status;
    MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,status);
    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
    mvPortCtrlApPortResetExec((GT_U32)recvMsg->phyPortNum, portSm, recvMsg);

    /* Mark port init failure */
    *portInit |= MV_PORT_CTRL_NUM(portIndex);

    /* Release message */
    mvPortCtrlApPortPendMsg(portIndex, &recvMsg, msgType, PORT_MSG_FREE);

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
    MCD_IPC_CTRL_MSG_AP_STRUCT     *recvMsg   = NULL;
    MCD_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    MCD_PORT_CTRL_AP_INIT       *portInit  = &(mvHwsPortCtrlApPortMng.apInitStatus);
    MCD_AP_SM_INFO              *apSm      = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
#ifndef BV_DEV_SUPPORT
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState;
    GT_U32                          fcRxPauseEn, fcTxPauseEn;
    MV_HWS_PORT_INIT_PARAMS        *curPortParams;
    #endif
    MV_HWS_PORT_STANDARD            portMode;
    MV_HWS_AUTO_TUNE_STATUS_RES     tuneRes;
    GT_STATUS                       rc        = GT_OK;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    /* Check if there are new pending messages for execution */
    if (mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PEEK) != GT_OK)
    {
        /* No messages to process */
        return;
    }

    /* Handle reset message in any port execution state */
    if ((msgType == MCD_IPC_PORT_RESET_MSG) ||
        (msgType == MCD_IPC_PORT_RESET_EXT_MSG))
    {
        if (mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_PROC) == GT_OK)
        {
            if ((portSm->state != PORT_SM_IDLE_STATE) &&
                (portSm->status != PORT_SM_DELETE_SUCCESS))
            {
                mvPortCtrlApPortResetExec((GT_U32)recvMsg->phyPortNum, portSm, recvMsg);
            }

            /* Clear port init failure */
            *portInit &= ~(MV_PORT_CTRL_NUM(portIndex));

            /* Release message */
            mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
            return;
        }
    }
    /* Handle init nessage in IDLE state, any other state Ignore */
    else if (msgType == MCD_IPC_PORT_INIT_MSG)
    {
        if (portSm->state == PORT_SM_IDLE_STATE)
        {
            if(AP_PORT_DELETE_IN_PROGRESS == apSm->status)
            {
                portSm->status = PORT_SM_DELETE_IN_PROGRESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_DELETE_IN_PROGRESS);
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
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_START_EXECUTE);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                portSm->state  = PORT_SM_SERDES_CONFIG_STATE;
                MCD_STAT_PORT_SM_STATE_INSERT(portIndex,PORT_SM_SERDES_CONFIG_STATE);
                portSm->status = PORT_SM_SERDES_CONFIG_IN_PROGRESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_CONFIG_IN_PROGRESS);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                /**PLACE Holder: HWS config SERDES*/
                if (0)
                {
                    portSm->status = PORT_SM_MAC_PCS_CONFIG_FAILURE;
                    MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_MAC_PCS_CONFIG_FAILURE);
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                    mvPortCtrlApPortResetExec((GT_U32)recvMsg->phyPortNum, portSm, recvMsg);

                    /* Mark port init failure */
                    *portInit |= MV_PORT_CTRL_NUM(portIndex);

                    /* Release message */
                    mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                    return;
                }

                /**PLACE Holder: HWS config MAC / PCS*/

                /* Execute HWS Port Init */
                portMode = (MV_HWS_PORT_STANDARD)recvMsg->portMode;
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
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_CONFIG_SUCCESS);
                portSm->state  = PORT_SM_MAC_PCS_CONFIG_STATE;
                MCD_STAT_PORT_SM_STATE_INSERT(portIndex,PORT_SM_MAC_PCS_CONFIG_STATE);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                portSm->status = PORT_SM_MAC_PCS_CONFIG_IN_PROGRESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_MAC_PCS_CONFIG_IN_PROGRESS);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                portSm->status = PORT_SM_MAC_PCS_CONFIG_SUCCESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_MAC_PCS_CONFIG_SUCCESS);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                portSm->status = PORT_SM_SERDES_POLARITY_SET_IN_PROGRESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_POLARITY_SET_IN_PROGRESS);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                rc = mvApPolarityCfg(apSm->portNum,portMode,apSm->polarityVector);
                if (rc != GT_OK)
                {
                    mvPortCtrlApPortFail("Error, Port Mng, Port-%d Set polarity Failed\n",
                                        portIndex, &msgType, recvMsg, PORT_SM_SERDES_POLARITY_SET_FAILURE);
                    return;
                }
                portSm->status = PORT_SM_SERDES_POLARITY_SET_SUCCESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_POLARITY_SET_SUCCESS);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));


                portSm->status = PORT_SM_FC_STATE_SET_IN_PROGRESS;
                MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_FC_STATE_SET_IN_PROGRESS);
                mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    if ((MCD_MODE_P100S  == (MCD_OP_MODE)portMode) ||
                        (MCD_MODE_P100C  == (MCD_OP_MODE)portMode) ||
                        (MCD_MODE_P50R2S == (MCD_OP_MODE)portMode) ||
                        (MCD_MODE_P40K  == (MCD_OP_MODE)portMode) ||
                        (MCD_MODE_P40C  == (MCD_OP_MODE)portMode) ||
                        (MCD_MODE_P25S  == (MCD_OP_MODE)portMode) ||
                        (MCD_MODE_P10S  == (MCD_OP_MODE)portMode))
                    {
                    portSm->status = PORT_SM_FEC_CONFIG_SUCCESS;
                    MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_FEC_CONFIG_SUCCESS);
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    portSm->state  = PORT_SM_SERDES_TRAINING_STATE;
                    MCD_STAT_PORT_SM_STATE_INSERT(portIndex,PORT_SM_SERDES_TRAINING_STATE);
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));
                    portSm->status = PORT_SM_SERDES_TRAINING_IN_PROGRESS;
                    MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_IN_PROGRESS);
                    mvPortCtrlLogAdd(AP_PORT_MNG_LOG(portSm->state, portSm->status, recvMsg->phyPortNum));

                    /* Enable the Tx signal, the signal was disabled during Serdes init */
                    if (mvHwsPortTxEnable(0, 0, (GT_U32)recvMsg->phyPortNum, portMode, GT_TRUE) != GT_OK)
                    {
                    mvPortCtrlApPortFail("Error, Port Mng, Port-%d mvHwsPortTxEnable Failed\n",
                                            portIndex, &msgType, recvMsg, PORT_SM_MAC_PCS_CONFIG_FAILURE);
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
                        MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_NOT_READY);
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
                    }
                }
                else
                {
                    portSm->state  = PORT_SM_ACTIVE_STATE;
                    MCD_STAT_PORT_SM_STATE_INSERT(portIndex,PORT_SM_ACTIVE_STATE);
                    portSm->status = PORT_SM_SERDES_TRAINING_SUCCESS;
                    MCD_STAT_PORT_SM_STATUS_INSERT(portIndex,PORT_SM_SERDES_TRAINING_SUCCESS);

                    /* System config timer reset */
                    mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(portTimer->sysCfgThreshold));

                    /* Release message */
                    mvPortCtrlApPortPendMsg(portIndex, &recvMsg, &msgType, PORT_MSG_FREE);
                }
            }
        }
    }
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
GT_STATUS mvPortCtrlApPortPendAdd(GT_U32 port, MCD_IPC_CTRL_MSG_AP_STRUCT *msg)
{
    GT_U8 nextPendTaskAddIndex;
    MCD_PORT_CTRL_PEND_MSG_TBL *portCtrlSmPendMsgPtr;

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
        osMemCpy(&(portCtrlSmPendMsgPtr->pendTasks[nextPendTaskAddIndex]), msg, sizeof(MCD_IPC_CTRL_MSG_AP_STRUCT));
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
GT_STATUS mvPortCtrlApPortPendMsg(GT_U32 port, MCD_IPC_CTRL_MSG_AP_STRUCT **msg,
                                  GT_U32 *msgType, GT_U32 action)
{
    GT_U8 nextPendTasksIndex;
    MCD_PORT_CTRL_PEND_MSG_TBL *portCtrlSmPendMsgPtr;

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
    MCD_IPC_CTRL_MSG_AP_STRUCT apIpcMsg;

    apIpcMsg.ctrlMsgType    = msgType;
    apIpcMsg.msgQueueId     = queueId;
    apIpcMsg.phyPortNum     = portNum;
    apIpcMsg.portMode       = portMode;
    apIpcMsg.action         = action;
    apIpcMsg.refClock       = refClk;
    apIpcMsg.refClockSource = refClkSrc;

    return mvPortCtrlApPortPendAdd(portIndex, &apIpcMsg);
}

/*******************************************************************************
*                            AP Port Log State/Status                          *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mcdStatPortSmStateInsert function
* @endinternal
*
* @brief   Port Log State Insert
*          Keeping the PORT_SM state information coded in the argument
*          Binary 00 01 10 11... - means according to the MV_AP_PORT_SM_STATE above
*          This function must be updated if the State # will raise above 15 (4 bits coded)
*/
void  mcdStatPortSmStateInsert(GT_U8 portIndex, GT_U16 state)
{
#ifdef  MCD_AP_STATE_STATUS_LOG
    MCD_PORT_SM_DECODE(portIndex)      = ((MCD_PORT_SM_DECODE(portIndex) << 4) | ((MCD_U16)state & 0xF));
    MCD_PORT_SM_DECODE_TYPE(portIndex) = ((MCD_PORT_SM_DECODE_TYPE(portIndex) << 1) | ((MCD_U16)1));
#endif  /*MCD_AP_STATE_STATUS_LOG*/
}


/**
* @internal mcdStatPortSmStatusInsert function
* @endinternal
*
* @brief   Port Port Log Status Insert
*          Keeping the PORT_SM state information coded in the argument
*          Binary 00000 00001 00010 00011... - means according to the MV_AP_PORT_SM_STATUS above
*          This function must be updated if the Status # will raise above 31 (5 bits coded)
*/
void  mcdStatPortSmStatusInsert(GT_U8 portIndex, GT_U16 status)
{
#ifdef  MCD_AP_STATE_STATUS_LOG
    MCD_PORT_SM_DECODE(portIndex)      = ((MCD_PORT_SM_DECODE(portIndex) << 5) | ((MCD_U16)status & 0x1F));
    MCD_PORT_SM_DECODE_TYPE(portIndex) = ((MCD_PORT_SM_DECODE_TYPE(portIndex) << 1));
#endif  /*MCD_AP_STATE_STATUS_LOG*/
}

