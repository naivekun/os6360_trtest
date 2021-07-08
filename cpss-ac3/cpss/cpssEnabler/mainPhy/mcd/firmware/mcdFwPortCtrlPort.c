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
* @file mcdFwPortCtrlPort.c
*
* @brief Port Control Port State Machine
*
* @version   1
********************************************************************************
*/
#include <mcdFwPortCtrlInc.h>
/* Avago include */
#include <aapl.h>
#include <aapl_core.h>
#include <serdes_core.h>
#include <sbus.h>

#include "mcdApiTypes.h"
#include "mcdApiRegs.h"
#include "mcdHwSerdesCntl.h"
#include "mcdAPI.h"
#include "mcdAPIInternal.h"
#include "mcdHwCntl.h"

static MCD_PORT_CTRL_COMMON *infoPtr;
extern MCD_DEV mcdDevDb;
extern MCD_SERDES_TXRX_TUNE_PARAMS mcdSerdesTxRxTuneParamsArray[];
extern MCD_STATUS mcdGetPortParams
(
    IN MCD_DEV_PTR pDev,
    IN MCD_U32 mdioPort,
    IN MCD_OP_MODE portMode,
    OUT MCD_U32  *laneSpeed,
    OUT MCD_U32 *numOfActLanes
);

void mcdFwPortCtrlPollingDataInit(void);
void mcdFwPortCtrlPollingLinkCheckIdle(void);
GT_STATUS mcdFwPortCtrlPollingLinkCheckEnable(MCD_IPC_CTRL_MSG_STRUCT *msg, MCD_U32 enable);

static int mcdClbkMode = 0;
/* 0 - 2 clbck as planed
   1 - only hcd clbck used
   2 - hcd clbck swith clks and training clbck set rf_reset
*/

/**
* @internal mvPortCtrlPortRoutine function
* @endinternal
*
* @brief   Port mng process execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvPortCtrlPortRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvPortCtrlPortMng.info);

    mcdFwPortCtrlPollingDataInit();

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
        if (mvPortCtrlPortMng.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvPortCtrlPortMng.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Ctrl, Port Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
        mcdFwPortCtrlPollingLinkCheckIdle();
    }
}

/**
* @internal mvPortCtrlPortResetExec function
* @endinternal
*
* @brief   Port mng message processing - reset execution
*         - Execute port reset and return to Idle state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlPortResetExec(GT_U32 port,
                                  MCD_PORT_CTRL_PORT_SM *info,
                                  MCD_IPC_CTRL_MSG_STRUCT *msg)
{
    MCD_IPC_PORT_RESET_DATA_STRUCT *msgParams;

    msgParams = (MCD_IPC_PORT_RESET_DATA_STRUCT*)&(msg->msgData);

    info->status = PORT_SM_DELETE_IN_PROGRESS;
    mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));

    /* Execute HWS Port Reset */
    if (mvHwsPortReset(msg->devNum,
                       msgParams->portGroup,
                       msgParams->phyPortNum,
                       msgParams->portMode,
                       msgParams->action) != GT_OK)
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

/**
* @internal mvPortCtrlPortMsg function
* @endinternal
*
* @brief   Port mng message processing state execution
*         - Read message from port message queue queue
*         Message received
*         Add Log entry
*         Execute message to completion
*         Send reply message
*         Message not received
*         Trigger state change
*/
void mvPortCtrlPortMsg(void)
{
    GT_STATUS                        rcode;
    MCD_IPC_CTRL_MSG_STRUCT          recvMsg;
    MCD_IPC_PORT_INIT_DATA_STRUCT    *msgParams;

    infoPtr->state = PORT_MNG_MSG_PROCESS_STATE;
    osMemSet(&recvMsg, 0, sizeof(MCD_IPC_CTRL_MSG_STRUCT));

    while (1)
    {
        rcode = mvPortCtrlProcessMsgRecv(M2_PORT_MNG, MV_PROCESS_MSG_RX_NO_DELAY, &recvMsg);
        if (rcode != GT_OK) break;
        switch (recvMsg.ctrlMsgType)
        {
            case MCD_IPC_PORT_POLLING_START_MSG:
                rcode = mcdFwPortCtrlPollingLinkCheckEnable(
                    &recvMsg, 1);
                /* reply MCD_OK already sent by Supervisor task */
                break;
            case MCD_IPC_PORT_POLLING_STOP_MSG:
                rcode = mcdFwPortCtrlPollingLinkCheckEnable(
                    &recvMsg, 0);
                /* reply MCD_OK already sent by Supervisor task */
                break;
            default:
                msgParams = (MCD_IPC_PORT_INIT_DATA_STRUCT*)&(recvMsg.msgData);
                mvPortCtrlLogAdd(PORT_MNG_LOG(
                    PORT_MNG_MSG_PROCESS_STATE, msgParams->phyPortNum, recvMsg.ctrlMsgType));
                rcode = mvPortCtrlPortMsgExec(msgParams->phyPortNum, &recvMsg);
                mvPortCtrlPortMsgReply(rcode, recvMsg.msgQueueId, recvMsg.ctrlMsgType);
                break;
        }
    }

    infoPtr->event = PORT_MNG_DELAY_EVENT;
}

/**
* @internal mvPortCtrlPortMsgExec function
* @endinternal
*
* @brief   Port mng message processing state execution
*         - Idle State
*         Init message - Execute port init from Idle state to Active state
*         Reset message - Execute port reset and return to Idle state
*         - Active State
*         Init message - Return Error
*         Reset message - Execuet port reset and return to Idle state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlPortMsgExec(GT_U32 port,
                                MCD_IPC_CTRL_MSG_STRUCT *msg)
{
    MCD_IPC_PORT_INIT_DATA_STRUCT *msgParams;
    MCD_PORT_CTRL_PORT_SM         *portCtrlSmPtr = &(mvPortCtrlPortSm[port]);
    GT_U8                            msgType = msg->ctrlMsgType;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    switch (portCtrlSmPtr->state)
    {
        case PORT_SM_IDLE_STATE:
            if (msgType == MCD_IPC_PORT_INIT_MSG)
            {
                portCtrlSmPtr->status = PORT_SM_START_EXECUTE;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                portCtrlSmPtr->state  = PORT_SM_SERDES_CONFIG_STATE;
                portCtrlSmPtr->status = PORT_SM_SERDES_CONFIG_IN_PROGRESS;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                /**PLACE Holder: HWS config SERDES*/
                if (0)
                {
                    portCtrlSmPtr->status = PORT_SM_MAC_PCS_CONFIG_FAILURE;
                    mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                    /* Move to Idle state to enable SM execution for future messages */
                    portCtrlSmPtr->state  = PORT_SM_IDLE_STATE;
                    return GT_FAIL;
                }
                portCtrlSmPtr->status = PORT_SM_SERDES_CONFIG_SUCCESS;
                portCtrlSmPtr->state  = PORT_SM_MAC_PCS_CONFIG_STATE;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                portCtrlSmPtr->status = PORT_SM_MAC_PCS_CONFIG_IN_PROGRESS;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                /**PLACE Holder: HWS config MAC / PCS*/
                msgParams = (MCD_IPC_PORT_INIT_DATA_STRUCT*)&(msg->msgData);
                osMemSet(&portInitInParam, 0,sizeof(portInitInParam));
                portInitInParam.lbPort = msgParams->lbPort;
                portInitInParam.refClock = msgParams->refClock;
                portInitInParam.refClockSource = msgParams->refClockSource;

                /* Execute HWS Port Init */
                if (mvHwsPortInit(msg->devNum,
                                  msgParams->portGroup,
                                  msgParams->phyPortNum,
                                  msgParams->portMode,
                                  &portInitInParam) != GT_OK)
                {
                    mvPcPrintf("Error, Port Mng, Port-%d Init Failed\n", port);

                    portCtrlSmPtr->status = PORT_SM_SERDES_CONFIG_FAILURE;
                    mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                    /* Move to Idle state to enable SM execution for future messages */
                    portCtrlSmPtr->state  = PORT_SM_IDLE_STATE;
                    return GT_FAIL;
                }
                portCtrlSmPtr->status = PORT_SM_MAC_PCS_CONFIG_SUCCESS;
                portCtrlSmPtr->state  = PORT_SM_ACTIVE_STATE;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
            }
            else if ((msgType == MCD_IPC_PORT_RESET_MSG) ||
                     (msgType == MCD_IPC_PORT_RESET_EXT_MSG))
            {
                if (mvPortCtrlPortResetExec(port, portCtrlSmPtr, msg) != GT_OK)
                {
                    return GT_FAIL;
                }
            }
            break;

        case PORT_SM_ACTIVE_STATE:
            if ((msgType == MCD_IPC_PORT_RESET_MSG) ||
                (msgType == MCD_IPC_PORT_RESET_EXT_MSG))
            {
                if (mvPortCtrlPortResetExec(port, portCtrlSmPtr, msg) != GT_OK)
                {
                    return GT_FAIL;
                }
            }
            else if (msgType == MCD_IPC_PORT_INIT_MSG)
            {
                mvPcPrintf("Error, Port Mng, Port-%d Init in Active state\n", port);
            }
            break;
    }

    return GT_OK;
}

/**
* @internal mvPortCtrlPortMsgReply function
* @endinternal
*
* @brief   Port mng message reply
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlPortMsgReply(GT_U32 rcode, GT_U8 queueId, GT_U8 msgType)
{
    MCD_IPC_REPLY_MSG_STRUCT reply;

    reply.replyTo    = msgType;
    reply.returnCode = rcode;

    /* Build and Send IPC reply */
    CHECK_STATUS(mvHwsIpcReplyMsg(queueId, &reply));

    return GT_OK;
}

/**
* @internal mvPortCtrlPortDelay function
* @endinternal
*
* @brief   Port mng message delay state execution
*         - Execute delay
*         - Trigger state change
*/
void mvPortCtrlPortDelay(void)
{
    infoPtr->state = PORT_MNG_DELAY_STATE;
    mvPortCtrlProcessDelay(infoPtr->delayDuration);
    infoPtr->event = PORT_MNG_MSG_PROCESS_EVENT;
}

/*============================================================================*/
/*                    Port Polling State Machine                              */
/*============================================================================*/
#include <FreeRTOS.h>
#include <stdint.h>
#include <string.h>
#include <strtoul.h>
#include <FreeRTOS_CLI.h>
static int mcdFwPortCtrlPollingTrace_EnableCmdRegistred = 0;
static MCD_U32 mcdFwPortCtrlPollingTrace_EnableMask = 0;
/* Set Trace mask commans */
static portBASE_TYPE mcdFwPortCtrlPollingTrace_MaskSet(
    int8_t *pcWriteBuffer,
    size_t xWriteBufferLen,
    const int8_t *pcCommandString )
{
    const char *p = (const char *)pcCommandString;
    char *pEnd;
    unsigned long mask;

    /* bypass command name */
    while (*p > ' ') {p++;};

    mask = strtoul(p, &pEnd, 0/*base*/);
    mcdFwPortCtrlPollingTrace_EnableMask = mask;
    printf("MCD Port polling trace mask set to %d", mask);

    return pdFALSE;
}

static const CLI_Command_Definition_t mcdFwPortCtrlPollingTrace_MaskSetCommand =
{
    ( const int8_t * const ) "mcd_trace_mask",
    ( const int8_t * const ) "mcd_trace_mask <mask>:\n mask for MCD port polling trace.\n\n",
    mcdFwPortCtrlPollingTrace_MaskSet,
    1
};

static void mcdFwPortCtrlPollingTrace_MaskSetCmdRegister(void);/*prototype*/
static void mcdFwPortCtrlPollingTrace_MaskSetCmdRegister()
{
    if (mcdFwPortCtrlPollingTrace_EnableCmdRegistred == 0)
    {
        FreeRTOS_CLIRegisterCommand(&mcdFwPortCtrlPollingTrace_MaskSetCommand);
        mcdFwPortCtrlPollingTrace_EnableCmdRegistred = 1;
    }
}

/* both in for Retimer and for PCS modes the primary check                 */
/* classifies lanes by 2 criterias: SINAL OFF/ON AND and CHANGED/UNCHANGED */
typedef enum
{
    /* elements */
    PRV_SIGNAL_STATE_OFF_UNCHANGED = 1,
    PRV_SIGNAL_STATE_ON_UNCHANGED = 2,
    PRV_SIGNAL_STATE_OFF_CHANGED = 4,
    PRV_SIGNAL_STATE_ON_CHANGED = 8,
    /* sets */
    PRV_SIGNAL_STATE_NONE = 0,
    PRV_SIGNAL_STATE_OFF =
        (PRV_SIGNAL_STATE_OFF_UNCHANGED | PRV_SIGNAL_STATE_OFF_CHANGED),
    PRV_SIGNAL_STATE_ON =
        (PRV_SIGNAL_STATE_ON_UNCHANGED | PRV_SIGNAL_STATE_ON_CHANGED),
    PRV_SIGNAL_STATE_CHANGED =
        (PRV_SIGNAL_STATE_OFF_CHANGED | PRV_SIGNAL_STATE_ON_CHANGED),
    PRV_SIGNAL_STATE_UNCHANGED =
        (PRV_SIGNAL_STATE_OFF_UNCHANGED | PRV_SIGNAL_STATE_ON_UNCHANGED),
    PRV_SIGNAL_STATE_ANY =
        (PRV_SIGNAL_STATE_OFF |  PRV_SIGNAL_STATE_ON)

} PRV_SIGNAL_STATE_ENT;

/* used to get bitmap of tuned serdes lanes */
static MCD_U16 prvMcdFwPortCtrlPollingTuneSerdesBmpGet
(
    IN PRV_SIGNAL_STATE_ENT signalStateBmp,
    IN MCD_U16 fullSerdesBmp,
    IN MCD_U16 signalOnSerdesBmp,
    IN MCD_U16 signalChangedSerdesBmp
)
{
    /* supposed and not checked that fullSerdesBmp contains other bitmaps */
    MCD_U16 signalOnChangedSerdesBmp    = (signalOnSerdesBmp & signalChangedSerdesBmp);
    MCD_U16 signalOnUnhangedSerdesBmp   = (signalOnSerdesBmp & (~ signalChangedSerdesBmp));
    MCD_U16 signalOffChangedSerdesBmp   = ((~ signalOnSerdesBmp) & signalChangedSerdesBmp);
    MCD_U16 signalOffUnhangedSerdesBmp  =
        (fullSerdesBmp & (~ signalOnSerdesBmp) & (~ signalChangedSerdesBmp));
    MCD_U16 tuneSerdesBmp = 0;

    if (signalStateBmp & PRV_SIGNAL_STATE_OFF_UNCHANGED) tuneSerdesBmp |= signalOffUnhangedSerdesBmp;
    if (signalStateBmp & PRV_SIGNAL_STATE_ON_UNCHANGED)  tuneSerdesBmp |= signalOnUnhangedSerdesBmp;
    if (signalStateBmp & PRV_SIGNAL_STATE_OFF_CHANGED)   tuneSerdesBmp |= signalOffChangedSerdesBmp;
    if (signalStateBmp & PRV_SIGNAL_STATE_ON_CHANGED)    tuneSerdesBmp |= signalOnChangedSerdesBmp;

    return tuneSerdesBmp;
}

/* Retimer */

/* Run ICAL from ON to OFF state */
PRV_SIGNAL_STATE_ENT mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_OnToOffICal =
    PRV_SIGNAL_STATE_ANY;

/* Run ICAL from from CHECK_STABILITY on bad voltage data offset */
PRV_SIGNAL_STATE_ENT mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityDvos =
    PRV_SIGNAL_STATE_ANY;

/* Run ICAL from CHECK_STABILITY to OFF state */
PRV_SIGNAL_STATE_ENT mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityToOffICal =
    PRV_SIGNAL_STATE_ANY;

/* Run ICAL from CHECK_STABILITY to ON state */
int mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityToOnICal = 0;

/* Run <PCAL> from CHECK_STABILITY to ON state */
int mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityToOnPCal = 1;

/* PCS */

/* Run ICAL from ON to OFF state */
PRV_SIGNAL_STATE_ENT mcdFwPortCtrlPollingLinkCheckSignalOnPcs_OnToOffICal =
    PRV_SIGNAL_STATE_ANY;

/* Run ICAL from from CHECK_STABILITY on bad voltage data offset */
PRV_SIGNAL_STATE_ENT mcdFwPortCtrlPollingLinkCheckSignalOnPcs_CheckStabilityDvos =
    PRV_SIGNAL_STATE_ANY;

/* Run ICAL from CHECK_STABILITY to OFF state */
PRV_SIGNAL_STATE_ENT mcdFwPortCtrlPollingLinkCheckSignalOnPcs_CheckStabilityToOffICal =
    PRV_SIGNAL_STATE_ANY;

