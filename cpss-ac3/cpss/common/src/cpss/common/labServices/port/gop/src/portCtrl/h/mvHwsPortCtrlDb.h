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
* @file mvHwsPortCtrlDb.h
*
* @brief Port Control Database
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlDb_H
#define __mvHwsPortCtrlDb_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Port Control function pointer definition   */
typedef void (*PORT_CTRL_FUNCPTR)(void);

/* Port Control common table definition */
typedef struct
{
    GT_U8 state;
    GT_U8 event;
    GT_U8 delayDuration;

}MV_HWS_PORT_CTRL_COMMON;

/* Port Control Supervisor module (M1) table definition */
typedef struct
{
    MV_HWS_PORT_CTRL_COMMON info;
    GT_U8                   highMsgThreshold;
    GT_U8                   lowMsgThreshold;
    PORT_CTRL_FUNCPTR       funcTbl[SPV_MAX_EVENT][SPV_MAX_STATE];
}MV_HWS_PORT_CTRL_SUPERVISOR;

/* Port management module (M2) table definition */
typedef struct
{
    MV_HWS_PORT_CTRL_COMMON info;
    PORT_CTRL_FUNCPTR       funcTbl[PORT_MNG_MAX_EVENT][PORT_MNG_MAX_STATE];

}MV_HWS_PORT_CTRL_PORT_MNG;

#define MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_HF_SHIFT           4
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_LOOPWIDTH_SHIFT    4
#define MV_HWS_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT        7

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
typedef struct
{
    GT_U16    fieldOverrideBmp;
    GT_U8     dcGain;
    GT_U8     lfHf; /* bits 0-3 = LF, bits 4-7 = HF */
    GT_U8     bandLoopWidth; /* bits 0-3 = bandWidth, bits 4-7 = loopBandwidth */
    GT_U16    squelch;
    GT_U8     etlMinDelay;
    GT_U8     etlMaxDelay;   /* bits 0-7 = max delay , bit 7 = etl enable value*/
}MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_DB;
#endif

#if (defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
typedef struct
{
    GT_BOOL   etlMode;
    GT_U8     etlMinDelay;
    GT_U8     etlMaxDelay;   /* bits 0-7 = max delay , bit 7 = etl enable value*/
}MV_HWS_PORT_CTRL_AP_PORT_ETL_STRUCT;
#endif

/* Port Control Port State machine module (M3) table definition */
typedef struct
{
    GT_U16 type;
    GT_U8  state;
    GT_U8  status;

    /*Rx parameters to oveeride port serdes configuration for 0=10G and 1=25G*/
#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
     MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_DB apPortRxConfigOverride[2];
#ifndef BOBK_DEV_SUPPORT
     MV_HWS_PORT_CTRL_AP_PORT_ETL_STRUCT apPortEtlParam;
#endif
#endif
}MV_HWS_PORT_CTRL_PORT_SM;

/* Port Control AP Port management module (M4) - Pending message table definition */
typedef struct
{
    GT_U8 ctrlMsgType;
    GT_U8 msgQueueId;
    GT_U8 phyPortNum;
    GT_U8 portMode;
    GT_U8 action;
    GT_U8 refClock;
    GT_U8 refClockSource;

}MV_HWS_IPC_CTRL_MSG_AP_STRUCT;

typedef struct
{
    GT_U8                         pendTasksCount;
    GT_U8                         pendTasksAddIndex;
    GT_U8                         pendTasksProcIndex;
    MV_HWS_IPC_CTRL_MSG_AP_STRUCT pendTasks[MV_PORT_CTRL_PORT_PEND_MSG_NUM];

}MV_HWS_PORT_CTRL_PEND_MSG_TBL;

/* Port Control AP Port management module (M4) - timer table definition */
typedef struct
{
    GT_U8  sysCfState;
    GT_U8  sysCfStatus;
    GT_U32 sysCfMode; /*MV_HWS_PORT_STANDARD*/
    GT_U32 sysCfgThreshold;
    GT_U8  trainingCount;
    GT_U8  trainingMaxInterval;
    GT_U32 trainingThreshold;

}MV_HWS_PORT_CTRL_AP_PORT_TIMER;

typedef GT_U32 MV_HWS_PORT_CTRL_AP_INIT;

