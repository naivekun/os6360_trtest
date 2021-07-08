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
* @file mcdFwPortCtrlDb.h
*
* @brief Port Control Database
*
* @version   1
********************************************************************************
*/

#ifndef __mcdFwPortCtrlDb_H
#define __mcdFwPortCtrlDb_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Port Control function pointer definition   */
typedef void (*PORT_CTRL_FUNCPTR)(void);

/* Port Control common table definition */
typedef struct
{
    MCD_U8 state;
    MCD_U8 event;
    MCD_U8 delayDuration;

}MCD_PORT_CTRL_COMMON;

/* Port Control Supervisor module (M1) table definition */
typedef struct
{
    MCD_PORT_CTRL_COMMON info;
    MCD_U8                   highMsgThreshold;
    MCD_U8                   lowMsgThreshold;
    PORT_CTRL_FUNCPTR       funcTbl[SPV_MAX_EVENT][SPV_MAX_STATE];
}MCD_PORT_CTRL_SUPERVISOR;

/* Port management module (M2) table definition */
typedef struct
{
    MCD_PORT_CTRL_COMMON info;
    PORT_CTRL_FUNCPTR       funcTbl[PORT_MNG_MAX_EVENT][PORT_MNG_MAX_STATE];

}MCD_PORT_CTRL_PORT_MNGR;

typedef struct
{
    MCD_U16    fieldOverrideBmp;
    MCD_U8     dcGain;
    MCD_U8     lfHf; /* bits 0-3 = LF, bits 4-7 = HF */
    MCD_U8     bandLoopWidth; /* bits 0-3 = bandWidth, bits 4-7 = loopBandwidth */
    MCD_U16    squelch;
    MCD_U8     etlMinDelay;
    MCD_U8     etlMaxDelay;   /* bits 0-7 = max delay , bit 7 = etl enable value*/
}MCD_MAN_TUNE_CTLE_CONFIG_OVERRIDE_DB;

/* Port Control Port State machine module (M3) table definition */
typedef struct
{
    MCD_U16 type;
    MCD_U8  state;
    MCD_U8  status;
    MCD_MAN_TUNE_CTLE_CONFIG_OVERRIDE_DB apPortRxConfigOverride[2];

}MCD_PORT_CTRL_PORT_SM;

typedef MCD_STATUS (* MCD_PORT_CTRL_AP_HCD_FOUND)
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode
);

typedef MCD_STATUS (* MCD_PORT_CTRL_AP_TRX_TRAINING_END)
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode
);
typedef MCD_STATUS (* MCD_PORT_CTRL_AP_LINK_FALLING)
(
    IN MCD_U32              devNum,
    IN MCD_U16              portNum,
    IN MV_HWS_PORT_STANDARD portMode

);

typedef struct
{
    MCD_PORT_CTRL_AP_HCD_FOUND          apHcdFoundClbk;       /* called when resolution is found */
    MCD_PORT_CTRL_AP_TRX_TRAINING_END   apTrxTrainingEndClbk; /* called after training completed */
    MCD_PORT_CTRL_AP_LINK_FALLING       apLinkFallingClbk;    /* called when link is falling */

}MCD_PORT_CTRL_AP_CALLBACKS;

/* Port Control AP Port management module (M4) - Pending message table definition */
typedef struct
{
    MCD_U8 ctrlMsgType;
    MCD_U8 msgQueueId;
    MCD_U8 phyPortNum;
    MCD_U8 portMode;
    MCD_U8 action;
    MCD_U8 refClock;
    MCD_U8 refClockSource;

}MCD_IPC_CTRL_MSG_AP_STRUCT;

typedef struct
{
    MCD_U8                         pendTasksCount;
    MCD_U8                         pendTasksAddIndex;
    MCD_U8                         pendTasksProcIndex;
    MCD_IPC_CTRL_MSG_AP_STRUCT pendTasks[MV_PORT_CTRL_PORT_PEND_MSG_NUM];

}MCD_PORT_CTRL_PEND_MSG_TBL;