/* Run ICAL from CHECK_STABILITY to ON state */
int mcdFwPortCtrlPollingLinkCheckSignalOnPcs_CheckStabilityToOnICal = 0;

/* Run <PCAL> from CHECK_STABILITY to ON state */
int mcdFwPortCtrlPollingLinkCheckSignalOnPcs_CheckStabilityToOnPCal = 1;

/* Run <PCAL> from ON to ON state when PCS Operational staus == 0 */
/* always used full serdes mask                                   */
int mcdFwPortCtrlPollingLinkCheckSignalOnPcs_OnToOnNoPcsOperationalPCal = 1;


#define POLLING_TRACE_EXT(_mask, x) \
    if (mcdFwPortCtrlPollingTrace_EnableMask & _mask){printf("[%04d]",__LINE__); printf x;}
#define POLLING_ERROR(x)      POLLING_TRACE_EXT(1, x)
#define POLLING_REPORT(x)     POLLING_TRACE_EXT(2, x)
#define POLLING_ICAL_TRACE(x) POLLING_TRACE_EXT(4, x)
#define POLLING_TRACE(x)      POLLING_TRACE_EXT(8, x)

#define POLLING_NUM_OF_SERDES_HOST_CNS         8
#define POLLING_NUM_OF_SERDES_LINE_CNS         8
#define POLLING_NUM_OF_PORTS                   8
/* Squelch values converted to signal_ok_threshold */
#define POLLING_SERDES_SQUELCH_HIGHEST         0
#define POLLING_SERDES_SQUELCH_DEFAULT         0

#define POLLING_SERDES_SHORT_DWELL            16
#define POLLING_SERDES_LONG_DWELL             16 /*default*/

#define POLLING_ATTEMPT(xFuncToTry) \
    {if (xFuncToTry != MCD_OK) \
    {POLLING_ERROR(("POLLING_ATTEMPT failed at <<<%s>>>\n", #xFuncToTry)); \
    return MCD_FAIL;}}

#define POLLING_ATTEMPT_VOID(xFuncToTry) \
    {if (xFuncToTry != MCD_OK) \
    {POLLING_ERROR(("POLLING_ATTEMPT failed at <<<%s>>>\n", #xFuncToTry)); \
    return;}}

#define POLLING_EXEC(xFuncToTry) \
    {if (xFuncToTry != MCD_OK) \
    {POLLING_ERROR(("POLLING_ATTEMPT failed at <<<%s>>>\n", #xFuncToTry));}}

/* The CPU core clock cycle in BV is 170MHz, so clock is around 5.88ns */
/* The 32-bit clock counter overflows each 24 seconds approximatelly   */
#define POLLING_PORTS_MILLIECOND_TIME         170000 /*one millisecond in clocks */
/* both modes */
#define POLLING_PORTS_SIGNAL_CHECK_TIME              (POLLING_PORTS_MILLIECOND_TIME * 100)
/* PCS mode */
#define POLLING_PORTS_PCS_OPER_OFF_PCAL_TICK_TIME    100
#define POLLING_PORTS_PCS_OPER_OFF_PCAL_MAX_COUNT     10

/*This is 300 msec since polling rate is every 100 msec*/
#define POLLING_PORTS_MAX_UNSTABLE_SIGNAL_TICK_TIME  300

/* noPpm delay for clocks cinfiguration */
#define PRV_MCD_DELAY_NOPPM_CLOCK_SWITCH 25


typedef enum
{
    POLLING_PORT_STAGE_INVALID,
    POLLING_PORT_STAGE_INITIALIZED,
    POLLING_PORT_STAGE_COMPLETE

} POLLING_PORT_STAGE;

typedef enum
{
    POLLING_PORT_LINK_CHECK_STAGE_START,
    POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON,
    POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF,
    POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY,
    POLLING_PORT_LINK_CHECK_STAGE_LAST

} POLLING_PORT_LINK_CHECK_STAGE;

typedef struct
{
    uint     sbus_addr;
    MCD_U16  eye_val;
} POLLING_SERDES_STATE_STC;

typedef struct
{
    MCD_U16                       mdioPort;
    MCD_U16                       retimerMode;
    POLLING_PORT_STAGE            stage;
    MCD_U32                       lineDelayStartTime;
    MCD_U32                       lineDelayDuration; /* 0 means no delay */
    MCD_U32                       hostDelayStartTime;
    MCD_U32                       hostDelayDuration; /* 0 means no delay */
    MCD_SERDES_SPEED              lineSerdesSpeed;
    MCD_SERDES_SPEED              hostSerdesSpeed;
    MCD_U16                       lineSerdesBitmap;
    MCD_U16                       hostSerdesBitmap;
    MCD_U16                       lineSignalOnSerdesBitmap;
    MCD_U16                       hostSignalOnSerdesBitmap;
    POLLING_PORT_LINK_CHECK_STAGE lineLinkCheckStage;
    POLLING_PORT_LINK_CHECK_STAGE hostLinkCheckStage;
    MCD_U16                       lineSerdesTrained;
    MCD_U16                       hostSerdesTrained;
    MCD_U16                       linePcsOperationOffCount;
    MCD_U16                       hostPcsOperationOffCount;
    MCD_U32                       linePcsOperationOffTickTime;
    MCD_U32                       hostPcsOperationOffTickTime;
    MCD_U32                       lineSignalUnstableStartTickTime;
    MCD_U32                       hostSignalUnstableStartTickTime;
    /* statistic used last times */
    MCD_U32                       hostLastToOffTime;
    MCD_U32                       lineLastToOffTime;
    MCD_U32                       hostLastToCheckStabilityTime;
    MCD_U32                       lineLastToCheckStabilityTime;
    MCD_U32                       hostLastToOnTime;
    MCD_U32                       lineLastToOnTime;
    MCD_U32                       hostLinkUpStartTime;
    MCD_U32                       lineLinkUpStartTime;
    /*used for AP retimer mode*/
    MCD_U16                       apLaneNum;
    MCD_U32                       apAdvMode;
    MCD_U16                       apOptions;
    MCD_U8                        apPolarityVector;
    MCD_U8                        retimerWithAp;
    /* used for manual featues */
    MCD_U16                       noPpmMode;
    MCD_U16                       hostRxRemapVecotr;
    MCD_U16                       hostTxRemapVecotr;
    MCD_U16                       lineRxRemapVector;
    MCD_U16                       lineTxRemapVector;
    MCD_CTLE_CALIBRATION_MODE_E   calibrationMode;
    MCD_CALIBRATION_ALGO_E        calibrationAlgo;
    MCD_U32                       minEyeThreshold;
    MCD_U32                       maxEyeThreshold;
    MCD_CONFIDENCE_INTERVAL_PARAMS_OVERRIDE  confidenceParams;
} POLLING_PORT_STATE_STC;


typedef struct
{
    MCD_U32                       lineStartTime;
    MCD_U32                       hostStartTime;
} POLLING_PORT_TIME_STAMP_STC;

static POLLING_SERDES_STATE_STC mcdFwPortCtrlHostSerdesState[POLLING_NUM_OF_SERDES_HOST_CNS] = {{0}};
static POLLING_SERDES_STATE_STC mcdFwPortCtrlLineSerdesState[POLLING_NUM_OF_SERDES_LINE_CNS] = {{0}};
static POLLING_PORT_STATE_STC   mcdFwPortCtrlPortState[POLLING_NUM_OF_PORTS] = {{0}};
static MCD_U8                   mcdFwPortCtrlPortEoCounter[2][POLLING_NUM_OF_PORTS] = {{0}};
static MCD_U8                   mcdFwPortCtrlPortVerifyStopAdaptive[2][POLLING_NUM_OF_PORTS] = {{0}};



MCD_POLLING_PORT_STATISTIC_STC mcdFwPortCtrlPollingStat;

/* returns time from time0 to time1 */
MCD_U32 mcdFwPortCtrlPortGetTimeDiff(MCD_U32 time1, MCD_U32 time0)
{
    /* assumed that sysem time reaches 0xFFFFFFFF and continues by 0 */
    /* full time cycle is about 50 days                              */
    static const MCD_U32 max_time = 0xFFFFFFFF;
    if (time1 >= time0)
    {
        return (time1 - time0);
    }
    /* time overflow */
    return ((max_time - time0) + time1);
}

/*
- setSerdesTxRxAttributesAndMaxSquelch
- setSerdesTxRxAttributesAndDefaultSquelch
- runSerdesRxTraining -  existing mcdSerdesEnhanceTune
- setClockForRetimerMode -  existing mcdSerdesSetRetimerClock(pDev, serdesLs, baudRate)
- runApOnPort - existing mvHwsInitialConfiguration(portIndex)
- runSerdesTxRxTraining
- stop AP on Port - existing mvApPortDeleteMsg(portIndex)
*/

/* returns 1 - if from time_start to time_cur passed not least the time_delay, */
/* 0 - otherwise                                                               */
MCD_U32 mcdFwPortCtrlPollingIsTimePassed
(
    MCD_U32 time_start,
    MCD_U32 time_cur,
    MCD_U32 time_delay
)
{
    MCD_U32 time_bound = time_start + time_delay;
    MCD_U32 delay_time_passed = 0;

    if ((time_start < time_bound))
    {
        /* no overflow adding delay_time_passed */
        if ((time_cur > time_bound) || (time_cur < time_start))
        {
            /* current time already not between start and bound */
            delay_time_passed = 1;
        }
    }
    else /*(time_bound < time_start)*/
    {
        /* overflow adding delay_time_passed */
        if ((time_cur > time_bound) && (time_cur < time_start))
        {
            /* current time already between bound and start */
            delay_time_passed = 1;
        }
    }
    return delay_time_passed;
}

void mcdFwPortCtrlPollingDataInit(void)
{
    MCD_U32 i;

    mcdFwPortCtrlPollingTrace_MaskSetCmdRegister();
    osMemSet(mcdFwPortCtrlLineSerdesState, 0, sizeof(mcdFwPortCtrlLineSerdesState));
    osMemSet(mcdFwPortCtrlHostSerdesState, 0, sizeof(mcdFwPortCtrlHostSerdesState));
    osMemSet(mcdFwPortCtrlPortState, 0, sizeof(mcdFwPortCtrlPortState));
    osMemSet(&mcdFwPortCtrlPollingStat, 0, sizeof(mcdFwPortCtrlPollingStat));

    for (i = 0; (i < POLLING_NUM_OF_PORTS); i++)
    {
        mcdFwPortCtrlPortState[i].stage          = POLLING_PORT_STAGE_INVALID;
        mcdFwPortCtrlPortState[i].mdioPort       = i;
        mcdFwPortCtrlPortState[i].lineDelayDuration  = 0;
        mcdFwPortCtrlPortState[i].hostDelayDuration  = 0;
        mcdFwPortCtrlPortState[i].lineLinkCheckStage = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
        mcdFwPortCtrlPortState[i].hostLinkCheckStage = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
    }
    for (i = 0; (i < POLLING_NUM_OF_SERDES_HOST_CNS); i++)
    {
        mcdFwPortCtrlHostSerdesState[i].sbus_addr = (i + 9); /* 9-18 at host side */
    }
    for (i = 0; (i < POLLING_NUM_OF_SERDES_LINE_CNS); i++)
    {
        mcdFwPortCtrlLineSerdesState[i].sbus_addr = (i + 1); /* 1-8 at line side */
    }
}

void mcdFwPortCtrlPollingLinkCheckDfeStateGet
(
    IN   MCD_U32 portIndex,
    IN   int     lineSide,
    IN   MCD_U16 serdesBitmap,
    OUT  MCD_U16 *notCompletedDfeSerdesBitmapPtr,
    OUT  MCD_U16 *failedDfeSerdesBitmapPtr
)
{
    MCD_AUTO_TUNE_STATUS           rxStatus;
    MCD_U32                        i;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdes;

    if (lineSide)
    {
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
    }
    else
    {
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
    }

    *notCompletedDfeSerdesBitmapPtr = 0;
    *failedDfeSerdesBitmapPtr       = 0;
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        POLLING_ATTEMPT_VOID(mcdSerdesRxAutoTuneStatusGet(
            &mcdDevDb, serdes, &rxStatus));
        if (rxStatus == MCD_TUNE_NOT_COMPLITED)
        {
            *notCompletedDfeSerdesBitmapPtr |= (1 << i);
        }
        else if (rxStatus != MCD_TUNE_PASS)
        {
            mcdFwPortCtrlPollingStat.trainFailCount[serdes] ++;
            mcdFwPortCtrlPollingStat.countICalFailed[serdes] ++;
            *failedDfeSerdesBitmapPtr |= (1 << i);
        }
    }
}

void mcdFwPortCtrlPollingLinkCheckTrainingStatusGet
(
    IN   MCD_U32 portIndex,
    IN   int     lineSide,
    IN   MCD_U16 serdesBitmap,
    OUT  MCD_U16 *failedDfeSerdesBitmapPtr
)
{
    MCD_AUTO_TUNE_STATUS           rxStatus;
    MCD_U32                        i;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdes;

    if (lineSide)
    {
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
    }
    else
    {
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
    }

    *failedDfeSerdesBitmapPtr       = 0;
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        POLLING_ATTEMPT_VOID(mcdSerdesAdaptiveStatusGet(
            &mcdDevDb, serdes, &rxStatus));
        if (rxStatus != MCD_TUNE_PASS)
        {
            *failedDfeSerdesBitmapPtr |= (1 << i);
            mcdFwPortCtrlPollingStat.stopAdaptiveFailed[serdes] ++;
        }
    }
}

void mcdFwPortCtrlPollingLinkCheckDfeStateByMaskGet
(
    IN   MCD_U32         portIndex,
    IN   int             lineSide,
    IN   MCD_U16         serdesBitmap,
    IN   MCD_DFE_STATUS  dfeStatusMask,
    IN   MCD_DFE_STATUS  dfeStatusMaskPattern,
    OUT  MCD_U16         *resultDfeSerdesBitmapPtr
)
{
    MCD_U16/*MCD_DFE_STATUS */                dfeStatus;
    MCD_U32                        i;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdes;

    if (lineSide)
    {
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
    }
    else
    {
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
    }

    *resultDfeSerdesBitmapPtr = 0;
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        POLLING_ATTEMPT_VOID(mcdSerdesDfeStatusGet(
            &mcdDevDb, serdes, &dfeStatus));
        if ((dfeStatus & dfeStatusMask) == dfeStatusMaskPattern)
        {
            *resultDfeSerdesBitmapPtr |= (1 << i);
        }
    }
}

void mcdFwReducedRxTrainingStart
(
    IN MCD_U16     serdesBitMap,
    IN MCD_U8      calibrationMode
)
{
    MCD_U32 serdesNum;
    MCD_U16 numOfSerdes = POLLING_NUM_OF_SERDES_LINE_CNS;

    for (serdesNum =0 ; serdesNum< numOfSerdes; serdesNum++)
    {
        if (((1 << serdesNum) & (serdesBitMap)) == 0)
        {
            continue;
        }
        else
        {
            POLLING_ATTEMPT_VOID(mcdSerdesBypassCtleTuneStart(&mcdDevDb, (MCD_U8)serdesNum, (MCD_CTLE_CALIBRATION_MODE_E)calibrationMode, MCD_SPEED_NA));
        }
    }
}

void mcdFwNoPpmModeClockSelect
(
    IN MCD_U8     serdesBitMap,
    IN MCD_U16    hostSerdesSpeed,
    IN MCD_U32    chan,
    IN MCD_U32    slice,
    IN MCD_BOOL   enable
)
{
    MCD_U32 serdesNum;
    MCD_U16 numOfSerdes = POLLING_NUM_OF_SERDES_HOST_CNS;

    for (serdesNum =0 ; serdesNum< numOfSerdes; serdesNum++)
    {
        if (((1 << serdesNum) & (serdesBitMap)) == 0)
        {
            continue;
        }
        else
        {
            POLLING_ATTEMPT_VOID(mcdSerdesNoPpmModeSet(&mcdDevDb, serdesNum + POLLING_NUM_OF_SERDES_HOST_CNS , hostSerdesSpeed , chan, slice, enable));
        }
    }

}

void mcdFwPortCtrlPollingLinkCheckDfeStart
(
    IN   MCD_U32 portIndex,
    IN   int     lineSide,
    IN   MCD_U16 serdesBitmap,
    IN   MCD_DFE_MODE  dfeMode
)
{
    MCD_U32                        i;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdes;

    if (lineSide)
    {
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
    }
    else
    {
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
    }

    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        POLLING_ATTEMPT_VOID(mcdSerdesDfeConfig/*mcdSerdesDfeConfig_noSignalCheck*/(
            &mcdDevDb, serdes, dfeMode));
        mcdFwPortCtrlPollingStat.trainCount[serdes] ++;
        POLLING_ICAL_TRACE((
            "%s START port %d %s serdes %d \n",
            ((dfeMode == MCD_DFE_PCAL) ? "PCAL" : (dfeMode == MCD_DFE_ICAL) ?"ICAL" :
             (dfeMode == MCD_DFE_START_ADAPTIVE) ? "START_ADAPTIVE": "STOP_ADAPTIVE"),
            portIndex, (lineSide ? "LINE" : "HOST"), i));
    }
}

void mcdFwPortCtrlPollingLinkCheckRealCdrStateGet
(
    IN   MCD_U32 portIndex,
    IN   int     lineSide,
    IN   MCD_U16 serdesBitmap,
    OUT  MCD_U16 *notLockedCdrSerdesBitmapPtr
)
{
    MCD_U32                        i;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdes;
    MCD_BOOL                       serdesLocked;

    if (lineSide)
    {
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
    }
    else
    {
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
    }

    *notLockedCdrSerdesBitmapPtr = 0;
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        POLLING_ATTEMPT_VOID(mcdSerdesRealCdrStateGet(
            &mcdDevDb, serdes, &serdesLocked));
        if (serdesLocked == MCD_FALSE)
        {
            *notLockedCdrSerdesBitmapPtr |= (1 << i);
        }
    }
}

void mcdFwGetAutoTuneResult
(
    IN MCD_U32    portIndex,
    IN MCD_U16    serdesBitMap,
    IN int        lineSide,
    IN MCD_U16    *resultEyeOpenBitMapPtr,
    IN MCD_U16    *resultConfIntBitMapPtr
)
{
    MCD_U32 serdesNum, serdes;
    MCD_U16 numOfSerdes = 8;
    MCD_AUTO_TUNE_RESULTS results;
    MCD_U32 minEyeTh = MCD_MIN_EYE_TH;
    MCD_U32 maxEyeTh = MCD_MAX_EYE_TH;
    MCD_U32 lowLfTh = MCD_LF_LOW_TH;
    MCD_U32 highLfTh = MCD_LF_HIGH_TH;
    MCD_U32 hfTh = MCD_HF_TH;

    if (resultEyeOpenBitMapPtr != NULL)
    {
        *resultEyeOpenBitMapPtr = 0;
    }
    if (resultConfIntBitMapPtr != NULL)
    {
        *resultConfIntBitMapPtr = 0;
    }

    for (serdesNum =0 ; serdesNum< numOfSerdes; serdesNum++)
    {
        if (((1 << serdesNum) & (serdesBitMap)) == 0)
        {
            continue;
        }
        else
        {
            if (lineSide)
            {
                serdes = serdesNum;
                minEyeTh = mcdFwPortCtrlPortState[portIndex].minEyeThreshold;
                maxEyeTh = mcdFwPortCtrlPortState[portIndex].maxEyeThreshold;
                lowLfTh = mcdFwPortCtrlPortState[portIndex].confidenceParams.lfLowThreshold;
                highLfTh = mcdFwPortCtrlPortState[portIndex].confidenceParams.lfHighThreshold;
                hfTh = mcdFwPortCtrlPortState[portIndex].confidenceParams.hfThreshold;
            }
            else
            {
                serdes = serdesNum + POLLING_NUM_OF_SERDES_HOST_CNS;
            }
            POLLING_ATTEMPT_VOID(mcdSerdesAutoTuneResult(&mcdDevDb, serdes ,&results));
            if (resultConfIntBitMapPtr != NULL)
            {
                 /* confidence interval validation */
                if ((results.HF <= hfTh) && ((results.LF <= highLfTh) && (results.LF >= lowLfTh))
                    && (results.EO > minEyeTh))
                {
                    *resultConfIntBitMapPtr |= (1 << serdesNum);
                }
                else
                {
                    mcdFwPortCtrlPollingStat.countConfidenceIntFailed[serdes]++;
                }
            }
            if (resultEyeOpenBitMapPtr != NULL)
            {
                /* EO WA validation */
                if ((results.EO > minEyeTh) && (results.EO < maxEyeTh))
                {
                    *resultEyeOpenBitMapPtr |= (1 << serdesNum);
                }
                else
                {
                    mcdFwPortCtrlPollingStat.countEoFailed[serdes]++;
                }
            }
        }
    }
}

void mcdFwRxCtleParamsSet
(
    IN MCD_U16    serdesBitMap,
    IN int        lineSide,
    IN MCD_U16    serdesSpeed
)
{
    MCD_U32 serdesNum;
    MCD_U16 numOfSerdes = 8, serdesOffset = 0;
    if (lineSide == 0)
    {
        serdesOffset = POLLING_NUM_OF_SERDES_HOST_CNS;
    }
    for (serdesNum =0 ; serdesNum< numOfSerdes; serdesNum++)
    {
        if (((1 << serdesNum) & (serdesBitMap)) == 0)
        {
            continue;
        }
        else
        {
            POLLING_ATTEMPT_VOID(mcdSerdesManualCtleConfig(&mcdDevDb, serdesNum + serdesOffset, mcdSerdesTxRxTuneParamsArray[serdesSpeed].dcGain,
                                            mcdSerdesTxRxTuneParamsArray[serdesSpeed].lowFrequency, mcdSerdesTxRxTuneParamsArray[serdesSpeed].highFrequency,
                                            mcdSerdesTxRxTuneParamsArray[serdesSpeed].bandWidth , mcdSerdesTxRxTuneParamsArray[serdesSpeed].loopBandwidth,
                                            mcdSerdesTxRxTuneParamsArray[serdesSpeed].squelch));
        }
    }
}

/**
* @internal mcdFwPortCtrlPcsLowSpeedAutoNegRestart function
* @endinternal
*
* @brief    Restarts Auto-neg state machine, if AN-done bit is not set for lower speed i.e 1G.
*
*/
void mcdFwPortCtrlPcsLowSpeedAutoNegRestart
(
    MCD_U32     portIndex
)
{
    MCD_U32     anEnableSts, anDoneSts;
    MCD_U32     mdioPort;
    MCD_U32     mask = 0x1;

    mdioPort = mcdFwPortCtrlPortState[portIndex].mdioPort;

    /* Read status of ANenable bit */
    POLLING_ATTEMPT_VOID(mcdLowSpeedANEnableGet(&mcdDevDb, mdioPort, MCD_LINE_SIDE, &anEnableSts));

    if (anEnableSts & mask)
    {
        /* Read status of ANdone bit */
        POLLING_ATTEMPT_VOID(mcdLowSpeedANDoneGet(&mcdDevDb, mdioPort, MCD_LINE_SIDE, &anDoneSts));

        if ((!anDoneSts) & mask)
        {
            /* Restart AN state machine. Note, this bit is self cleared */
            POLLING_ATTEMPT_VOID(mcdLowSpeedANRestart(&mcdDevDb, mdioPort, MCD_LINE_SIDE));
            POLLING_REPORT(
                ("mcdFwPortCtrlPcsLowSpeedAutoNegRestart: ANrestarted on Port %d\n", mdioPort));
        }
    }
}

void mcdFwPortCtrlPollingLinkCheckDfeStartIcal
(
    IN   MCD_U32  portIndex,
    IN   int      lineSide,
    IN   MCD_U16  serdesBitmap,
    IN   MCD_CTLE_CALIBRATION_MODE_E  calMode,
    IN   MCD_BOOL setCtle,
    IN   MCD_U16  serdesSpeed
)
{
    if (serdesSpeed == MCD_1_25G) {
         mcdFwPortCtrlPollingLinkCheckDfeStart(
             portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL_VSR);
    }
    else if (((calMode == MCD_OPTICAL_CTLE_BYPASS_CALIBRATION) ||
                (calMode == MCD_REDUCE_CALIBRATION)) && lineSide)
    {
        mcdFwReducedRxTrainingStart(serdesBitmap, (MCD_U8)calMode);
    }
    else /* Will be used for: MCD_DEFAULT_CALIBRATION & MCD_OPTICAL_CALIBRATION */
    {
        if (setCtle) {
            mcdFwRxCtleParamsSet(serdesBitmap, lineSide, serdesSpeed);
        }
        mcdFwPortCtrlPollingLinkCheckDfeStart(
            portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL);
    }
}

void mcdFwPortCtrlPollingLinkCheckVoltageOffsetGet
(
    IN   MCD_U32 portIndex,
    IN   int     lineSide,
    IN   MCD_U16 serdesBitmap,
    OUT  MCD_U16 *badVoltageOffsetSerdesBitmapPtr
)
{
    MCD_U32                        i;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdes;
    MCD_BOOL                       voltageOffsetOk;

    if (lineSide)
    {
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
    }
    else
    {
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
    }

    *badVoltageOffsetSerdesBitmapPtr = 0;
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        POLLING_ATTEMPT_VOID(mcdSerdesDataVoltageOffsetCheck(
            &mcdDevDb, serdes, &voltageOffsetOk));
        if (voltageOffsetOk == MCD_FALSE)
        {
            *badVoltageOffsetSerdesBitmapPtr |= (1 << i);
            mcdFwPortCtrlPollingStat.countICalFailed[serdes] ++;
        }
    }
}

void mcdFwPortCtrlPollingLinkCheckOtherSideReset
(
    IN   MCD_U32   portIndex,
    IN   int       lineSide,
    IN   MCD_BOOL  reset
)
{
    MCD_U32                        i;
    MCD_U16                        serdesBitmap;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdes;

    if (lineSide == 0) /* other side values */
    {
        serdesBitmap   = mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap;
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
    }
    else
    {
        serdesBitmap   = mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap;
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
    }
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        POLLING_ATTEMPT_VOID(mcdSerdesResetImpl(
            &mcdDevDb, serdes, MCD_FALSE, reset, MCD_FALSE));
    }
}



void mcdFwPortCtrlPollingLinkCheckSignalOnPrimaryState
(
    IN   MCD_U32 portIndex,
    IN   int     lineSide,
    OUT  MCD_U16 *fullSerdesBitmapPtr,
    OUT  MCD_U16 *signalOnSerdesBitmapPtr,
    OUT  MCD_U16 *signalChangedSerdesBitmapPtr
)
{
    MCD_U32                        i, data;
    int                            signal_on;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS;
    MCD_U16                        serdesBitmap, serdes;
    MCD_U16                        signalOnSerdesBitmap;
    MCD_U16                        signalChangedSerdesBitmap;
    MCD_U16                        *savedSignalOnSerdesBitmapPtr;
    MCD_U32 slice;
    MCD_U32 phySerdesNum;

    /* get slice number */
    slice = portIndex/MCD_MAX_CHANNEL_NUM_PER_SLICE;

    if (lineSide)
    {
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
        serdesBitmap   = mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap;
        savedSignalOnSerdesBitmapPtr   = &(mcdFwPortCtrlPortState[portIndex].lineSignalOnSerdesBitmap);
    }
    else
    {
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
        serdesBitmap   = mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap;
        savedSignalOnSerdesBitmapPtr   = &(mcdFwPortCtrlPortState[portIndex].hostSignalOnSerdesBitmap);
    }

    signalOnSerdesBitmap = 0;
    /* check SIGNAL_OK_LIVE in any state */
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((MCD_U32)serdes,phySerdesNum);

        /*  Check Signal OK Live */
        mcdHwGetPhyRegField32bit(
            &mcdDevDb, DONT_CARE, MCD_SERDES_Core_Status(phySerdesNum), 4, 1, &data);
        signal_on = data;
        mcdFwPortCtrlPollingStat.signalCheckCount[serdes] ++;
        if (0 == signal_on)
        {
            mcdFwPortCtrlPollingStat.signalCheckFailCount[serdes] ++;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(
                lineSide, POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF, portIndex));
            POLLING_TRACE
                (("mcdFwPortCtrlPollingLinkCheckSignalOn: SIGNAL_OFF %s port %d lane %d sbus %d\n",
                 (lineSide ? "LINE" : "HOST"), portIndex, i, serdesStateArr[i].sbus_addr));
            continue;
        }
        signalOnSerdesBitmap |= (1 << i);
    }

    /* check signal change interrupts in any state */
    signalChangedSerdesBitmap = 0;
    for (i = 0; (i < numOfSerdes); i++)
    {
        if (((1 << i) & serdesBitmap) == 0)
        {
            continue;
        }
        serdes = serdesStateArr[i].sbus_addr - 1;
        MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC(serdes,phySerdesNum);
        /* check signal_OK change interrupt - Core status bit[4] change interrupt */
        mcdHwGetPhyRegField32bit(&mcdDevDb, DONT_CARE, MCD_SERDES_Interrupt0_cause(phySerdesNum), 5, 1, &data);
        if (data)
        {   /* inerrupt occured */
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(
                lineSide, POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY, portIndex));
            POLLING_TRACE
                (("mcdFwPortCtrlPollingLinkCheckSignalOn: SIGNAL_CHANGED %s port %d lane %d sbus %d\n",
                 (lineSide ? "LINE" : "HOST"), portIndex, i, serdesStateArr[i].sbus_addr));
            signalChangedSerdesBitmap |= (1 << i);
        }
    }

    /* save state */
    *savedSignalOnSerdesBitmapPtr  = (signalOnSerdesBitmap | signalChangedSerdesBitmap);
    /* output parameters */
    *fullSerdesBitmapPtr           = serdesBitmap;
    *signalOnSerdesBitmapPtr       = signalOnSerdesBitmap;
    *signalChangedSerdesBitmapPtr  = signalChangedSerdesBitmap;
}