/* Port Control AP Port management module (M4) table */
typedef struct
{
    MV_HWS_PORT_CTRL_COMMON        info;
    PORT_CTRL_FUNCPTR              funcTbl[AP_PORT_MNG_MAX_EVENT][AP_PORT_MNG_MAX_STATE];
    MV_HWS_PORT_CTRL_PORT_SM       apPortSm[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MV_HWS_PORT_CTRL_AP_PORT_TIMER apPortTimer[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MV_HWS_PORT_CTRL_PEND_MSG_TBL  tasks[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MV_HWS_PORT_CTRL_AP_INIT       apInitStatus;

}MV_HWS_PORT_CTRL_AP_PORT_MNG;

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
    GT_U32 ifNum;

    /*
    ** Capability
    ** [00:00] Advertisement 40GBase KR4
    ** [01:01] Advertisement 10GBase KR
    ** [02:02] Advertisement 10GBase KX4
    ** [03:03] Advertisement 1000Base KX
    ** [04:04] Advertisement 20GBase KR2
    ** [05:05] Advertisement 10GBase KX2
    ** [06:06] Advertisement 100GBASE-KR4
    ** [07:07] Advertisement 100GBASE-CR4
    ** [08:08] Advertisement 25GBASE-KR-S
    ** [09:09] Advertisement 25GBASE-KR
    ** [10:10] Advertisement consortium 25GBase KR1
    ** [11:11] Advertisement consortium 25GBase CR1
    ** [12:12] Advertisement consortium 50GBase KR2
    ** [13:13] Advertisement consortium 50GBase CR2
    ** [14:14] Advertisement 40GBase CR4
    ** [15:15] Advertisement 25GBASE-CR-S
    ** [16:16] Advertisement 25GBASE-CR
    ** [17:31] Reserved
    */
    GT_U32 capability;

    /*
    ** Options
    ** [00:00] Flow Control Pause Enable
    ** [01:01] Flow Control Asm Direction
    ** [02:03] Reserved
    ** [04:04] FEC Suppress En
    ** [05:05] FEC Request
    ** [06:06] BASE-R FEC requested
    ** [07:07] RS-FEC requested
    ** [08:08] Reserved
    ** [09:09] loopback Enable
    ** [10:10]consortium (RS-FEC) ability (F1)
    ** [11:11]consortium (BASE-R FEC) ability (F2)
    ** [12:12]consortium (RS-FEC) request (F3)
    ** [13:13]consortium (BASE-R FEC) request (F4)
    ** [14:14] ctleBiasValue
    ** [15:15] Reserved
    */
    GT_U16 options;

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
    GT_U16 ARMSmStatus;

    /*
    ** AP Status Info
    ** ==============
    */
    GT_U8 state;

    /*
    ** AP Status
    ** [00:00] Signal Detect
    ** [01:01] CDR Lock
    ** [02:02] PCS Lock
    ** [03:07] Reserved
    */
    GT_U8 status;

    GT_U8 portNum;
    /*GT_U8 queueId;*/
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
    GT_U8 polarityVector;

    /** HCD Status
    ** [03:03] Found
    ** [04:04] ARBSmError
    ** [05:05] FEC Result
    ** [06:06] Flow Control Rx Result
    ** [07:07] Flow Control Tx Result
    ** [08:19] Local HCD Type -------------------------------->        Port_1000Base_KX,
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
    **                                                                 Port_25GBASE_KR1_CONSORTIUM,
    **                                                                 Port_25GBASE_CR1_CONSORTIUM,
    **                                                                 Port_50GBASE_KR2_CONSORTIUM,
    **                                                                 Port_50GBASE_CR2_CONSORTIUM,
    **                                                                 Port_25GBASE_CR_S,
    **                                                                 Port_25GBASE_CR,
    **
    ** [20:20] Consortium type result
    ** [22:23] FEC type-----------> [0:0] BASE-R FEC
    **                              [1:1] RS-FEC
    ** [29:29] HCD
    ** Interrupt Trigger [30:30] Link Interrupt Trigger [31:31] Link
    **  */
    GT_U32 hcdStatus;

}MV_HWS_AP_SM_INFO;

/* Port Control AP Detection State machine module (M5) - timer table definition */
typedef struct
{
    GT_U32 txDisThreshold;
    GT_U32 abilityThreshold;
    GT_U16 abilityCount;
    GT_U32 linkThreshold;
    GT_U16 linkCount;

}MV_HWS_AP_SM_TIMER;

/* Port Control AP Detection State machine module (M5) - Debug section definition */
typedef struct
{
    GT_U16 abilityInvalidCnt; /* Number of Ability detect invalid intervals - ability detected but no resolution */

    GT_U16 txDisCnt;          /* Number of Tx Disable intervals executed */
    GT_U16 abilityCnt;        /* Number of Ability detect intervals executed */
    GT_U16 abilitySuccessCnt; /* Number of Ability detect successfull intervals executed */
    GT_U16 linkFailCnt;       /* Number of Link check fail intervals executed */
    GT_U16 linkSuccessCnt;    /* Number of Link check successfull intervals executed */

    GT_U32 timestampTime;     /* temp parameter holds time stamp for HCD and link up time calc */
    GT_U32 hcdResoultionTime; /* Time duration for HCD resolution */
    GT_U32 linkUpTime;        /* Time duration for Link up */

}MV_HWS_AP_SM_STATS;

/* Port Control AP Detection State machine module (M5) table definition */
typedef struct
{
    MV_HWS_AP_SM_INFO  info;
    MV_HWS_AP_SM_TIMER timer;
    MV_HWS_AP_SM_STATS stats;

}MV_HWS_AP_DETECT_SM;

/* Port Control AP Detection State machine module (M5) - Interop section definition */
typedef struct
{
    GT_U16 attrBitMask;
    GT_U16 txDisDuration;          /* Tx Disable duration - default 60 msec */
    GT_U16 abilityDuration;        /* Ability detect duration - default 6 msec */
    GT_U16 abilityMaxInterval;     /* Ability detect max interval - default 21 = 126msec */
    GT_U16 abilityFailMaxInterval; /* Ability detect fail interval - default 10, ARB SM report GOOD CHECK but no HCD */
    GT_U16 apLinkDuration;         /* AP Link check duration - default 20 msec */
    GT_U16 apLinkMaxInterval;      /* AP Link check max interval - default 25 */
    GT_U16 pdLinkDuration;         /* PD Link check duration - default 25 msec */
    GT_U16 pdLinkMaxInterval;      /* PD Link check max interval - default 10 */

}MV_HWS_AP_DETECT_ITEROP;

/* Port Control AP Detection State machine module (M5) table definition */

typedef struct
{
    MV_HWS_PORT_CTRL_COMMON info;
    PORT_CTRL_FUNCPTR funcTbl[AP_PORT_DETECT_MAX_EVENT][AP_PORT_DETECT_MAX_STATE];
    MV_HWS_AP_DETECT_SM apPortSm[MV_PORT_CTRL_MAX_AP_PORT_NUM];
    MV_HWS_AP_DETECT_ITEROP introp;

}MV_HWS_PORT_CTRL_AP_DETECT;

#ifdef AP_GENERAL_TASK
/* Port Control General module (M6) table definition */
typedef struct
{
    MV_HWS_PORT_CTRL_COMMON info;
    PORT_CTRL_FUNCPTR funcTbl[PORT_GEN_MAX_EVENT][PORT_GEN_MAX_STATE];

}MV_HWS_PORT_CTRL_GENERAL;
#endif

/* Port Control Supervisor module (M1) table */
extern MV_HWS_PORT_CTRL_SUPERVISOR mvPortCtrlSpv;
#ifdef REG_PORT_TASK
/* Port Control Port management module (M2) table */
extern MV_HWS_PORT_CTRL_PORT_MNG mvPortCtrlPortMng;
/* Port Control Port State machine module (M3) table */
extern MV_HWS_PORT_CTRL_PORT_SM mvPortCtrlPortSm[MV_PORT_CTRL_MAX_PORT_NUM];
#endif
/* Port Control AP Port management module (M4) table */
extern MV_HWS_PORT_CTRL_AP_PORT_MNG mvHwsPortCtrlApPortMng;
/* Port Control AP Port State machine module (M5) table */
extern MV_HWS_PORT_CTRL_AP_DETECT mvHwsPortCtrlApPortDetect;
#ifdef AP_GENERAL_TASK
/* Port Control General module (M6) table */
extern MV_HWS_PORT_CTRL_GENERAL mvPortCtrlGen;
#endif

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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlDb_H */