/* Port Control AP Port management module (M4) - timer table definition */
typedef struct
{
    MCD_U8  sysCfState;
    MCD_U8  sysCfStatus;
    MCD_U32 sysCfgThreshold;
    MCD_U8  trainingCount;
    MCD_U8  trainingMaxInterval;
    MCD_U32 trainingThreshold;

}MCD_PORT_CTRL_AP_PORT_TIMER;

typedef MCD_U32 MCD_PORT_CTRL_AP_INIT;

/* Port Control AP Port management module (M4) table */
typedef struct
{
    MCD_PORT_CTRL_COMMON        info;
    PORT_CTRL_FUNCPTR              funcTbl[AP_PORT_MNG_MAX_EVENT][AP_PORT_MNG_MAX_STATE];
    MCD_PORT_CTRL_PORT_SM       apPortSm[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MCD_PORT_CTRL_AP_PORT_TIMER apPortTimer[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MCD_PORT_CTRL_PEND_MSG_TBL  tasks[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MCD_PORT_CTRL_AP_INIT       apInitStatus;

}MCD_PORT_CTRL_AP_PORT_MNG;

/* Port Control AP Detection State machine module (M5) - Info section definition */

typedef struct
{
    /*
    ** AP Configuration Info
    ** =====================
    */

    /*
    ** Interface Number
    ** [00:07] Lane Number
    ** [08:15] PCS Number
    ** [16:23] MAC Number
    ** [24:31] queueId
    */
    MCD_U32 ifNum;

/*
      ** Capability
      ** [00:00] Advertisement 1000BASE-KX                            0x0001
      ** [01:01] Advertisement 10GBase-KX4                            0x0002
      ** [02:02] Advertisement 10GBase-KR                             0x0004
      ** [03:03] Advertisement 40GBASE-KR4                            0x0008
      ** [04:04] Advertisement 40GBASE-CR4                            0x0010
      ** [05:05] Advertisement 100GBASE-CR10                          0x0020
      ** [06:06] Advertisement 100GBASE-KP4                           0x0040
      ** [07:07] Advertisement 100GBASE-KR4                           0x0080
      ** [08:08] Advertisement 100GBASE-CR4                           0x0100
      ** [09:09] Advertisement 25GBASE-KR-S or 25GBASE-CR-S           0x0200
      ** [10:10] Advertisement 25GBASE-KR or 25GBASE-CR               0x0400
      ** [11:11] Advertisement consortium 25GBase KR1                 0x0800
      ** [12:12] Advertisement consortium 25GBase CR1                 0x1000
      ** [13:13] Advertisement consortium 50GBase KR2                 0x2000
      ** [14:14] Advertisement consortium 50GBase CR2                 0x4000
      ** [15:131] Reserved
*/
    MCD_U32 capability;




    /*
    ** Options
    ** [00:00] Flow Control Pause Enable
    ** [01:01] Flow Control Asm Direction
    ** [02:03] Reserved
    ** [04:04] FEC Suppress En
    ** [05:05] FEC Request
    ** [06:07] Reserved   = RS-FEC requested, BASE-R FEC requested (for 25G IEEE)
    ** [08:08] ctleBiasValue
    ** [09:09] loopback Enable
    ** [10:10]consortium (RS-FEC) ability (F1)
    ** [11:11]consortium (BASE-R FEC) ability (F2)
    ** [12:12]consortium (RS-FEC) request (F3)
    ** [13:13]consortium (BASE-R FEC) request (F4)
    ** [14:15] noPpmMode
    */
    MCD_U16 options;

    /*
    ** ARBSmStatus
    ** [00:00] ST_AN_ENABLE
    ** [01:01] ST_TX_DISABLE
    ** [02:02] ST_LINK_STAT_CK
    ** [03:03] ST_PARALLEL_FLT
    ** [04:04] ST_ABILITY_DET
    ** [05:05] ST_ACK_DETECT
    ** [06:06] ST_COMPLETE_ACK
    ** [07:07] ST_NP_WAIT
    ** [08:08] ST_AN_GOOD_CK
    ** [09:09] ST_AN_GOOD
    ** [10:10] ST_SERDES_WAIT
    */
    MCD_U16 ARMSmStatus;


    /*
    ** AP Status Info
    ** ==============
    */
   MCD_U8 state;

    /*
    ** AP Status
    ** [00:00] Signal Detect
    ** [01:01] CDR Lock
    ** [02:02] PCS Lock
    ** [03:07] Reserved
    */
    MCD_U8 status;

    MCD_U8 portNum;
   /* MCD_U8 queueId;*/
    /*
    ** polarityVector
    ** [00:00] Rx polarity of active lane=(num of active lane)
    ** [01:01] Tx polarity of active lane=(num of active lane)
    ** [02:02] Rx polarity of active lane=(num of active lane - 1)
    ** [03:03] Tx polarity of active lane=(num of active lane - 1)
    ** [04:04] Rx polarity of active lane=(num of active lane - 2)
    ** [05:05] Tx polarity of active lane=(num of active lane - 2)
    ** [06:06] Rx polarity of active lane=(num of active lane - 3)
    ** [07:07] Rx polarity of active lane=(num of active lane - 3)
    */
    MCD_U8 polarityVector;

    /*
    ** HCD Status
    ** [03:03] Found
    ** [04:04] ARBSmError
    ** [05:05] FEC Result
    ** [06:06] Flow Control Rx Result
    ** [07:07] Flow Control Tx Result
    ** [08:18] Local HCD Type -------------------------------->        Port_1000Base_KX,
    **                                                                 Port_10GBase_KX4,
    **                                                                 Port_10GBase_R,
    **                                                                 Port_25GBASE_KR_S,
    **                                                                 Port_25GBASE_KR,
    **                                                                 Port_40GBase_R,
    **                                                                 Port_40GBASE_CR4,
    **                                                                 Port_100GBASE_CR10,
    **                                                                 Port_100GBASE_KP4,
    **                                                                 Port_100GBASE_KR4,
    **                                                                 Port_100GBASE_CR4,
    ** [19:22] Consortium type result  ------------------------>       Port_25GBASE_KR1_CONSORTIUM,
    **                                                                 Port_25GBASE_CR1_CONSORTIUM,
    **                                                                 Port_50GBASE_KR2_CONSORTIUM,
    **                                                                 Port_50GBASE_CR2_CONSORTIUM,
    **
    ** [22:23] FEC type --------------> [0:0] BASE-R FEC
    **                                  [1:1] RS-FEC
    ** [29:29] HCD Interrupt Trigger
    ** [30:30] Link Interrupt Trigger
    ** [31:31] Link
    **
    */
    MCD_U32 hcdStatus;

}MCD_AP_SM_INFO;

/* Port Control AP Detection State machine module (M5) - timer table definition */
typedef struct
{
    MCD_U32 txDisThreshold;
    MCD_U32 abilityThreshold;
    MCD_U16 abilityCount;
    MCD_U32 linkThreshold;
    MCD_U16 linkCount;

}MCD_AP_SM_TIMER;

/* Port Control AP Detection State machine module (M5) - Debug section definition */
typedef struct
{
    MCD_U16 abilityInvalidCnt; /* Number of Ability detect invalid intervals - ability detected but no resolution */

    MCD_U16 txDisCnt;          /* Number of Tx Disable intervals executed */
    MCD_U16 abilityCnt;        /* Number of Ability detect intervals executed */
    MCD_U16 abilitySuccessCnt; /* Number of Ability detect successfull intervals executed */
    MCD_U16 linkFailCnt;       /* Number of Link check fail intervals executed */
    MCD_U16 linkSuccessCnt;    /* Number of Link check successfull intervals executed */

    MCD_U32 timestampTime;     /* temp parameter holds time stamp for HCD and link up time calc */
    MCD_U32 hcdResoultionTime; /* Time duration for HCD resolution */
    MCD_U32 linkUpTime;        /* Time duration for Link up */
    MCD_U32 portEnableTime;
    MCD_U32 hcdClbkStartTime;
    MCD_U32 hcdClbkStopTime;
    MCD_U32 trxClbkStartTime;
    MCD_U32 trxClbkStopTime;
    MCD_U32 linkClbkStartTime;
    MCD_U32 linkClbkStopTime;
}MCD_AP_SM_STATS;

/* Port Control AP Detection State machine module (M5) table definition */
typedef struct
{
    MCD_AP_SM_INFO  info;
    MCD_AP_SM_TIMER timer;
    MCD_AP_SM_STATS stats;

}MCD_AP_DETECT_SM;

/* Port Control AP Detection State machine module (M5) - Interop section definition */
typedef struct
{
    MCD_U16 attrBitMask;
    MCD_U16 txDisDuration;          /* Tx Disable duration - default 60 msec */
    MCD_U16 abilityDuration;        /* Ability detect duration - default 6 msec */
    MCD_U16 abilityMaxInterval;     /* Ability detect max interval - default 21 = 126msec */
    MCD_U16 abilityFailMaxInterval; /* Ability detect fail interval - default 10, ARB SM report GOOD CHECK but no HCD */
    MCD_U16 apLinkDuration;         /* AP Link check duration - default 20 msec */
    MCD_U16 apLinkMaxInterval;      /* AP Link check max interval - default 25 */
    MCD_U16 pdLinkDuration;         /* PD Link check duration - default 25 msec */
    MCD_U16 pdLinkMaxInterval;      /* PD Link check max interval - default 10 */

}MCD_AP_DETECT_ITEROP;

/* Port Control AP Detection State machine module (M5) table definition */

typedef struct
{
    MCD_PORT_CTRL_COMMON info;
    PORT_CTRL_FUNCPTR funcTbl[AP_PORT_DETECT_MAX_EVENT][AP_PORT_DETECT_MAX_STATE];
    MCD_AP_DETECT_SM apPortSm[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MCD_AP_DETECT_ITEROP introp;

}MCD_PORT_CTRL_AP_DETECT;

/* Port Control General module (M6) table definition */
typedef struct
{
    MCD_PORT_CTRL_COMMON info;
    PORT_CTRL_FUNCPTR funcTbl[PORT_GEN_MAX_EVENT][PORT_GEN_MAX_STATE];

}MCD_PORT_CTRL_GENERAL;

/* Port Control Supervisor module (M1) table */
extern MCD_PORT_CTRL_SUPERVISOR mvPortCtrlSpv;
/* Port Control Port management module (M2) table */
extern MCD_PORT_CTRL_PORT_MNGR mvPortCtrlPortMng;
/* Port Control Port State machine module (M3) table */
extern MCD_PORT_CTRL_PORT_SM mvPortCtrlPortSm[MV_PORT_CTRL_MAX_PORT_NUM];
/* Port Control AP Port management module (M4) table */
extern MCD_PORT_CTRL_AP_PORT_MNG mvHwsPortCtrlApPortMng;
/* Port Control AP Port State machine module (M5) table */
extern MCD_PORT_CTRL_AP_DETECT mvHwsPortCtrlApPortDetect;
/* Port Control General module (M6) table */
extern MCD_PORT_CTRL_GENERAL mvPortCtrlGen;

/**
* @internal mvPortCtrlDbInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlDbInit(void);

/**
* @internal mvPortCtrlDevInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlDevInit(void);

/**
* @internal mvPortCtrlApCallbacksInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlApCallbacksInit(MCD_PORT_CTRL_AP_CALLBACKS *apCallbacksPtr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mcdFwPortCtrlDb_H */