/**
* @internal mcdFwPortCtrlPollingStatisticsPerPortClear function
* @endinternal
*
* @brief   Clear all the polling statistics DB per specific portIndex
*
* @param[in] portIndex
*
* @retval 0                        - void
*/
void mcdFwPortCtrlPollingStatisticsPerPortClear
(
    IN   MCD_U32 portIndex
)
{
    int i;

    mcdFwPortCtrlPollingStat.lineLastUpTime[portIndex] = 0;
    mcdFwPortCtrlPollingStat.hostLastUpTime[portIndex] = 0;
    mcdFwPortCtrlPollingStat.lineLongestUpTime[portIndex] = 0;
    mcdFwPortCtrlPollingStat.hostLongestUpTime[portIndex] = 0;
    mcdFwPortCtrlPollingStat.linePcsCheckCount[portIndex] = 0;
    mcdFwPortCtrlPollingStat.linePcsCheckFailCount[portIndex] = 0;
    mcdFwPortCtrlPollingStat.hostPcsCheckCount[portIndex] = 0;
    mcdFwPortCtrlPollingStat.hostPcsCheckFailCount[portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalOffToCheckStable[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalOffToCheckStable[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalCheckStableToOff[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalCheckStableToOff[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalCheckStableToOn[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalCheckStableToOn[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalOnToOff[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.countSignalOnToOff[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.minSignalOnTime[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.minSignalOnTime[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.maxSignalOffToCheckTime[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.maxSignalOffToCheckTime[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.maxSignalOffToOnTime[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.maxSignalOffToOnTime[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.maxSignalOtherSideOnToBothSidesOnTime[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.maxSignalOtherSideOnToBothSidesOnTime[1][portIndex] = 0;
    mcdFwPortCtrlPollingStat.minSignalOnBothSidesTime[portIndex] = 0;
    mcdFwPortCtrlPollingStat.registedSignalLastStates[0][portIndex] = 0;
    mcdFwPortCtrlPollingStat.registedSignalLastStates[1][portIndex] = 0;
#ifdef  MCD_AP_STATE_STATUS_LOG
    /* AP statistics */
    mcdFwPortCtrlPollingStat.registedApSmLog[portIndex] = 0;
    mcdFwPortCtrlPollingStat.registedApSmLogType[portIndex] = 0;
    mcdFwPortCtrlPollingStat.registedPortSmLog[portIndex] = 0;
    mcdFwPortCtrlPollingStat.registedPortSmLogType[portIndex] = 0;
#endif  /*MCD_AP_STATE_STATUS_LOG*/

    for (i = 0; (i < 8); i++)
    {
        if (((1 << i) & mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap) == 0)
            continue;

        mcdFwPortCtrlPollingStat.signalCheckCount[i] = 0;
        mcdFwPortCtrlPollingStat.signalCheckFailCount[i] = 0;
        mcdFwPortCtrlPollingStat.trainCount[i] = 0;
        mcdFwPortCtrlPollingStat.trainFailCount[i] = 0;
        mcdFwPortCtrlPollingStat.countICalFailed[i] = 0;
        mcdFwPortCtrlPollingStat.countEoFailed[i] = 0;
        mcdFwPortCtrlPollingStat.stopAdaptiveFailed[i] = 0;
        mcdFwPortCtrlPollingStat.countConfidenceIntFailed[i] = 0;
    }
    for (i = 0; (i < 8); i++)
    {
        if (((1 << i) & mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap) == 0)
            continue;

        mcdFwPortCtrlPollingStat.signalCheckCount[i + 8] = 0;
        mcdFwPortCtrlPollingStat.signalCheckFailCount[i + 8] = 0;
        mcdFwPortCtrlPollingStat.trainCount[i + 8] = 0;
        mcdFwPortCtrlPollingStat.trainFailCount[i + 8] = 0;
        mcdFwPortCtrlPollingStat.countICalFailed[i + 8] = 0;
        mcdFwPortCtrlPollingStat.countEoFailed[i + 8] = 0;
        mcdFwPortCtrlPollingStat.stopAdaptiveFailed[i + 8] = 0;
        mcdFwPortCtrlPollingStat.countConfidenceIntFailed[i + 8] = 0;
    }
    
}


void mcdFwPortCtrlPollingStatisticsPerPort
(
    IN   MCD_U32 portIndex,
    IN   int     lineSide,
    IN   POLLING_PORT_LINK_CHECK_STAGE oldLinkCheckStage,
    IN   POLLING_PORT_LINK_CHECK_STAGE newLinkCheckStage
)
{
    MCD_U32   tickTime;
    MCD_U32   *newTickTimePtr;
    MCD_U32   *oldTickTimePtr;
    MCD_U32   *linkUpStartTimePtr;
    MCD_U32   *linkUpLastTimePtr;
    MCD_U32   *linkUpLongestTimePtr;
    MCD_U16   serdesBitmap;
    MCD_U16   resultSerdesBitmap;
    MCD_U32   tickTimeDiff, tickTimeDiff1;
    MCD_U32   tickTime1;
    POLLING_PORT_LINK_CHECK_STAGE otherSideLinkCheckStage;
    MCD_U32   otherSideToOnTime;

    serdesBitmap = lineSide
        ? mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap
        : mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap;

    tickTime = xTaskGetTickCount();

    switch (newLinkCheckStage)
    {
        default:
        case POLLING_PORT_LINK_CHECK_STAGE_START:
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF:
            newTickTimePtr = lineSide
                ? &(mcdFwPortCtrlPortState[portIndex].lineLastToOffTime)
                : &(mcdFwPortCtrlPortState[portIndex].hostLastToOffTime);
            break;
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY:
            newTickTimePtr = lineSide
                ? &(mcdFwPortCtrlPortState[portIndex].lineLastToCheckStabilityTime)
                : &(mcdFwPortCtrlPortState[portIndex].hostLastToCheckStabilityTime);
            break;
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON:
            newTickTimePtr = lineSide
                ? &(mcdFwPortCtrlPortState[portIndex].lineLastToOnTime)
                : &(mcdFwPortCtrlPortState[portIndex].hostLastToOnTime);
            break;
    }

    if (newLinkCheckStage != oldLinkCheckStage)
    {
        *newTickTimePtr = tickTime;
    }

    switch (oldLinkCheckStage)
    {
        default:
        case POLLING_PORT_LINK_CHECK_STAGE_START:
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF:
            oldTickTimePtr = lineSide
                ? &(mcdFwPortCtrlPortState[portIndex].lineLastToOffTime)
                : &(mcdFwPortCtrlPortState[portIndex].hostLastToOffTime);
            break;
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY:
            oldTickTimePtr = lineSide
                ? &(mcdFwPortCtrlPortState[portIndex].lineLastToCheckStabilityTime)
                : &(mcdFwPortCtrlPortState[portIndex].hostLastToCheckStabilityTime);
            break;
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON:
            oldTickTimePtr = lineSide
                ? &(mcdFwPortCtrlPortState[portIndex].lineLastToOnTime)
                : &(mcdFwPortCtrlPortState[portIndex].hostLastToOnTime);
            break;
    }

    /* per port counters */

    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF)
        && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY))
    {
        mcdFwPortCtrlPollingStat.countSignalOffToCheckStable[lineSide][portIndex] ++;
        MCD_STAT_SIGNAL_INSERT(lineSide, portIndex, MCD_STAT_OFF_TO_CHK_STABLE);
    }
    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY)
        && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF))
    {
        mcdFwPortCtrlPollingStat.countSignalCheckStableToOff[lineSide][portIndex] ++;
        MCD_STAT_SIGNAL_INSERT(lineSide, portIndex, MCD_STAT_CHK_STABLE_TO_OFF);
    }
    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY)
        && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
    {
        mcdFwPortCtrlPollingStat.countSignalCheckStableToOn[lineSide][portIndex] ++;
        MCD_STAT_SIGNAL_INSERT(lineSide, portIndex, MCD_STAT_CHK_STABLE_TO_ON);
    }
    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON)
        && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF))
    {
        mcdFwPortCtrlPollingStat.countSignalOnToOff[lineSide][portIndex] ++;
        MCD_STAT_SIGNAL_INSERT(lineSide, portIndex, MCD_STAT_ON_TO_OFF);
    }

    /* LINK UP Time */

    linkUpStartTimePtr = lineSide
        ? &(mcdFwPortCtrlPortState[portIndex].lineLinkUpStartTime)
        : &(mcdFwPortCtrlPortState[portIndex].hostLinkUpStartTime);

    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF)
        || (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF))
    {
        if (*linkUpStartTimePtr == 0)
        {
            mcdFwPortCtrlPollingLinkCheckDfeStateByMaskGet(
                portIndex, lineSide, serdesBitmap,
                (MCD_DFE_ICAL_PENDING | MCD_DFE_PCAL_PENDING) /*dfeStatusMask*/,
                (MCD_DFE_ICAL_PENDING | MCD_DFE_PCAL_PENDING) /*dfeStatusMaskPattern*/,
                &resultSerdesBitmap);
            if (resultSerdesBitmap == 0)
            {
                *linkUpStartTimePtr = tickTime;
            }
        }
    }

    if ((oldLinkCheckStage != POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON)
        || (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
    {
        if (*linkUpStartTimePtr != 0)
        {
            linkUpLastTimePtr = lineSide
                ? &(mcdFwPortCtrlPollingStat.lineLastUpTime[portIndex])
                : &(mcdFwPortCtrlPollingStat.hostLastUpTime[portIndex]);
            linkUpLongestTimePtr = lineSide
                ? &(mcdFwPortCtrlPollingStat.lineLongestUpTime[portIndex])
                : &(mcdFwPortCtrlPollingStat.hostLongestUpTime[portIndex]);
            *linkUpLastTimePtr = mcdFwPortCtrlPortGetTimeDiff(tickTime, *linkUpStartTimePtr);
            if (*linkUpLongestTimePtr < *linkUpLastTimePtr)
            {
                *linkUpLongestTimePtr = *linkUpLastTimePtr;
            }
            /* reset start time */
            *linkUpStartTimePtr = 0;
        }
    }

    /* times */
    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF)
        && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY))
    {
        tickTimeDiff = mcdFwPortCtrlPortGetTimeDiff(*newTickTimePtr, *oldTickTimePtr);
        if (mcdFwPortCtrlPollingStat.maxSignalOffToCheckTime[lineSide][portIndex]
            < tickTimeDiff)
        {
            mcdFwPortCtrlPollingStat.maxSignalOffToCheckTime[lineSide][portIndex]
                = tickTimeDiff;
        }
    }
    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY)
        && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
    {
        tickTime1 = lineSide
            ? mcdFwPortCtrlPortState[portIndex].lineLastToOffTime
            : mcdFwPortCtrlPortState[portIndex].hostLastToOffTime;
        tickTimeDiff = mcdFwPortCtrlPortGetTimeDiff(*newTickTimePtr, tickTime1);
        if (mcdFwPortCtrlPollingStat.maxSignalOffToOnTime[lineSide][portIndex]
            < tickTimeDiff)
        {
            mcdFwPortCtrlPollingStat.maxSignalOffToOnTime[lineSide][portIndex]
                = tickTimeDiff;
        }
    }
    if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON)
        && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF))
    {
        tickTimeDiff = mcdFwPortCtrlPortGetTimeDiff(*newTickTimePtr, *oldTickTimePtr);
        if ( (mcdFwPortCtrlPollingStat.minSignalOnTime[lineSide][portIndex] == 0)
            || (mcdFwPortCtrlPollingStat.minSignalOnTime[lineSide][portIndex]
            > tickTimeDiff))
        {
            mcdFwPortCtrlPollingStat.minSignalOnTime[lineSide][portIndex]
                = tickTimeDiff;
        }
    }

    /* both sides statistics */
    otherSideLinkCheckStage = lineSide
        ? mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage
        : mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage;

    if (otherSideLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON)
    {
        if ((oldLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON)
            && (newLinkCheckStage != POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
        {
            tickTimeDiff = mcdFwPortCtrlPortGetTimeDiff(
                tickTime, mcdFwPortCtrlPortState[portIndex].hostLastToOnTime);
            tickTimeDiff1 = mcdFwPortCtrlPortGetTimeDiff(
                tickTime, mcdFwPortCtrlPortState[portIndex].lineLastToOnTime);
            if (tickTimeDiff < tickTimeDiff1)
            {
                tickTimeDiff = tickTimeDiff1;
            }
            if ((mcdFwPortCtrlPollingStat.minSignalOnBothSidesTime[portIndex]
                > tickTimeDiff)
                || (mcdFwPortCtrlPollingStat.minSignalOnBothSidesTime[portIndex] == 0))
            {
                mcdFwPortCtrlPollingStat.minSignalOnBothSidesTime[portIndex]
                    = tickTimeDiff;
            }
        }

        if ((oldLinkCheckStage != POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON)
            && (newLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
        {
            otherSideToOnTime = lineSide
                ? mcdFwPortCtrlPortState[portIndex].hostLastToOnTime
                : mcdFwPortCtrlPortState[portIndex].lineLastToOnTime;
            tickTimeDiff = mcdFwPortCtrlPortGetTimeDiff(tickTime, otherSideToOnTime);
            if (mcdFwPortCtrlPollingStat.maxSignalOtherSideOnToBothSidesOnTime[lineSide][portIndex]
                < tickTimeDiff)
            {
                mcdFwPortCtrlPollingStat.maxSignalOtherSideOnToBothSidesOnTime[lineSide][portIndex]
                    = tickTimeDiff;
            }
        }
    }
}

void mcdFwPortCtrlPollingLinkCheckSignalOnAtCaseOff
(
    IN  MCD_U32 portIndex,
    IN  int     lineSide,
    IN  MCD_U16 fullSerdesBitmap,
    IN  MCD_U16 signalOnSerdesBitmap,
    IN  MCD_U16 signalChangedSerdesBitmap
)
{
    MCD_U16                        signalOffSerdesBitmap;
    POLLING_PORT_LINK_CHECK_STAGE  *linkCheckStagePtr;
    MCD_U32                        *signalUnstableStartTickTimePtr;

    if (lineSide)
    {
        linkCheckStagePtr              = &(mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage);
        signalUnstableStartTickTimePtr = &(mcdFwPortCtrlPortState[portIndex].lineSignalUnstableStartTickTime);
    }
    else
    {
        linkCheckStagePtr              = &(mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage);
        signalUnstableStartTickTimePtr = &(mcdFwPortCtrlPortState[portIndex].hostSignalUnstableStartTickTime);
    }

    signalOffSerdesBitmap = fullSerdesBitmap & (~ (signalOnSerdesBitmap | signalChangedSerdesBitmap));

    if (signalOffSerdesBitmap)
    {
        /* SIGNAL_OFF state not changed */
        return;
    }

    /* SIGNAL_OFF => SIGNAL_CHECK_STABILITY */
    *signalUnstableStartTickTimePtr = xTaskGetTickCount();
    *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY;
}

/* restart ICAL fail of previous ICAL or on bad DVOS or on out-of-threshold */
/* amount of OFF => CHECK_STABILITY switcings                               */
/* rechecks SIGNAL and SIGNAL_CHANGED                                       */
/* optionally restarts ICAL if found problems and turns to OFF              */
/* returns GT_TRUE - contilnue to ON, GT_FALSE - stay at CHECK_STABILITY or to OFF */
MCD_BOOL mcdFwPortCtrlPollingLinkCheckSignalCommonCheckStabilityToOff
(
    IN  MCD_U32              portIndex,
    IN  int                  lineSide,
    IN  PRV_SIGNAL_STATE_ENT retuneTuneSerdesTypes,
    IN  PRV_SIGNAL_STATE_ENT toOffTuneSerdesTypes
)
{
    MCD_U32                        i;
    POLLING_PORT_LINK_CHECK_STAGE  *linkCheckStagePtr;
    MCD_U16                        signalOff, signalChanged;
    MCD_U16                        serdesBitmap, serdesSpeed;
    MCD_U16                        signalOnSerdesBitmap, signalChangedSerdesBitmap;
    MCD_U16                        workBitmap0, workBitmap1, recalSerdesBitmap;
    MCD_U16                        retuneSerdesBitmap, toOfTuneSerdesBitmap;
    MCD_U16                        iCalStarted;
    MCD_CTLE_CALIBRATION_MODE_E    calMode = MCD_DEFAULT_CALIBRATION;
    MCD_U32                        *signalUnstableStartTickTimePtr;
    MCD_U32                        tickTime;

    if (lineSide)
    {
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage);
        signalUnstableStartTickTimePtr = &(mcdFwPortCtrlPortState[portIndex].lineSignalUnstableStartTickTime);
        serdesSpeed = mcdFwPortCtrlPortState[portIndex].lineSerdesSpeed;
        calMode = mcdFwPortCtrlPortState[portIndex].calibrationMode;
    }
    else
    {
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage);
        signalUnstableStartTickTimePtr = &(mcdFwPortCtrlPortState[portIndex].hostSignalUnstableStartTickTime);
        serdesSpeed = mcdFwPortCtrlPortState[portIndex].hostSerdesSpeed;
    }


    /* check counter of OFF => CHECK_STABILITY time                     */
    /* maybe too early to recheck signal - measure time after last ICAL */
    tickTime = xTaskGetTickCount();
    if (mcdFwPortCtrlPortGetTimeDiff(tickTime, *signalUnstableStartTickTimePtr) <
        POLLING_PORTS_MAX_UNSTABLE_SIGNAL_TICK_TIME)
    {
        POLLING_ICAL_TRACE((
            "CHECK_STABILITY DELAY port %d %s \n",
            portIndex, (lineSide ? "LINE" : "HOST")));
        /* This condition should prevent the case of flapping between  stability to OFF states*/
        /* stay in CHECK_STABILITY without ICAL */
        return MCD_FALSE;
    }
    *signalUnstableStartTickTimePtr = tickTime;

    /* additional signal and interrupt check */

    signalOff = 0;
    signalChanged = 0;
    for (i = 0; (i < 3); i++)
    {
        mcdFwPortCtrlPollingLinkCheckSignalOnPrimaryState(
            portIndex,  lineSide,
            &serdesBitmap, &signalOnSerdesBitmap, &signalChangedSerdesBitmap);
        if (serdesBitmap != signalOnSerdesBitmap)
        {
            signalOff = 1;
        }
        if (signalChangedSerdesBitmap != 0)
        {
            signalChanged = 1;
        }
    }

    if ((signalOff == 0) && (signalChanged == 0))
    {
        /* caller will switch to signal on */
        return MCD_TRUE;
    }

    retuneSerdesBitmap = prvMcdFwPortCtrlPollingTuneSerdesBmpGet(
        retuneTuneSerdesTypes, serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);
    POLLING_REPORT(
        ("SIGNAL_CHECK_STABILITY SIGNAL_OFF OR SIGNAL_CHANGE %s port %d retuneSerdesBitmap 0x%X\n",
         (lineSide ? "LINE" : "HOST"), portIndex, retuneSerdesBitmap));

    iCalStarted = 0;
    recalSerdesBitmap = 0;
    mcdFwPortCtrlPollingLinkCheckRealCdrStateGet(
        portIndex, lineSide, retuneSerdesBitmap, &workBitmap0);
    retuneSerdesBitmap &= (~ workBitmap0);
    if (workBitmap0) /* lanes with bad CDR */
    {
        POLLING_REPORT(
            ("SIGNAL_OFF CDR_NOT_LOCKED %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
        iCalStarted = 1; /* forbidden to start ICAL */
    }
    mcdFwPortCtrlPollingLinkCheckDfeStateGet(
        portIndex, lineSide, retuneSerdesBitmap,
        &workBitmap0/*notCompletedDfeSerdesBitmapPtr*/, &workBitmap1/*failedDfeSerdesBitmapPtr*/);
    retuneSerdesBitmap &= (~ workBitmap0);
    recalSerdesBitmap  |= workBitmap1; /* lanes with bad completed DFI */
    if (workBitmap0) /* lanes with not completed DFI */
    {
        POLLING_REPORT(
            ("SIGNAL_OFF TUNE_NOT_COMPLITED %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
        iCalStarted = 1; /* present not completed ICAL */
    }
    if (workBitmap1) /* lanes with bad completed DFI */
    {
        POLLING_REPORT(
            ("SIGNAL_OFF TUNE_FAILED %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
    }
    mcdFwPortCtrlPollingLinkCheckVoltageOffsetGet(
        portIndex, lineSide, retuneSerdesBitmap,
        &workBitmap0/*badVoltageOffsetSerdesBitmapPtr*/);
    recalSerdesBitmap  |= workBitmap0;
    if (workBitmap0) /* lanes bad voltage offset */
    {
        POLLING_REPORT(
            ("SIGNAL_OFF BAD_VOLTAGE_OFFSET %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
        iCalStarted = 1; /* forbidden to start ICAL */
    }

    /* restart ICAL on relevant lanes */
    /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
    {
        mcdFwReducedRxTrainingStart(recalSerdesBitmap, calMode);
    }
    else
    {
        mcdFwPortCtrlPollingLinkCheckDfeStart(
            portIndex, lineSide, recalSerdesBitmap, MCD_DFE_ICAL);
    }*/

    mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, recalSerdesBitmap,calMode,MCD_FALSE, serdesSpeed);
    /* SIGNAL_CHECK_STABILITY => SIGNAL_OFF */
    if (iCalStarted == 0)
    {
        toOfTuneSerdesBitmap = prvMcdFwPortCtrlPollingTuneSerdesBmpGet(
            toOffTuneSerdesTypes, serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);
        /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
        {
            mcdFwReducedRxTrainingStart(toOfTuneSerdesBitmap, calMode);
        }
        else
        {
            mcdFwPortCtrlPollingLinkCheckDfeStart(
                portIndex, lineSide, toOfTuneSerdesBitmap, MCD_DFE_ICAL);
        }*/
        mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, toOfTuneSerdesBitmap, calMode, MCD_FALSE, serdesSpeed);
    }
    *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
    return MCD_FALSE;
}

/* SIGNAL_CHECK_STABILITY ==> SIGNAL_ON                           */
/* called when all serdes lanes have SIGNAL and no SIGNAL_CHANGED */
void mcdFwPortCtrlPollingLinkCheckSignalCommonCheckStabilityToOn
(
    IN  MCD_U32              portIndex,
    IN  int                  lineSide,
    IN  int                  toOnICalSerdesTypes,
    IN  int                  toOnPCalSerdesTypes,
    IN  MCD_BOOL             retimerMode,
    OUT MCD_BOOL             *needRfResetPtr
)
{
    MCD_U32                        i;
    POLLING_SERDES_STATE_STC       *serdesStateArr;
    MCD_U16                        numOfSerdes = POLLING_NUM_OF_PORTS, serdesSpeed;
    MCD_U16                        serdesBitmap, serdes;
    POLLING_PORT_LINK_CHECK_STAGE  *linkCheckStagePtr;
    MCD_U32                        data;
    MCD_AUTO_TUNE_STATUS           rxStatus;
    MCD_U16                        numOfDelays;
    MCD_U16                        iCalFailed;
    MCD_BOOL                       retimerWithAp = MCD_FALSE;
    MCD_CTLE_CALIBRATION_MODE_E    calMode = MCD_DEFAULT_CALIBRATION;

    if (lineSide)
    {
        serdesBitmap = mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap;
        serdesStateArr = mcdFwPortCtrlLineSerdesState;
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage);
        serdesSpeed = mcdFwPortCtrlPortState[portIndex].lineSerdesSpeed;
        calMode = mcdFwPortCtrlPortState[portIndex].calibrationMode;
    }
    else
    {
        serdesBitmap = mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap;
        serdesStateArr = mcdFwPortCtrlHostSerdesState;
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage);
        retimerWithAp = mcdFwPortCtrlPortState[portIndex].retimerWithAp;
        serdesSpeed = mcdFwPortCtrlPortState[portIndex].hostSerdesSpeed;
    }

    *needRfResetPtr = MCD_TRUE;
    if (toOnICalSerdesTypes)
    {
        /* restart ICAL on relevant lanes */
        /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
        {
            mcdFwReducedRxTrainingStart(serdesBitmap, calMode);
        }
        else
        {
            mcdFwPortCtrlPollingLinkCheckDfeStart(
                portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL);
        }*/
        mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, serdesBitmap, calMode, MCD_FALSE, serdesSpeed);

        numOfDelays = 0;
        iCalFailed = 0;
        for (i = 0; (i < numOfSerdes); i++)
        {
            if (((1 << i) & serdesBitmap) == 0)
            {
                continue;
            }
            serdes = serdesStateArr[i].sbus_addr - 1;
            /* wait for ICal end                               */
            /* 20 seconds maximum for all sedes lanes together */
            /* success not checked here                        */
            for (; (numOfDelays < 2000); numOfDelays++)
            {
                mcdWrapperMsSleep(10);
                POLLING_ATTEMPT_VOID(mcdSerdesRxAutoTuneStatusGet(
                    &mcdDevDb, serdes, &rxStatus));
                if (rxStatus == MCD_TUNE_NOT_COMPLITED) continue;
                if (rxStatus != MCD_TUNE_PASS)
                {
                    iCalFailed = 1;
                    /* start ICAL */
                    if (serdesSpeed == MCD_1_25G) {
                        mcdFwPortCtrlPollingLinkCheckDfeStart(
                            portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL_VSR);
                    }
                    else if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                    {
                        mcdFwReducedRxTrainingStart(serdesBitmap, calMode);
                    }
                    else
                    {
                        POLLING_ATTEMPT_VOID(mcdSerdesDfeConfig/*mcdSerdesDfeConfig_noSignalCheck*/(
                            &mcdDevDb, serdes, MCD_DFE_ICAL));
                    }
                    mcdFwPortCtrlPollingStat.trainCount[serdes] ++;
                    POLLING_ICAL_TRACE((
                        "ICAL START port %d %s serdes %d \n",
                        portIndex, (lineSide ? "LINE" : "HOST"), i));
                    break; /* pass to the next serdes */
                }
            }
            /* clear signal_OK change interrupt - Core status bit[4] change interrupt */
            mcdHwGetPhyRegField32bit(
                &mcdDevDb, DONT_CARE, MCD_SERDES_Interrupt0_cause(serdes), 5, 1, &data);
        }
        if ((numOfDelays >= 2000) || iCalFailed)
        {
            /* timeout reached or iCal failed */
            *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
            return ;
        }
    }
    if (toOnPCalSerdesTypes)
    {
        if (serdesSpeed == MCD_1_25G)
        {
        POLLING_REPORT(
            ("PCAL NOT NEEDED %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
        }
        else
        {
            /* call adaptive start */
            mcdFwPortCtrlPollingLinkCheckDfeStart(
                portIndex, lineSide, serdesBitmap, MCD_DFE_START_ADAPTIVE/*MCD_DFE_PCAL*/);
        }

        /*rrr retimer ON state*/
        if ((retimerMode == MCD_TRUE) && (retimerWithAp== MCD_TRUE)) {
            /*printf("\n*************************\nON state retimerWithAp %d port %d \n*************************\n", retimerWithAp, portIndex);*/
            /*in case of AP with retimer RF_RESET will be called after AP port is done*/
            /*Start AP port*/
             mvHwsIpcApRetimerRequestSet(MCD_IPC_PORT_AP_ENABLE_MSG, portIndex,
                                 mcdFwPortCtrlPortState[portIndex].apAdvMode,
                                 mcdFwPortCtrlPortState[portIndex].apLaneNum,
                                 mcdFwPortCtrlPortState[portIndex].apOptions,
                                 mcdFwPortCtrlPortState[portIndex].apPolarityVector,
                                 retimerWithAp,
                                 mcdFwPortCtrlPortState[portIndex].lineRxRemapVector,
                                 mcdFwPortCtrlPortState[portIndex].lineTxRemapVector);
             *needRfResetPtr = MCD_FALSE;
        }

    }
    *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON;
}

void mcdFwPortCtrlPollingLinkCheckSignalOnRetimer(MCD_U32 portIndex, int lineSide)
{
    MCD_U16                        serdesBitmap;
    MCD_U16                        signalOnSerdesBitmap;
    MCD_U16                        signalChangedSerdesBitmap;
    POLLING_PORT_LINK_CHECK_STAGE  *linkCheckStagePtr;
    POLLING_PORT_LINK_CHECK_STAGE  prevLinkCheckStage;
    MCD_U16                        signalOff, signalChanged;
    MCD_U16                        tuneSerdesBitmap, serdesSpeed;
    MCD_BOOL                       continue_to_on;
    MCD_BOOL                       retimerWithAp = MCD_FALSE, needRfReset;
    MCD_U16                        numOfSerdes = 0;
    MCD_U16                        serdes, i;
    MCD_U16                        resultEyeOpenBitMap = 0;
    MCD_U16                        resultConfIntBitMap = 0;
    MCD_U16                        failSerdesBitmap = 0;
    MCD_U16                        calMode = MCD_DEFAULT_CALIBRATION;

    if (lineSide)
    {
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage);
        serdesSpeed   = mcdFwPortCtrlPortState[portIndex].lineSerdesSpeed;
        calMode = mcdFwPortCtrlPortState[portIndex].calibrationMode;
    }
    else
    {
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage);
        numOfSerdes    = POLLING_NUM_OF_SERDES_HOST_CNS;
        retimerWithAp = mcdFwPortCtrlPortState[portIndex].retimerWithAp;
        serdesSpeed = mcdFwPortCtrlPortState[portIndex].hostSerdesSpeed;
    }

    prevLinkCheckStage   = *linkCheckStagePtr;
    mcdFwPortCtrlPollingLinkCheckSignalOnPrimaryState(
        portIndex, lineSide,
        &serdesBitmap, &signalOnSerdesBitmap, &signalChangedSerdesBitmap);
    signalOff = (signalOnSerdesBitmap ? 0 : 1);
    signalChanged = (signalChangedSerdesBitmap ? 1 : 0);

    if (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF)
    {
        mcdFwPortCtrlPollingLinkCheckSignalOnAtCaseOff(
            portIndex,  lineSide, serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);
        return;
    }

    if (mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] != 0)
    {
            mcdFwPortCtrlPollingLinkCheckTrainingStatusGet(portIndex,lineSide,serdesBitmap,&failSerdesBitmap);
            if (failSerdesBitmap != 0)
            {
                /* check stop adaptive state */
                return;
            }
            else
            {
                mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] = 0;
                if (signalOff || signalChanged)
                {
                    tuneSerdesBitmap = prvMcdFwPortCtrlPollingTuneSerdesBmpGet(
                            mcdFwPortCtrlPollingLinkCheckSignalOnPcs_OnToOffICal,
                            serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);
                    /* restart ICAL on relevant lanes */
                    /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                    {
                        mcdFwReducedRxTrainingStart(tuneSerdesBitmap, calMode);
                    }
                    else
                    {
                        mcdFwPortCtrlPollingLinkCheckDfeStart(
                            portIndex, lineSide, tuneSerdesBitmap, MCD_DFE_ICAL);
                    }*/
                    mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, tuneSerdesBitmap, calMode, MCD_FALSE, serdesSpeed);
                }
                else
                {
                    /* write default CTLE values */
                    /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                    {
                        mcdFwReducedRxTrainingStart(serdesBitmap, calMode);
                    }
                    else
                    {
                        mcdFwRxCtleParamsSet(serdesBitmap,lineSide,serdesSpeed);
                        mcdFwPortCtrlPollingLinkCheckDfeStart(
                            portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL);
                    }*/
                    mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, serdesBitmap, calMode, MCD_TRUE, serdesSpeed);
                }
                *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
                return;
            }
    }

    /* SIGNAL_ON or SIGNAL_CHECK_STABILITY */

    if ((signalOff == 0) && (signalChanged == 0) &&
        (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
    {
        if (mcdFwPortCtrlPortState[portIndex].calibrationAlgo != MCD_CONFIDENCE_INTERVAL_ALGO)
            {
            mcdFwPortCtrlPortEoCounter[lineSide][portIndex]++;
            if (mcdFwPortCtrlPortEoCounter[lineSide][portIndex] >= 10)
            {
                mcdFwPortCtrlPortEoCounter[lineSide][portIndex] = 0;
                mcdFwGetAutoTuneResult(portIndex, serdesBitmap, lineSide, &resultEyeOpenBitMap, NULL);
                if (serdesBitmap != resultEyeOpenBitMap)
                {
                    mcdFwPortCtrlPollingLinkCheckOtherSideReset(portIndex, lineSide, MCD_TRUE);
                    if (serdesSpeed != MCD_1_25G)
                    {
                       /* call adaptive stop - retimer*/
                        mcdFwPortCtrlPollingLinkCheckDfeStart(
                            portIndex, lineSide, serdesBitmap, MCD_DFE_STOP_ADAPTIVE);
                        mcdFwPortCtrlPollingLinkCheckTrainingStatusGet(portIndex,lineSide,serdesBitmap,&failSerdesBitmap);
                        if (failSerdesBitmap != 0)
                        {
                            /* check stop adaptive state */
                            mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] = 1;
                            return;
                        }
                    }
                     /* write default CTLE values */
                    /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                    {
                        mcdFwReducedRxTrainingStart(serdesBitmap, calMode);
                    }
                    else
                    {
                        mcdFwRxCtleParamsSet(serdesBitmap,lineSide,serdesSpeed);
                        mcdFwPortCtrlPollingLinkCheckDfeStart(
                            portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL);
                    }*/
                    mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, serdesBitmap, calMode, MCD_TRUE, serdesSpeed);
                    *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
                }
            }
        }
        /* SIGNAL_ON state not changed */
        return;
    }

    if (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY)
    {
        /* restart ICAL fail of previous ICAL or on bad DVOS or on out-of-threshold */
        /* amount of OFF => CHECK_STABILITY switcings                               */
        /* rechecks SIGNAL and SIGNAL_CHANGED                                       */
        /* optionally restarts ICAL if found problems and turns to OFF              */
        continue_to_on = mcdFwPortCtrlPollingLinkCheckSignalCommonCheckStabilityToOff(
            portIndex,  lineSide,
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityDvos,
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityToOffICal);
        if (continue_to_on == MCD_FALSE)
        {
            return;
        }
        if (*linkCheckStagePtr == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF)
        {
            return;
        }

        /* start confidence interval algorithm for optics */
        if (lineSide && (calMode == MCD_OPTICAL_CALIBRATION))
        {
            if (mcdFwPortCtrlPortState[portIndex].calibrationAlgo != MCD_EYE_OPENING_ALGO)
            {
                mcdFwGetAutoTuneResult(portIndex, serdesBitmap, lineSide, NULL, &resultConfIntBitMap);
                if (serdesBitmap != resultConfIntBitMap)
                    {
                        resultConfIntBitMap = (~(resultConfIntBitMap)&(serdesBitmap));
                       /* run iCal on bad hf lf serdes's */
                       mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, resultConfIntBitMap, calMode, MCD_TRUE, serdesSpeed);
                       *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
                       return;
                       /* move to OFF state */
                    }
            }
        }


        /* SIGNAL_CHECK_STABILITY => SIGNAL_ON */
        mcdFwPortCtrlPollingLinkCheckSignalCommonCheckStabilityToOn(
            portIndex, lineSide,
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityToOnICal,
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityToOnPCal, MCD_TRUE, &needRfReset);

        if (needRfReset)
        {
            mcdFwPortCtrlPollingLinkCheckOtherSideReset(
                portIndex, lineSide, MCD_FALSE);
        }
        return;
    }

    if ((signalOff || signalChanged) &&
        (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
    {
        /* SIGNAL_ON => SIGNAL_OFF */
        mcdFwPortCtrlPortEoCounter[lineSide][portIndex] = 0;
        mcdFwPortCtrlPollingLinkCheckOtherSideReset(
            portIndex, lineSide, MCD_TRUE);

        /* get bitmap of tuned serdes lanes */
        tuneSerdesBitmap = prvMcdFwPortCtrlPollingTuneSerdesBmpGet(
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_OnToOffICal,
            serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);

        if (serdesSpeed != MCD_1_25G)
        {
            /* call adaptive stop - retimer*/
            mcdFwPortCtrlPollingLinkCheckDfeStart(
                portIndex, lineSide, tuneSerdesBitmap, MCD_DFE_STOP_ADAPTIVE);
        }

        /*rrr retimer OFF state */
        if (retimerWithAp) {
            /*printf("\n*************************\nOFF state retimerWithAp %d port %d \n*************************\n", retimerWithAp, portIndex);*/
            /*in case of AP with retimer disable AP port and switch back the clocks*/
            POLLING_ATTEMPT_VOID(mvHwsIpcApRetimerRequestSet(MCD_IPC_PORT_AP_DISABLE_MSG, portIndex,
                                 mcdFwPortCtrlPortState[portIndex].apAdvMode,
                                 mcdFwPortCtrlPortState[portIndex].apLaneNum,
                                 mcdFwPortCtrlPortState[portIndex].apOptions,
                                 mcdFwPortCtrlPortState[portIndex].apPolarityVector,
                                 retimerWithAp,
                                 mcdFwPortCtrlPortState[portIndex].lineRxRemapVector,
                                 mcdFwPortCtrlPortState[portIndex].lineTxRemapVector));
            for (i = 0; (i < numOfSerdes); i++)
            {

                if (((1 << i) & tuneSerdesBitmap) == 0)
                {
                    continue;
                }
                serdes = mcdFwPortCtrlHostSerdesState[i].sbus_addr - 1;
                POLLING_ATTEMPT_VOID(mcdSerdesClockRegular(&mcdDevDb, serdes, mcdFwPortCtrlPortState[portIndex].hostSerdesSpeed));
            }
        }

        if (serdesSpeed != MCD_1_25G)
        {
            mcdFwPortCtrlPollingLinkCheckTrainingStatusGet(portIndex,lineSide,tuneSerdesBitmap,&failSerdesBitmap);
            if (failSerdesBitmap != 0)
            {
            	/* check stop adaptive state */
	            mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] = 1;
                return;
            }
            /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
            {
                mcdFwReducedRxTrainingStart(tuneSerdesBitmap, calMode);
            }
            else
            {
                mcdFwPortCtrlPollingLinkCheckDfeStart(
                    portIndex, lineSide, tuneSerdesBitmap, MCD_DFE_ICAL);
            }*/
        }
        mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, tuneSerdesBitmap, calMode, MCD_FALSE, serdesSpeed);

        *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
        return;
    }
}

/* linkCheckStage will be evaluated from START up to FAILED or STABLE_ON   */
/* ALIVE_ON stage will be kept for later calls if the time yet later than  */
/* signalLastClearTime less than LINK_STABLE_TIME                          */
void mcdFwPortCtrlPollingLinkCheckRetimer(MCD_U32 portIndex, int lineSide)
{
    MCD_U32                         delay_time_passed;
    MCD_U32                         *delayDurationPtr;
    MCD_U32                         *delayStartPtr;
    MCD_U32                         current_time;
    POLLING_PORT_LINK_CHECK_STAGE   *linkCheckStagePtr;
    POLLING_PORT_LINK_CHECK_STAGE   prevLinkCheckStage;


    if (portIndex >= POLLING_NUM_OF_PORTS)
    {
        POLLING_ERROR((
            "mcdFwPortCtrlPollingLinkCheckRetimer portIndex %d wrong\n", portIndex));
        return;
    }
    if (mcdFwPortCtrlPortState[portIndex].stage == POLLING_PORT_STAGE_INVALID)
    {
        POLLING_ERROR((
            "mcdFwPortCtrlPollingLinkCheckRetimer STAGE is INVALID\n"));
        return;
    }

    if (lineSide)
    {
        delayDurationPtr = &(mcdFwPortCtrlPortState[portIndex].lineDelayDuration);
        delayStartPtr    = &(mcdFwPortCtrlPortState[portIndex].lineDelayStartTime);
        linkCheckStagePtr          = &(mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage);
        mvPortCtrlLogAdd(PORT_GENERAL_LOG(lineSide, POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF, portIndex));
    }
    else
    {
        delayDurationPtr = &(mcdFwPortCtrlPortState[portIndex].hostDelayDuration);
        delayStartPtr    = &(mcdFwPortCtrlPortState[portIndex].hostDelayStartTime);
        linkCheckStagePtr          = &(mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage);
    }
    if (*delayDurationPtr)
    {
        /* wait enable */
        current_time = mcdGetHwTimestamp(&mcdDevDb);
        delay_time_passed = mcdFwPortCtrlPollingIsTimePassed(
            *delayStartPtr/*time_start*/,
            current_time/*time_cur*/,
            *delayDurationPtr/*time_delay*/);
        if (delay_time_passed == 0) return;
        /* disable wait at the end of time */
        *delayDurationPtr = 0;
    }

    prevLinkCheckStage = *linkCheckStagePtr;

    switch (*linkCheckStagePtr)
    {
        case POLLING_PORT_LINK_CHECK_STAGE_START:
            *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer(portIndex, lineSide);
            *delayStartPtr     = mcdGetHwTimestamp(&mcdDevDb);
            *delayDurationPtr  = POLLING_PORTS_SIGNAL_CHECK_TIME;
            break;
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF:
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY:
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON:
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer(portIndex, lineSide);
            *delayStartPtr     = mcdGetHwTimestamp(&mcdDevDb);
            *delayDurationPtr  = POLLING_PORTS_SIGNAL_CHECK_TIME;
            break;
        default:
            break;
    }
    mcdFwPortCtrlPollingStatisticsPerPort(
        portIndex, lineSide, prevLinkCheckStage, *linkCheckStagePtr);
    if (prevLinkCheckStage != *linkCheckStagePtr)
    {
        switch (*linkCheckStagePtr)
        {
            case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF:
                POLLING_REPORT(
                    ("RETIMER SIGNAL_OFF %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
                break;
            case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY:
                POLLING_REPORT(
                    ("RETIMER SIGNAL_CHECK_STABILITY %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
                break;
            case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON:
                POLLING_REPORT(
                    ("RETIMER SIGNAL_ON %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
                break;
            default:
                break;
        }
    }
}

/* read PCS operation bit  */
/* returns 1 - on, 0 - off */
MCD_U32 mcdFwPortCtrlPollingLinkPcsAliveOnGet(MCD_U32 portIndex, int lineSide)
{
    MCD_U32 hostRegAddr;
    MCD_U32 lineRegAddr;
    MCD_U32 bitIndex;
    MCD_U32 slice, chan;
    MCD_U32 hostData, lineData;
    MCD_U32 mdioPort;

    mdioPort = mcdFwPortCtrlPortState[portIndex].mdioPort;
    slice = mdioPort / MCD_MAX_CHANNEL_NUM_PER_SLICE;
    chan  = mdioPort % MCD_MAX_CHANNEL_NUM_PER_SLICE;
    if (mcdFwPortCtrlPortState[portIndex].lineSerdesSpeed != MCD_1_25G)
    {
        /* high speed */
        hostRegAddr = UMAC_MCPCS_BASE_R_and_10GBASE_T_PCS_Status1(slice,chan);
        lineRegAddr = UMAC_MCPCS_LINE_REG_ADDR(hostRegAddr);
        bitIndex    = 12;
    }
    else
    {
        /* low speed */
        hostRegAddr = UMAC_LSMCPCS_sts(slice,chan);
        lineRegAddr = UMAC_LSMCPCS_LINE_REG_ADDR(hostRegAddr);
        bitIndex    = 1;
    }
    if (lineSide)
    {
        /* line side */
        if (MCD_OK != mcdHwGetPhyRegField32bit(
            &mcdDevDb, mdioPort, lineRegAddr, bitIndex, 1, &lineData))
        {
            POLLING_ERROR((
                "mcdFwPortCtrlPollingLinkPcsAliveOnGet mcdHwGetPhyRegField32bit error \n"));
            return 0;
        }
        mcdFwPortCtrlPollingStat.linePcsCheckCount[portIndex] ++;
        if (lineData == 0)
        {
            mcdFwPortCtrlPollingStat.linePcsCheckFailCount[portIndex] ++;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(lineSide, POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF, portIndex));
            POLLING_TRACE(
                ("mcdFwPortCtrlPollingLinkPcsAliveOnGet: STAGE_PCS_ALIVE_OFF LINE port %d\n",
                 portIndex));
            return 0;
        }
        mvPortCtrlLogAdd(PORT_GENERAL_LOG(lineSide,POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON, portIndex));
    }
    else
    {
        /* default - updated on success */
        /* host side */
        if (MCD_OK != mcdHwGetPhyRegField32bit(
            &mcdDevDb, mdioPort, hostRegAddr, bitIndex, 1, &hostData))
        {
            POLLING_ERROR((
                "mcdFwPortCtrlPollingLinkPcsAliveOnGet mcdHwGetPhyRegField32bit error \n"));
            return 0;
        }
        mcdFwPortCtrlPollingStat.hostPcsCheckCount[portIndex] ++;
        if (hostData == 0)
        {
            /* update statistic */
            mcdFwPortCtrlPollingStat.hostPcsCheckFailCount[portIndex] ++;
            mvPortCtrlLogAdd(PORT_GENERAL_LOG(lineSide, POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF, portIndex));
            POLLING_TRACE(
                ("mcdFwPortCtrlPollingLinkPcsAliveOnGet: STAGE_PCS_ALIVE_OFF HOST port %d\n",
                 portIndex));
            return 0;
        }
        mvPortCtrlLogAdd(PORT_GENERAL_LOG(lineSide, POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON, portIndex));
    }
    return 1;
}

void mcdFwPortCtrlPollingLinkCheckSignalOnPcs(MCD_U32 portIndex, int lineSide)
{
    MCD_U16                        serdesBitmap;
    MCD_U16                        signalOnSerdesBitmap;
    MCD_U16                        signalChangedSerdesBitmap;
    MCD_U16                        noPpmMode;
    POLLING_PORT_LINK_CHECK_STAGE  *linkCheckStagePtr;
    POLLING_PORT_LINK_CHECK_STAGE  prevLinkCheckStage;
    MCD_U16                        signalOff, signalChanged;
    MCD_U32                        tickTime, mdioPort, slice, chan;
    MCD_U16                        tuneSerdesBitmap;
    MCD_U32                        pcsOperationOn;
    MCD_U16                        *pcsOperationOffCountPtr;
    MCD_U32                        *pcsOperationOffTickTimePtr;
    MCD_BOOL                       continue_to_on, needRfReset;
    MCD_U16                        hostSerdesSpeed, noPpmHostBitMap;
    MCD_CTLE_CALIBRATION_MODE_E    calMode = MCD_DEFAULT_CALIBRATION;
    MCD_U16                        serdesSpeed;
    MCD_U16                        resultEyeOpenBitMap = 0;
    MCD_U16                        resultConfIntBitMap = 0;
    MCD_U16                        failSerdesBitmap = 0;


    if (lineSide)
    {
        serdesBitmap   = mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap;
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage);
        pcsOperationOffCountPtr    = &(mcdFwPortCtrlPortState[portIndex].linePcsOperationOffCount);
        pcsOperationOffTickTimePtr = &(mcdFwPortCtrlPortState[portIndex].linePcsOperationOffTickTime);
        noPpmMode                  = mcdFwPortCtrlPortState[portIndex].noPpmMode;
        calMode                    = mcdFwPortCtrlPortState[portIndex].calibrationMode;
        serdesSpeed                = mcdFwPortCtrlPortState[portIndex].lineSerdesSpeed;
    }
    else
    {
        serdesBitmap   = mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap;
        linkCheckStagePtr = &(mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage);
        pcsOperationOffCountPtr     = &(mcdFwPortCtrlPortState[portIndex].hostPcsOperationOffCount);
        pcsOperationOffTickTimePtr  = &(mcdFwPortCtrlPortState[portIndex].hostPcsOperationOffTickTime);
        noPpmMode                   = mcdFwPortCtrlPortState[portIndex].noPpmMode;
        serdesSpeed                 = mcdFwPortCtrlPortState[portIndex].hostSerdesSpeed;
    }

    hostSerdesSpeed = mcdFwPortCtrlPortState[portIndex].hostSerdesSpeed; /* for noPpmMode host SD speed is needed */
    noPpmHostBitMap = mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap;

    mdioPort = mcdFwPortCtrlPortState[portIndex].mdioPort;
    slice = mdioPort / MCD_MAX_CHANNEL_NUM_PER_SLICE;
    chan  = mdioPort % MCD_MAX_CHANNEL_NUM_PER_SLICE;
    prevLinkCheckStage   = *linkCheckStagePtr;
    mcdFwPortCtrlPollingLinkCheckSignalOnPrimaryState(
        portIndex, lineSide,
        &serdesBitmap, &signalOnSerdesBitmap, &signalChangedSerdesBitmap);
    signalOff = (signalOnSerdesBitmap ? 0 : 1);
    signalChanged = (signalChangedSerdesBitmap ? 1 : 0);

    if (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF)
    {
        /* start trasmiting local faults from host side to prevent link flapping on MAC system side
           applicable only for optics calibration , not incuding low speeds*/
        if ((lineSide) && (calMode == MCD_OPTICAL_CALIBRATION))
        {
            POLLING_ATTEMPT_VOID(mcdForceOutputLocalFaults(&mcdDevDb, mdioPort, MCD_HOST_SIDE, serdesSpeed, MCD_TRUE));
        }
        mcdFwPortCtrlPollingLinkCheckSignalOnAtCaseOff(
            portIndex,  lineSide, serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);
        return;
    }

    /* TBD - check if the EO restarts should run on all serdes or only on bad EO serdes, handle no-ppm case also*/
    if (mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] != 0)
        {
                mcdFwPortCtrlPollingLinkCheckTrainingStatusGet(portIndex,lineSide,serdesBitmap,&failSerdesBitmap);
                if (failSerdesBitmap != 0)
                {
                    /* check stop adaptive state */
                    return;
                }
                else
                {
                    mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] = 0;
                    if (signalOff || signalChanged)
                    {
                        tuneSerdesBitmap = prvMcdFwPortCtrlPollingTuneSerdesBmpGet(
                                mcdFwPortCtrlPollingLinkCheckSignalOnPcs_OnToOffICal,
                                serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);
                        /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                        {
                            mcdFwReducedRxTrainingStart(tuneSerdesBitmap, calMode);
                        }
                        else
                        {
                            mcdFwPortCtrlPollingLinkCheckDfeStart(
                                portIndex, lineSide, tuneSerdesBitmap, MCD_DFE_ICAL);
                        }*/
                        mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, tuneSerdesBitmap, calMode, MCD_FALSE, serdesSpeed);
                    }
                    else
                    {
                        /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                        {
                            mcdFwReducedRxTrainingStart(serdesBitmap, calMode);
                        }
                        else
                        {
                            mcdFwRxCtleParamsSet(serdesBitmap,lineSide,serdesSpeed);
                            mcdFwPortCtrlPollingLinkCheckDfeStart(
                                portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL);
                        }*/
                        mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, serdesBitmap, calMode, MCD_TRUE, serdesSpeed);
                    }
                    *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
                    return;
                }

        }

    /* SIGNAL_ON or SIGNAL_CHECK_STABILITY */

    if ((signalOff == 0) && (signalChanged == 0) &&
        (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
    {
        if (mcdFwPortCtrlPortState[portIndex].calibrationAlgo != MCD_CONFIDENCE_INTERVAL_ALGO)
        {
            mcdFwPortCtrlPortEoCounter[lineSide][portIndex]++;
            if (mcdFwPortCtrlPortEoCounter[lineSide][portIndex] >= 10)
            {
                mcdFwPortCtrlPortEoCounter[lineSide][portIndex] = 0;
                mcdFwGetAutoTuneResult(portIndex, serdesBitmap, lineSide, &resultEyeOpenBitMap, NULL);
                if (serdesBitmap != resultEyeOpenBitMap)
                {
                    if (serdesSpeed != MCD_1_25G)
                    {
                        /* call adaptive stop - retimer*/
                        mcdFwPortCtrlPollingLinkCheckDfeStart(
                        portIndex, lineSide, serdesBitmap, MCD_DFE_STOP_ADAPTIVE);
                        mcdFwPortCtrlPollingLinkCheckTrainingStatusGet(portIndex,lineSide,serdesBitmap,&failSerdesBitmap);
                        if (failSerdesBitmap != 0)
                        {
                            /* check stop adaptive state */
                            mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] = 1;
                            return;
                        }
                    }
                   /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                    {
                        mcdFwReducedRxTrainingStart(serdesBitmap, calMode);
                    }
                    else
                    {
                        mcdFwRxCtleParamsSet(serdesBitmap,lineSide,serdesSpeed);
                        mcdFwPortCtrlPollingLinkCheckDfeStart(
                            portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL);
                    }*/
                    mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, serdesBitmap, calMode, MCD_TRUE, serdesSpeed);
                    *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
                    return;
                }
            }
        }

        /* Check if AN bit is set for 1.25G/P1_BaseX_SGMII, and restart-AN if 'andone' bit is not set */
        if ((lineSide) && (serdesSpeed == MCD_1_25G))
        {
            mcdFwPortCtrlPcsLowSpeedAutoNegRestart(portIndex);
        }

        /* SIGNAL_ON state will not be changed, but maybe PCAL needed */
        if (mcdFwPortCtrlPollingLinkCheckSignalOnPcs_OnToOnNoPcsOperationalPCal)
        {
            pcsOperationOn = mcdFwPortCtrlPollingLinkPcsAliveOnGet(portIndex, lineSide);
            if (pcsOperationOn)
            {
                /* PCS operation state is ON */
                *pcsOperationOffCountPtr = 0;
                return;
            }
            /* PCS operation state is OFF */
            if (*pcsOperationOffCountPtr > POLLING_PORTS_PCS_OPER_OFF_PCAL_MAX_COUNT)
            {
                /* PCAL started many times - not helps */
                /* SIGNAL_ON => SIGNAL_OFF */
                if (serdesSpeed != MCD_1_25G)
                {    /* call adaptive stop - retimer*/
                    mcdFwPortCtrlPollingLinkCheckDfeStart(
                    portIndex, lineSide, serdesBitmap, MCD_DFE_STOP_ADAPTIVE);
                    mcdFwPortCtrlPollingLinkCheckTrainingStatusGet(portIndex,lineSide,serdesBitmap,&failSerdesBitmap);
                    if (failSerdesBitmap != 0)
                    {
                        /* check stop adaptive state */
                        mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] = 1;
                        return;
                    }
                }
                /* restart ICAL on relevant lanes */
                /*if ((calMode != MCD_DEFAULT_CALIBRATION) && lineSide)
                {
                    mcdFwReducedRxTrainingStart(serdesBitmap, calMode);
                }
                else
                {
                    mcdFwPortCtrlPollingLinkCheckDfeStart(
                        portIndex, lineSide, serdesBitmap, MCD_DFE_ICAL);
                }*/
                mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, serdesBitmap, calMode, MCD_TRUE, serdesSpeed);

                *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
                /* SIGNAL_ON state not changed */
                return;
            }
            if (*pcsOperationOffCountPtr == POLLING_PORTS_PCS_OPER_OFF_PCAL_MAX_COUNT)
            {
                POLLING_REPORT(
                    ("mcdFwPortCtrlPollingLinkCheckSignalOnPcs: PCS_OPER_OFF_PCAL_MAX_COUNT reached\n"));
            }

            if (*pcsOperationOffCountPtr == 0)
            {
                /* save time only */
                tickTime = xTaskGetTickCount();
                *pcsOperationOffTickTimePtr = tickTime;
                (*pcsOperationOffCountPtr) = 1;
                return;
            }

            /* (*pcsOperationOffCountPtr > 0) */
            tickTime = xTaskGetTickCount();
            if (mcdFwPortCtrlPortGetTimeDiff(tickTime, *pcsOperationOffTickTimePtr) >=
                POLLING_PORTS_PCS_OPER_OFF_PCAL_TICK_TIME)
            {
                *pcsOperationOffTickTimePtr = tickTime;
                (*pcsOperationOffCountPtr) ++;
                /* call adaptive start - pcs*/
                if (serdesSpeed != MCD_1_25G)
                {
                    mcdFwPortCtrlPollingLinkCheckDfeStart(
                        portIndex, lineSide, serdesBitmap, MCD_DFE_START_ADAPTIVE/*MCD_DFE_PCAL*/);
                }
            }
        }
        return;
    }

    /* SIGNAL_CHECK_STABILITY => SIGNAL_OFF or no change */
    if (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY)
    {
        /* restart ICAL fail of previous ICAL or on bad DVOS or on out-of-threshold */
        /* amount of OFF => CHECK_STABILITY switcings                               */
        /* rechecks SIGNAL and SIGNAL_CHANGED                                       */
        /* optionally restarts ICAL if found problems and turns to OFF              */
        continue_to_on = mcdFwPortCtrlPollingLinkCheckSignalCommonCheckStabilityToOff(
            portIndex,  lineSide,
            mcdFwPortCtrlPollingLinkCheckSignalOnRetimer_CheckStabilityDvos,
            mcdFwPortCtrlPollingLinkCheckSignalOnPcs_CheckStabilityToOffICal);
        if (continue_to_on == MCD_FALSE)
        {
            return;
        }
        if (*linkCheckStagePtr == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF)
        {
            return;
        }

        /* start confidence interval algorithm for optics */
        if (lineSide && (calMode == MCD_OPTICAL_CALIBRATION))
        {
            if (mcdFwPortCtrlPortState[portIndex].calibrationAlgo != MCD_EYE_OPENING_ALGO)
            {
                mcdFwGetAutoTuneResult(portIndex, serdesBitmap, lineSide, NULL, &resultConfIntBitMap);
                if (serdesBitmap != resultConfIntBitMap)
                    {
                        resultConfIntBitMap = (~(resultConfIntBitMap)&(serdesBitmap));
                       /* run iCal on bad hf lf serdes's */
                       mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, resultConfIntBitMap, calMode, MCD_TRUE, serdesSpeed);
                       *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
                       return;
                       /* move to OFF state */
                    }
            }
        }

        /* SIGNAL_CHECK_STABILITY => SIGNAL_ON or to off */
        mcdFwPortCtrlPollingLinkCheckSignalCommonCheckStabilityToOn(
            portIndex, lineSide,
            mcdFwPortCtrlPollingLinkCheckSignalOnPcs_CheckStabilityToOnICal,
            mcdFwPortCtrlPollingLinkCheckSignalOnPcs_CheckStabilityToOnPCal, MCD_FALSE, &needRfReset);

        /* stop trasmiting local faults from host side to allow the MAC on system side
           to start receiving idles , not including 1G*/
        if ((lineSide) && (calMode == MCD_OPTICAL_CALIBRATION))
        {
            POLLING_ATTEMPT_VOID(mcdForceOutputLocalFaults(&mcdDevDb, mdioPort, MCD_HOST_SIDE, serdesSpeed, MCD_FALSE));
        }

        if ((lineSide != 0) && (noPpmMode != 0))
        {
            /* uses source clock from line */
            if (noPpmMode != 2 /* Rx Squelch Oper Mode */)
            {
                mcdFwNoPpmModeClockSelect(noPpmHostBitMap, hostSerdesSpeed, chan, slice, 1);
                mcdWrapperMsSleep(PRV_MCD_DELAY_NOPPM_CLOCK_SWITCH);
            }
            mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_Channel_MAC_Debug(slice,chan), 1, 1, 0);
            /* unreset digital reset */
            mcdFwPortCtrlPollingLinkCheckOtherSideReset(
                portIndex, lineSide, MCD_FALSE);
            if (noPpmMode == 2)
            {
                /* pcs software reset */
                mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_GLBL_Software_Reset(slice), 0, 4, (1 << chan));
                /* pcs software unreset */
                mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_GLBL_Software_Reset(slice), 0, 4, 0);
            }
        }
        /* PCAL on PCS Operational OFF counter */
        (*pcsOperationOffCountPtr) = 0;
        return;
    }

    if ((signalOff || signalChanged) &&
        (prevLinkCheckStage == POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON))
    {
        /* SIGNAL_ON => SIGNAL_OFF */
        mcdFwPortCtrlPortEoCounter[lineSide][portIndex] = 0;
        tuneSerdesBitmap = prvMcdFwPortCtrlPollingTuneSerdesBmpGet(
            mcdFwPortCtrlPollingLinkCheckSignalOnPcs_OnToOffICal,
            serdesBitmap, signalOnSerdesBitmap, signalChangedSerdesBitmap);

        if (serdesSpeed != MCD_1_25G)
        {
            /* call adaptive stop - pcs*/
            mcdFwPortCtrlPollingLinkCheckDfeStart(
                portIndex, lineSide, tuneSerdesBitmap, MCD_DFE_STOP_ADAPTIVE);
            mcdFwPortCtrlPollingLinkCheckTrainingStatusGet(portIndex,lineSide,tuneSerdesBitmap,&failSerdesBitmap);

            if (failSerdesBitmap != 0)
            {
                /* check stop adaptive state */
                mcdFwPortCtrlPortVerifyStopAdaptive[lineSide][portIndex] = 1;
                return;
            }
        }



        mcdFwPortCtrlPollingLinkCheckDfeStartIcal(portIndex, lineSide, tuneSerdesBitmap, calMode, MCD_FALSE, serdesSpeed);

        if ((lineSide != 0) && (noPpmMode != 0))
        {
            /*switch clock to local source clock */
            if (noPpmMode != 2 /* Rx Squelch Oper Mode */)
            {
                mcdFwNoPpmModeClockSelect(noPpmHostBitMap, hostSerdesSpeed, chan, slice, 0);
                mcdWrapperMsSleep(PRV_MCD_DELAY_NOPPM_CLOCK_SWITCH);
                mcdFwPortCtrlPollingLinkCheckOtherSideReset(portIndex, lineSide, MCD_FALSE);
            }
            if (noPpmMode == 2 /* rx squelch */ )
            {
                mcdFwPortCtrlPollingLinkCheckOtherSideReset(
                portIndex, lineSide, MCD_TRUE);
            }
            if (noPpmMode == 3)
            {
                mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_Channel_MAC_Debug(slice,chan), 1, 1, 1);
            }
        }
        *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
        return;
    }
}

/* linkCheckStage will be evaluated from START up to FAILED or STABLE_ON   */
/* ALIVE_ON stage will be kept for later calls if the time yet later than  */
/* signalLastClearTime less than LINK_STABLE_TIME                          */
void mcdFwPortCtrlPollingLinkCheckPcs(MCD_U32 portIndex, int lineSide)
{
    MCD_U32                         delay_time_passed;
    MCD_U32                         *delayDurationPtr;
    MCD_U32                         *delayStartPtr;
    MCD_U32                         current_time;
    POLLING_PORT_LINK_CHECK_STAGE   *linkCheckStagePtr;
    POLLING_PORT_LINK_CHECK_STAGE   prevLinkCheckStage;

    if (portIndex >= POLLING_NUM_OF_PORTS)
    {
        POLLING_ERROR((
            "mcdFwPortCtrlPollingLinkCheckPcs portIndex %d wrong\n", portIndex));
        return;
    }
    if (mcdFwPortCtrlPortState[portIndex].stage == POLLING_PORT_STAGE_INVALID)
    {
        POLLING_ERROR((
            "mcdFwPortCtrlPollingLinkCheckPcs STAGE is INVALID\n"));
        return;
    }

    if (lineSide)
    {
        delayDurationPtr = &(mcdFwPortCtrlPortState[portIndex].lineDelayDuration);
        delayStartPtr    = &(mcdFwPortCtrlPortState[portIndex].lineDelayStartTime);
        linkCheckStagePtr          = &(mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage);
        mvPortCtrlLogAdd(PORT_GENERAL_LOG(lineSide, POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF, portIndex));
    }
    else
    {
        delayDurationPtr = &(mcdFwPortCtrlPortState[portIndex].hostDelayDuration);
        delayStartPtr    = &(mcdFwPortCtrlPortState[portIndex].hostDelayStartTime);
        linkCheckStagePtr          = &(mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage);
    }
    if (*delayDurationPtr)
    {
        /* wait enable */
        current_time = mcdGetHwTimestamp(&mcdDevDb);
        delay_time_passed = mcdFwPortCtrlPollingIsTimePassed(
            *delayStartPtr/*time_start*/,
            current_time/*time_cur*/,
            *delayDurationPtr/*time_delay*/);
        if (delay_time_passed == 0) return;
        /* disable wait at the end of time */
        *delayDurationPtr = 0;
    }

    prevLinkCheckStage = *linkCheckStagePtr;

    switch (*linkCheckStagePtr)
    {
        case POLLING_PORT_LINK_CHECK_STAGE_START:
            *linkCheckStagePtr = POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF;
            mcdFwPortCtrlPollingLinkCheckSignalOnPcs(portIndex, lineSide);
            *delayStartPtr     = mcdGetHwTimestamp(&mcdDevDb);
            *delayDurationPtr  = POLLING_PORTS_SIGNAL_CHECK_TIME;
            break;
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF:
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY:
        case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON:
            mcdFwPortCtrlPollingLinkCheckSignalOnPcs(portIndex, lineSide);
            *delayStartPtr     = mcdGetHwTimestamp(&mcdDevDb);
            *delayDurationPtr  = POLLING_PORTS_SIGNAL_CHECK_TIME;
            break;
        default:
            break;
    }
    mcdFwPortCtrlPollingStatisticsPerPort(
        portIndex, lineSide, prevLinkCheckStage, *linkCheckStagePtr);
    if (prevLinkCheckStage != *linkCheckStagePtr)
    {
        switch (*linkCheckStagePtr)
        {
            case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_OFF:
                POLLING_REPORT(
                    ("PCS SIGNAL_OFF %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
                break;
            case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_CHECK_STABILITY:
                POLLING_REPORT(
                    ("PCS SIGNAL_CHECK_STABILITY %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
                break;
            case POLLING_PORT_LINK_CHECK_STAGE_SIGNAL_ON:
                POLLING_REPORT(
                    ("PCS SIGNAL_ON %s port %d \n", (lineSide ? "LINE" : "HOST"), portIndex));
                break;
            default:
                break;
        }
    }
}

void mcdFwPortCtrlPollingLinkCheckIdle(void)
{
    MCD_U32 portIndex;

    for (portIndex = 0; (portIndex < POLLING_NUM_OF_PORTS); portIndex++)
    {
        if (mcdFwPortCtrlPortState[portIndex].stage == POLLING_PORT_STAGE_INVALID)
        {
            continue;
        }
        if (mcdFwPortCtrlPortState[portIndex].retimerMode)
        {
            if (mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap != 0)
            {
                mcdFwPortCtrlPollingLinkCheckRetimer(portIndex, 0); /* host */
            }
            if (mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap != 0)
            {
                mcdFwPortCtrlPollingLinkCheckRetimer(portIndex, 1); /* line */
            }
        }
        else
        {
            if (mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap != 0)
            {
                mcdFwPortCtrlPollingLinkCheckPcs(portIndex, 0); /* host */
            }
            if (mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap != 0)
            {
                mcdFwPortCtrlPollingLinkCheckPcs(portIndex, 1); /* line */
            }
        }
    }
}
/**
* @internal mcdFwPortCtrlPollingLinkCheckEnable function
* @endinternal
*
* @brief   Include port to port set being monitored by
*         link status polling.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdFwPortCtrlPollingLinkCheckEnable(MCD_IPC_CTRL_MSG_STRUCT *msg, MCD_U32 enable)
{
    MCD_U32  portIndex, tickTime;
    MCD_IPC_POLLING_PORT_LINK_STATUS_START_STC *polling;
    MCD_IPC_POLLING_PORT_LINK_STATUS_STOP_STC  *pollingStop;

    if (enable)
    {
        polling = &(msg->msgData.pollingPortLinkStatusStart);
        POLLING_REPORT((
            "mcdFwPortCtrlPollingLinkCheckEnable enable port %d retimerMode %d  retimerWithAp %d\n",
            polling->mdioPort, polling->retimerMode, polling->retimerWithAp));
        POLLING_REPORT((
            "lineSpeed %d hostSpeed %d, lineBitmap 0x%X hostBitmap 0x%X \n",
             polling->lineSerdesSpeed, polling->hostSerdesSpeed,
             polling->lineSerdesBitmap,  polling->hostSerdesBitmap));
        for (portIndex = 0; (portIndex < POLLING_NUM_OF_PORTS); portIndex++)
        {
            if (mcdFwPortCtrlPortState[portIndex].mdioPort == polling->mdioPort) break;
        }
        if (portIndex >= POLLING_NUM_OF_PORTS) return MCD_FAIL;
        tickTime = xTaskGetTickCount();
        mcdFwPortCtrlPortState[portIndex].retimerMode      = polling->retimerMode;
        mcdFwPortCtrlPortState[portIndex].lineSerdesSpeed  = polling->lineSerdesSpeed;
        mcdFwPortCtrlPortState[portIndex].hostSerdesSpeed  = polling->hostSerdesSpeed;
        mcdFwPortCtrlPortState[portIndex].lineSerdesBitmap = polling->lineSerdesBitmap;
        mcdFwPortCtrlPortState[portIndex].hostSerdesBitmap = polling->hostSerdesBitmap;
        mcdFwPortCtrlPortState[portIndex].noPpmMode        = polling->noPpmMode;
        mcdFwPortCtrlPortState[portIndex].calibrationMode  = polling->calibrationMode;
        mcdFwPortCtrlPortState[portIndex].calibrationAlgo  = (MCD_CALIBRATION_ALGO_E)polling->calibrationAlgo;
        mcdFwPortCtrlPortState[portIndex].minEyeThreshold  = polling->minEyeThreshold;
        mcdFwPortCtrlPortState[portIndex].maxEyeThreshold  = polling->maxEyeThreshold;
        mcdFwPortCtrlPortState[portIndex].confidenceParams.lfLowThreshold = polling->lfLowThreshold;
        mcdFwPortCtrlPortState[portIndex].confidenceParams.lfHighThreshold = polling->lfHighThreshold;
        mcdFwPortCtrlPortState[portIndex].confidenceParams.hfThreshold = polling->hfThreshold;
        mcdFwPortCtrlPortState[portIndex].stage = POLLING_PORT_STAGE_INITIALIZED;
        mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage = POLLING_PORT_LINK_CHECK_STAGE_START;
        mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage = POLLING_PORT_LINK_CHECK_STAGE_START;
        mcdFwPortCtrlPortState[portIndex].lineSignalOnSerdesBitmap = polling->lineSerdesBitmap;
        mcdFwPortCtrlPortState[portIndex].hostSignalOnSerdesBitmap = polling->hostSerdesBitmap;
        mcdFwPortCtrlPortState[portIndex].linePcsOperationOffCount = 0;
        mcdFwPortCtrlPortState[portIndex].hostPcsOperationOffCount = 0;
        mcdFwPortCtrlPortState[portIndex].lineSignalUnstableStartTickTime = 0;
        mcdFwPortCtrlPortState[portIndex].hostSignalUnstableStartTickTime = 0;
        mcdFwPortCtrlPortState[portIndex].linePcsOperationOffTickTime = 0;
        mcdFwPortCtrlPortState[portIndex].hostPcsOperationOffTickTime = 0;
        mcdFwPortCtrlPortState[portIndex].hostLastToOffTime            = tickTime;
        mcdFwPortCtrlPortState[portIndex].lineLastToOffTime            = tickTime;
        mcdFwPortCtrlPortState[portIndex].hostLastToCheckStabilityTime = tickTime;
        mcdFwPortCtrlPortState[portIndex].lineLastToCheckStabilityTime = tickTime;
        mcdFwPortCtrlPortState[portIndex].hostLastToOnTime             = tickTime;
        mcdFwPortCtrlPortState[portIndex].lineLastToOnTime             = tickTime;
        mcdFwPortCtrlPortState[portIndex].apLaneNum = polling->laneNum;
        mcdFwPortCtrlPortState[portIndex].apAdvMode = polling->advMode;
        mcdFwPortCtrlPortState[portIndex].apOptions = polling->options;
        mcdFwPortCtrlPortState[portIndex].apPolarityVector = polling->polarityVector;
        mcdFwPortCtrlPortState[portIndex].retimerWithAp = polling->retimerWithAp;
        mcdFwPortCtrlPortState[portIndex].hostRxRemapVecotr = polling->hostRxRemapVector;
        mcdFwPortCtrlPortState[portIndex].hostTxRemapVecotr = polling->hostTxRemapVector;
        mcdFwPortCtrlPortState[portIndex].lineRxRemapVector = polling->lineRxRemapVector;
        mcdFwPortCtrlPortState[portIndex].lineTxRemapVector = polling->lineTxRemapVector;
        mcdDevDb.laneRemapCfg[portIndex/4].lineRxRemapVector = polling->lineRxRemapVector;
        mcdDevDb.laneRemapCfg[portIndex/4].hostRxRemapVector = polling->hostRxRemapVector;
        mcdDevDb.calibrationCfg[portIndex].calibrationMode = polling->calibrationMode;
        if (polling->lineRxRemapVector != 0)
        {
            mcdDevDb.laneRemapCfg[portIndex/4].lineRemapMode = MCD_LANE_REMAP_ENABLE;
        }
        if (polling->hostRxRemapVector != 0)
        {
            mcdDevDb.laneRemapCfg[portIndex/4].hostRemapMode = MCD_LANE_REMAP_ENABLE;
        }

        /* clear statistics */
        mcdFwPortCtrlPollingStatisticsPerPortClear(portIndex);
    }
    else
    {
        pollingStop = &(msg->msgData.pollingPortLinkStatusStop);
        POLLING_REPORT((
            "mcdFwPortCtrlPollingLinkCheckEnable disable port %d\n",
            pollingStop->mdioPort));
        for (portIndex = 0; (portIndex < POLLING_NUM_OF_PORTS); portIndex++)
        {
            if (mcdFwPortCtrlPortState[portIndex].mdioPort == pollingStop->mdioPort) break;
        }
        if (portIndex >= POLLING_NUM_OF_PORTS)
        {
            POLLING_REPORT((
                "mcdFwPortCtrlPollingLinkCheckEnable disable port - port not found in DB %d\n",
                pollingStop->mdioPort));
            return MCD_FAIL;
        }
        mcdFwPortCtrlPortState[portIndex].stage = POLLING_PORT_STAGE_INVALID;
        mcdFwPortCtrlPortState[portIndex].hostLinkCheckStage = POLLING_PORT_LINK_CHECK_STAGE_START;
        mcdFwPortCtrlPortState[portIndex].lineLinkCheckStage = POLLING_PORT_LINK_CHECK_STAGE_START;
        mcdFwPortCtrlPortState[portIndex].retimerWithAp = 0;
        mcdFwPortCtrlPortState[portIndex].calibrationMode = 0;
    }
    return MCD_OK;
}


/**
* @internal mvPortCtrlApHcdFound function
* @endinternal
*
* @brief   Callback to be called when resolution is found
*         1. Configure external serdes clock mux and data mux at line & host sides
*         2. Check if host RX PLL is ready
*         3. Switch clock (int 30 then int 0x5) at line & host sides
*         4. Wait 10 msec overall
*         5. Un_reset RF_RESET at line & host sides
* @param[in] devNum
* @param[in] portNum
* @param[in] portMode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvPortCtrlApHcdFound
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode
)
{
    MCD_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portNum].stats);
    MCD_U16 serdesLane,i ;
    MCD_U32 laneSpeed,numOfActLanes;
    MCD_BOOL changeClk = MCD_TRUE;
    MCD_U8 slice, chan;

    AP_PRINT_MCD_MAC((">>mvPortCtrlApHcdFound port:%d portMode:%d retimerMode:%d ppmMode:%d <<\n",portNum, portMode, mcdDevDb.apCfg[portNum].retimerMode,
                  mcdDevDb.apCfg[portNum].noPpmMode));
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb,portNum,portMode,&laneSpeed,&numOfActLanes));

    if ((mcdDevDb.apCfg[portNum].retimerMode))
    {
        apStats->hcdClbkStartTime = mvPortCtrlCurrentTs();
        AP_PRINT_MCD2_MAC((">>mvPortCtrlApHcdFound port:%d laneSpeed:%d numOfActLanes:%d<<\n",portNum, laneSpeed, numOfActLanes));
        for (i = 0; i < numOfActLanes; i++)
        {
            serdesLane = portNum + i;
            CHECK_STATUS(mcdSetSerdesLaneCfgForRetimerModePostSet(&mcdDevDb, portNum, serdesLane, laneSpeed, serdesLane, MCD_FALSE, changeClk));
            if (mcdClbkMode > 0)
            {
                serdesLane = portNum + i + MCD_HOST_SIDE_START_SERDES_NUMBER;
                CHECK_STATUS(mcdSetSerdesLaneCfgForRetimerModePostSet(&mcdDevDb, portNum, serdesLane, laneSpeed, serdesLane, MCD_FALSE, changeClk));
            }
        }
        mcdWrapperMsSleep(10);
        for (i = 0; i < numOfActLanes; i++)
        {
            serdesLane = portNum + i;
            CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane, MCD_FALSE, MCD_FALSE, MCD_FALSE));
            if (mcdClbkMode == 1)
            {
                serdesLane = portNum + i + MCD_HOST_SIDE_START_SERDES_NUMBER;
                CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane, MCD_FALSE, MCD_FALSE, MCD_FALSE));
            }
        }
        apStats->hcdClbkStopTime = mvPortCtrlCurrentTs();
    }

    if (mcdDevDb.apCfg[portNum].noPpmMode != 0)
    {
        slice = portNum / MCD_MAX_CHANNEL_NUM_PER_SLICE;
        chan  = portNum % MCD_MAX_CHANNEL_NUM_PER_SLICE;
        /* uses source clock from line */
        /*mcdFwNoPpmModeClockSelect(noPpmHostBitMap, hostSerdesSpeed, chan, slice, 1);*/

        if (mcdDevDb.apCfg[portNum].noPpmMode != 2 /* rx squelch */)
        {
            for (i = 0; i < numOfActLanes; i++)
            {

                serdesLane = portNum + MCD_HOST_SIDE_START_SERDES_NUMBER + i;
                CHECK_STATUS(mcdSerdesNoPpmModeSet(&mcdDevDb, serdesLane, laneSpeed , chan, slice, MCD_TRUE));
            }
            mcdWrapperMsSleep(PRV_MCD_DELAY_NOPPM_CLOCK_SWITCH);
        }

        mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_Channel_MAC_Debug(slice,chan), 1, 1, 0);

        /* unreset digital reset */
        /* mcdFwPortCtrlPollingLinkCheckOtherSideReset(portIndex, lineSide, MCD_FALSE);*/
        for (i = 0; (i < numOfActLanes); i++)
        {
            serdesLane = portNum + MCD_HOST_SIDE_START_SERDES_NUMBER + i;
            CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane, MCD_FALSE, MCD_FALSE, MCD_FALSE));
            if (mcdDevDb.apCfg[portNum].noPpmMode == 2)
            {
                /* pcs software reset */
                mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_GLBL_Software_Reset(slice), 0, 4, (1 << (chan)));
                /* pcs software unreset */
                mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_GLBL_Software_Reset(slice), 0, 4, 0);
            }
        }
    }
    return MCD_OK;
}


/**
* @internal mvPortCtrlApTrxTrainingEnd function
* @endinternal
*
* @brief   Callback to be called when Trx training finished
*         1.  Configure external serdes clock mux and data mux at host side
*         2.  Check if RX PLL to be ready
*         3.  Switch clock (int 30 then int 0x5) at host side
*         4.  Wait 10 msec overall
*         5.  Un_reset RF_RESET at host sides
* @param[in] devNum
* @param[in] portNum
* @param[in] portMode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvPortCtrlApTrxTrainingEnd
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode
)
{
    MCD_U16 serdesLane,i ;
    MCD_U32 laneSpeed,numOfActLanes;
    AP_PRINT_MCD_MAC((">>mvPortCtrlApTrxTrainingEnd port:%d portMode:%d<<\n",portNum, portMode));

    if (mcdDevDb.apCfg[portNum].retimerMode)
    {
        CHECK_STATUS(mcdGetPortParams(&mcdDevDb,portNum,portMode,&laneSpeed,&numOfActLanes));
        if (mcdClbkMode == 0)
        {

            for (i = 0 ; i < numOfActLanes; i++)
            {
                serdesLane = portNum + MCD_HOST_SIDE_START_SERDES_NUMBER + i;
                CHECK_STATUS(mcdSetSerdesLaneCfgForRetimerModePostSet(&mcdDevDb, portNum, serdesLane, laneSpeed, serdesLane, MCD_FALSE, MCD_TRUE));

            }
            mcdWrapperMsSleep(10);
            for (i = 0; (i < numOfActLanes); i++)
            {
                serdesLane = portNum + MCD_HOST_SIDE_START_SERDES_NUMBER + i;
                CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane, MCD_FALSE, MCD_FALSE, MCD_FALSE));
            }
            AP_PRINT_MCD2_MAC((">>do all<<\n"));
        }
        else if (mcdClbkMode == 2)
        {
            for (i = 0; (i < numOfActLanes); i++)
            {
                serdesLane = portNum + MCD_HOST_SIDE_START_SERDES_NUMBER + i;
                CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane, MCD_FALSE, MCD_FALSE, MCD_FALSE));
            }
            AP_PRINT_MCD2_MAC((">>do RF unreset<<\n"));
        }
        else
        {
            AP_PRINT_MCD2_MAC((">>do nothing<<\n"));
        }
    }
    return MCD_OK;
}

/**
* @internal mvPortCtrlApLinkFalling function
* @endinternal
*
* @brief   Callback to be called when link is falling or port deleted
*         1.  Write RF_RESET to host & line sides for all lanes
*         2.  Use local clock, which means rollback:
*         a.  int 30 with value 0 at line side
*         b.  int 5 with right divider at line side to work w/ 3.125 SerDes Speed
* @param[in] devNum
* @param[in] portNum
* @param[in] portMode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mvPortCtrlApLinkFalling
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode
)
{
    MCD_U16 serdesLane,i ;
    MCD_U32 laneSpeed,numOfActLanes;
    MCD_U8 slice, chan;
    MCD_U8 digitalReset = 0;
    MCD_U32 regData = 0;
    MCD_U8 tryCount = 0;
    MCD_U32 phySerdesLane;

    AP_PRINT_MCD_MAC((">>mvPortCtrlApLinkFalling port:%d portMode:%d retimerMode %d noPpmMode:%d<<\n",portNum, portMode, mcdDevDb.apCfg[portNum].retimerMode,
                  mcdDevDb.apCfg[portNum].noPpmMode));
    CHECK_STATUS(mcdGetPortParams(&mcdDevDb, portNum, portMode, &laneSpeed, &numOfActLanes));
    slice = portNum / MCD_MAX_CHANNEL_NUM_PER_SLICE;
    chan  = portNum % MCD_MAX_CHANNEL_NUM_PER_SLICE;

    if (mcdDevDb.apCfg[portNum].retimerMode)
    {
        for (i = 0; (i < numOfActLanes); i++)
        {
            serdesLane = portNum + i;
            /* convert logic serdes number to physical serdes number */
            MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((MCD_U32)serdesLane,phySerdesLane);
            /* if Training is in progress give it chance to finish and turn serdes off anyway */
            CHECK_STATUS(mcdHwGetPhyRegField32bit(&mcdDevDb,DONT_CARE,MCD_SERDES_Core_Status(phySerdesLane), 1, 1, &regData));
            tryCount = 0;
            while (regData && (tryCount++ < 5))
            {
                CHECK_STATUS(mcdHwGetPhyRegField32bit(&mcdDevDb,DONT_CARE,MCD_SERDES_Core_Status(phySerdesLane), 1, 1, &regData));
            }
            if (tryCount >=4)
            {
                mvPcPrintf(">>mvPortCtrlApLinkFalling regData %d serdesNum %d<<\n ", regData, phySerdesLane);
            }
        }
        for (i = 0; (i < numOfActLanes); i++)
        {
            serdesLane = portNum + i;
            CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane + MCD_HOST_SIDE_START_SERDES_NUMBER, MCD_FALSE, MCD_TRUE, MCD_FALSE));
            CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane, MCD_FALSE, MCD_TRUE, MCD_FALSE));
            CHECK_STATUS(mcdSerdesClockRegular(&mcdDevDb, serdesLane, MCD_3_125G));
        }

    }

    if (mcdDevDb.apCfg[portNum].noPpmMode != 0)
    {
        /* switch clock to local source clock */

        if (mcdDevDb.apCfg[portNum].noPpmMode != 2 /* RX squelch */)
        {
            for (i = 0; i < numOfActLanes; i++)
            {
                serdesLane = portNum + MCD_HOST_SIDE_START_SERDES_NUMBER + i;
                CHECK_STATUS(mcdSerdesNoPpmModeSet(&mcdDevDb, serdesLane, laneSpeed , chan, slice, MCD_FALSE));
            }
            mcdWrapperMsSleep(PRV_MCD_DELAY_NOPPM_CLOCK_SWITCH);
        }
        else
        {
            digitalReset = 1;
        }

        for (i = 0; i < numOfActLanes; i++)
        {
            serdesLane = portNum + MCD_HOST_SIDE_START_SERDES_NUMBER + i;
            {
                CHECK_STATUS(mcdSerdesResetImpl(&mcdDevDb, serdesLane, MCD_FALSE, digitalReset, MCD_FALSE));
            }
        }

        if (mcdDevDb.apCfg[portNum].noPpmMode == 3)
        {
            mcdHwSetPhyRegField32bit(&mcdDevDb, DONT_CARE, UMAC_Channel_MAC_Debug(slice,chan), 1, 1, 1);
        }
    }
    return MCD_OK;

}


