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
*/
/**
********************************************************************************
* @file mvHwsIpcDefs.h
*
* @brief Definitions for HWS IPS feature
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsIpcDefs_H
#define __mvHwsIpcDefs_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

/**************************** Definition *************************************************/
#define MV_HWS_IPC_HI_PRI_RX_CH_ID           0
#define MV_HWS_IPC_LO_PRI_RX_CH_ID           1
#define MV_HWS_IPC_TX_0_CH_ID                2
#define MV_HWS_IPC_TX_1_CH_ID                3
#define MV_HWS_IPC_TX_2_CH_ID                4
#define MV_HWS_IPC_TX_3_CH_ID                5
#define MV_HWS_IPC_CH_NUM                    6

#define MV_HWS_IPC_MSG_INFO_LENGTH           4  /* 4 bytes for msg info (dev size, type & queue) */
#define MV_HWS_IPC_MAX_MSG_DATA_LENGTH       56
#define MV_HWS_IPC_PORT_GEN_DATA_LENGTH      16 /* general data includes 4 parameters :
                                                   devNum;
                                                   portGroup;
                                                   phyPortNum;
                                                   portMode;*/

#define MV_HWS_IPC_PORT_MISC_DATA_LENGTH    (MV_HWS_IPC_MAX_MSG_DATA_LENGTH - MV_HWS_IPC_PORT_GEN_DATA_LENGTH)

#define MV_HWS_MAX_IPC_CTRL_MSG_LENGTH      (MV_HWS_IPC_MAX_MSG_DATA_LENGTH + MV_HWS_IPC_MSG_INFO_LENGTH)
#define MV_HWS_MAX_IPC_REPLY_LENGTH          40 /* the number should be updated after all APIs are implemented
                                                   = max length of reply data - now max is:
                                                   status per serdes in port = GT_BOOL*MV_HWS_MAX_LANES_NUM_PER_PORT */

#define MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM      6
#define MV_HWS_MAX_HWS2HOST_REPLY_QUEUE_NUM  MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM

#define MV_HWS_IPC_FREE_QUEUE                0
#define MV_HWS_IPC_QUEUE_BUSY                0xff

#define MV_HWS_IPC_MAX_PORT_NUM              72

#define MW_HWS_WRONG_IPC_MSG_TYPE            0xF0

typedef enum
{
    MV_HWS_IPC_HIGH_PRI_QUEUE,
    MV_HWS_IPC_LOW_PRI_QUEUE

}MV_HWS_IPC_PRIORITY_QUEUE;

typedef enum
{
    MV_HWS_IPC_PORT_AP_ENABLE_MSG,
    MV_HWS_IPC_PORT_AP_DISABLE_MSG,
    MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG,
    MV_HWS_IPC_PORT_AP_CFG_GET_MSG,
    MV_HWS_IPC_PORT_AP_STATUS_MSG,
    MV_HWS_IPC_PORT_AP_STATS_MSG,
    MV_HWS_IPC_PORT_AP_STATS_RESET_MSG,
    MV_HWS_IPC_PORT_AP_INTROP_GET_MSG,
    MV_HWS_IPC_PORT_AP_INTROP_SET_MSG,
    MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG,
    MV_HWS_IPC_PORT_PARAMS_MSG,
    MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG,
    MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG,
    MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG,
    MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG,
    MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG,
    MV_HWS_IPC_NOOP_MSG,
    MV_HWS_IPC_PORT_INIT_MSG,
    MV_HWS_IPC_PORT_RESET_MSG,
    MV_HWS_IPC_PORT_RESET_EXT_MSG,
    MV_HWS_IPC_HWS_LOG_GET_MSG,
    MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG,
    MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG,
    MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG,
    MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG,
    MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG,
    MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG,
    MV_HWS_IPC_PORT_SERDES_RESET_MSG,
    MV_HWS_IPC_PORT_POLARITY_SET_MSG,
    MV_HWS_IPC_PORT_FEC_CONFIG_MSG,
    MV_HWS_IPC_PORT_TX_ENABLE_MSG,
    MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG,
    MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG,
    MV_HWS_IPC_PORT_LOOPBACK_SET_MSG,
    MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG,
    MV_HWS_IPC_PORT_PPM_SET_MSG,
    MV_HWS_IPC_PORT_PPM_GET_MSG,
    MV_HWS_IPC_PORT_IF_GET_MSG,
    MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG,
    MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG,
    MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG,
    MV_HWS_IPC_PORT_FC_STATE_SET_MSG,
    MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG,
    MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG,
    MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG,
    MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG,
    MV_HWS_IPC_LAST_CTRL_MSG_TYPE

}MV_HWS_IPC_CTRL_MSG_DEF_TYPE;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16 portGroup;
    GT_U16 phyPortNum;
    GT_U32 advMode;
    GT_U16 options;
    GT_U8 laneNum;
    GT_U8 pcsNum;
    GT_U8 macNum;
    GT_U8 polarityVector;
    GT_U8 refClockSrcParams;

}MV_HWS_IPC_PORT_AP_DATA_STRUCT;

typedef struct
{
    GT_U32 state;

}MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U16 attrBitMask;
    GT_U16 txDisDuration;
    GT_U16 abilityDuration;
    GT_U16 abilityMaxInterval;
    GT_U16 abilityFailMaxInterval;
    GT_U16 apLinkDuration;
    GT_U16 apLinkMaxInterval;
    GT_U16 pdLinkDuration;
    GT_U16 pdLinkMaxInterval;

}MV_HWS_IPC_PORT_AP_INTROP_STRUCT;

/* General structure for all get/check status functions*/
/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
}MV_HWS_IPC_PORT_INFO_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U8                           portMiscData[MV_HWS_IPC_PORT_MISC_DATA_LENGTH];
}MV_HWS_IPC_PORT_GENERAL_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                            portGroup;
    GT_U16                            phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/   portStandard;
    GT_U32 /*MV_HWS_PORT_MAC_TYPE*/   portMacType;
    GT_U32                            portMacNumber;
    GT_U32 /*MV_HWS_PORT_PCS_TYPE*/   portPcsType;
    GT_U32                            portPcsNumber;
    GT_U32 /*MV_HWS_PORT_FEC_MODE*/   portFecMode;
    GT_U32 /*MV_HWS_SERDES_SPEED*/    serdesSpeed;
    GT_U8                             firstLaneNum;
    GT_U8                             numOfActLanes;
    GT_U8                             activeLanesList[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32 /*MV_HWS_SERDES_MEDIA*/    serdesMediaType;
    GT_U32 /*MV_HWS_SERDES_10B_MODE*/ serdes10BitStatus;

}MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                              portGroup;
    GT_U16                              phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD */    portMode;
    GT_U32 /*GT_BOOL*/                  lbPort;
    GT_U32 /*MV_HWS_REF_CLOCK_SUP_VAL*/ refClock;
    GT_U32 /*MV_HWS_REF_CLOCK_SOURCE*/  refClockSource;

}MV_HWS_IPC_PORT_INIT_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*MV_HWS_PORT_ACTION*/   action;

}MV_HWS_IPC_PORT_RESET_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                                portGroup;
    GT_U16                                phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/       portMode;
    GT_U32 /*MV_HWS_PORT_AUTO_TUNE_MODE*/ portTuningMode;
    GT_U32                                optAlgoMask;

}MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*GT_BOOL*/              stopRx;
    GT_U32 /*GT_BOOL*/              stopTx;

}MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT;

typedef struct
{
GT_U32 /*GT_BOOL*/              portEnableCtrl;

}MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_32                           txInvMask;
    GT_32                           rxInvMask;

}MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT;

typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*GT_BOOL*/              enable;

}MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*GT_BOOL*/              portFecType;/*0-AP_ST_HCD_FEC_RES_NONE, 1-AP_ST_HCD_FEC_RES_FC, 2-AP_ST_HCD_FEC_RES_RS*/

}MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*MV_HWS_UNIT*/          lpPlace;
    GT_U32 /*MV_HWS_PORT_LB_TYPE*/  lbType;

}MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT;

typedef struct
{
    GT_U16                                  portGroup;
    GT_U16                                  phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/         portMode;
    GT_U32 /*MV_HWS_PORT_FLOW_CONTROL_ENT*/ fcState;

}MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*MV_HWS_UNIT*/          lpPlace;

}MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*MV_HWS_PPM_VALUE*/     portPPM;

}MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT;

typedef struct
{
    GT_U16 portGroup;
    GT_U16 phyPortNum;

}MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT;

typedef struct
{
    GT_U16                               portGroup;
    GT_U32                               serdesNum;
    GT_U32 /*MV_HWS_SERDES_TYPE*/        serdesType;
    GT_U32 /*MV_HWS_PORT_MAN_TUNE_MODE*/ portTuningMode;
    GT_U32                               sqlch;
    GT_U32                               ffeRes;
    GT_U32                               ffeCap;
    GT_U32 /*GT_BOOL*/                   dfeEn;
    GT_U32                               alig;

}MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT;

typedef struct
{
    GT_U16                        portGroup;
    GT_U32                        serdesNum;
    GT_U32 /*MV_HWS_SERDES_TYPE*/ serdesType;
    GT_U32                        txAmp;
    GT_U32 /*GT_BOOL*/            txAmpAdj;
    GT_U32                        emph0;
    GT_U32                        emph1;
    GT_U32 /*GT_BOOL*/            txAmpShft;

}MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;

}MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT;

typedef struct
{
    GT_U32 data;
}MV_HWS_IPC_NOOP_DATA_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32                          vosOverrideParamsBufferPtr[8];
    GT_U32                          vosOverrideParamsBufferIdx;
    GT_U32                          vosOverrideParamsBufferLength;

}MV_HWS_IPC_VOS_OVERRIDE_PARAMS;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portMode;
    GT_U32 /*GT_BOOL*/              vosOverrideMode;

}MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT;

/* Note: the first three parameters MUSST have fixed sizes:
 * GT_U16 portGroup;GT_U16 phyPortNum; GT_U32 portMode
 */
typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    GT_U32                          serdesSpeed;
    GT_U8                           serdesLane;
    GT_U16                          offsets;

}MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC;

typedef struct
{
    GT_U16                          portGroup;
    GT_U16                          phyPortNum;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE rxCfg;/*for 10G and 25G*/
}MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC;

typedef struct{
    GT_U32 txOffsetBaseAddr;
    GT_U32 txOffsetSize;

} MV_HWS_IPC_PORT_TX_OFFSETS_INFO_STRUCT;

typedef union
{
    GT_U8                                               ctrlData[MV_HWS_IPC_MAX_MSG_DATA_LENGTH];
    MV_HWS_IPC_PORT_GENERAL_STRUCT                      portGeneral;
    MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT                  portParams;
    MV_HWS_IPC_PORT_INIT_DATA_STRUCT                    portInit;
    MV_HWS_IPC_PORT_RESET_DATA_STRUCT                   portReset;
    MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT       portAutoTuneSetExt;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portLinkStatus;
    MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT          portAutoTuneStop;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portAutoTuneStateChk;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portTxEnableGet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portFecConfigGet;
    MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT            portPolaritySet;
    MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT               portTxEnableData;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portSignalDetectGet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portCdrLockStatus;
    MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT              portFecConfig;
    MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT            portLoopbackSet;
    MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT            portLoopbackGet;
    MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT                 portPPMSet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portPPMGet;
    MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT                  portInterfaceGet;
    MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT serdesManualRxConfig;
    MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT serdesManualTxConfig;
    MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT   portPcsActiveStatus;
    MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT        portFcStateSet;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT                      apConfig;
    MV_HWS_IPC_PORT_INFO_STRUCT                         apSysConfig;
    MV_HWS_IPC_PORT_INFO_STRUCT                         apStatusGet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         apStatsGet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         apStatsReset;
    MV_HWS_IPC_PORT_INFO_STRUCT                         apIntropGet;
    MV_HWS_IPC_PORT_AP_INTROP_STRUCT                    apIntropSet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         logGet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         avagoAddrGet;
    MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT                    avagoGuiSet;
    MV_HWS_IPC_NOOP_DATA_STRUCT                         noop;
    MV_HWS_IPC_PORT_INFO_STRUCT                         avagoAaplInit;
    MV_HWS_IPC_VOS_OVERRIDE_PARAMS                      vosOverrideParams;
    MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT                 vosOverrideModeSet;
    MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT             portEnableCtrlSet;
    MV_HWS_IPC_PORT_INFO_STRUCT                         portEnableCtrlGet;
    MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC               portSerdesTxParametersOffsetConfig;
    MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC       portSerdesRxConfigOverride;
}MV_HWS_IPC_MSG_API_PARAMS;


typedef struct
{
    GT_U32 /*MV_HWS_AUTO_TUNE_STATUS*/ rxTune;
    GT_U32 /*MV_HWS_AUTO_TUNE_STATUS*/ txTune;
}MV_HWS_IPC_PORT_REPLY_AUTO_TUNE_STATE_CHK;

typedef struct
{
    GT_U32 /*GT_BOOL*/ status;

}MV_HWS_IPC_PORT_REPLY_STATUS_GET;

typedef struct
{
    GT_U32 /*GT_BOOL*/ status[MV_HWS_MAX_LANES_NUM_PER_PORT];

}MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET;

typedef struct
{
    GT_U32 ifNum;
    GT_U32 capability;
    GT_U16 options;

}MV_HWS_IPC_PORT_REPLY_AP_CFG_GET;

typedef struct
{
    GT_U16 state;
    GT_U16 status;
    GT_U16 laneNum;
    GT_U16 ARMSmStatus;
    GT_U32 hcdStatus;

}MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET;

typedef struct
{
    GT_U16 txDisCnt;
    GT_U16 abilityCnt;
    GT_U16 abilitySuccessCnt;
    GT_U16 linkFailCnt;
    GT_U16 linkSuccessCnt;
    GT_U32 hcdResoultionTime;
    GT_U32 linkUpTime;

}MV_HWS_IPC_PORT_REPLY_AP_STATS_GET;

typedef struct
{
    GT_U16 txDisDuration;
    GT_U16 abilityDuration;
    GT_U16 abilityMaxInterval;
    GT_U16 abilityFailMaxInterval;
    GT_U16 apLinkDuration;
    GT_U16 apLinkMaxInterval;
    GT_U16 pdLinkDuration;
    GT_U16 pdLinkMaxInterval;

}MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET;

typedef struct
{
    GT_U8 portEnableCtrl;

}MV_HWS_IPC_PORT_REPLY_AP_PORT_ENABLE_CTRL_GET;


typedef struct
{
    GT_U32 fwBaseAddr;
    GT_U32 logBaseAddr;
    GT_U32 logCountAddr;
    GT_U32 logPointerAddr;
    GT_U32 logResetAddr;

}MV_HWS_IPC_LOG_GET;

typedef struct
{
    GT_U32 data;
}MV_HWS_IPC_NOOP_REPLY;

typedef struct
{
    GT_U32 results;

}MV_HWS_IPC_PORT_REPLY_RESULT;

typedef struct
{
    GT_U32 /*MV_HWS_PORT_LB_TYPE*/ lbType;

}MV_HWS_IPC_PORT_REPLY_LOOPBACK_STATUS_GET;

typedef struct
{
    GT_U32 /*MV_HWS_PPM_VALUE*/ portPpm;

}MV_HWS_IPC_PORT_REPLY_PPM_GET;

typedef struct
{
    GT_U32 /*MV_HWS_PORT_STANDARD*/ portIf;

}MV_HWS_IPC_PORT_REPLY_IF_GET;

typedef struct
{
    GT_U32 addr;
    GT_U32 size;

}MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET;

typedef union
{
    GT_U8                                             resultData[MV_HWS_MAX_IPC_REPLY_LENGTH];
    MV_HWS_IPC_PORT_REPLY_AUTO_TUNE_STATE_CHK         portAutoTuneStateChk;
    MV_HWS_IPC_PORT_REPLY_STATUS_GET                  portStatusGet;
    MV_HWS_IPC_PORT_REPLY_RESULT                      portReplyGet;
    MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET       portSerdesTxEnableGet;
    MV_HWS_IPC_PORT_REPLY_LOOPBACK_STATUS_GET         portLoopbackStatusGet;
    MV_HWS_IPC_PORT_REPLY_PPM_GET                     portPpmGet;
    MV_HWS_IPC_PORT_REPLY_IF_GET                      portIfGet;
    MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET       portSerdesSignalDetectGet;
    MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET       portSerdesCdrLockStatusGet;
    MV_HWS_IPC_PORT_REPLY_AP_CFG_GET                  portApCfgGet;
    MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET               portApStatusGet;
    MV_HWS_IPC_PORT_REPLY_AP_STATS_GET                portApStatsGet;
    MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET               portApIntropGet;
    MV_HWS_IPC_LOG_GET                                logGet;
    MV_HWS_IPC_NOOP_REPLY                             noopReply;
    MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET              portAvagoAaplGet;
    MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT           portEnableCtrlGet;
    MV_HWS_IPC_PORT_TX_OFFSETS_INFO_STRUCT            portSerdesTxParametersOffsetInfoCfg;

}MV_HWS_IPC_REPLY_MSG_DATA_TYPE;

typedef struct
{
    GT_U8                      devNum;
    GT_U8                      ctrlMsgType;
    GT_U8                      msgQueueId;
    GT_U8                      msgLength;
    MV_HWS_IPC_MSG_API_PARAMS  msgData;

 }MV_HWS_IPC_CTRL_MSG_STRUCT;


typedef struct
{
    GT_U32                         returnCode;
    GT_U32                         replyTo;
    MV_HWS_IPC_REPLY_MSG_DATA_TYPE readData;

}MV_HWS_IPC_REPLY_MSG_STRUCT;

typedef struct
{
    GT_U32 mvHwsHostRxMsgCount[MV_HWS_IPC_LAST_CTRL_MSG_TYPE];
    GT_U32 mvHwsHostTxMsgCount[MV_HWS_IPC_LAST_CTRL_MSG_TYPE];

    GT_U32 mvHwsPortIpcFailureCount[MV_HWS_IPC_MAX_PORT_NUM];
    GT_U32 mvHwsIpcGenFailureCount;

}MV_HWS_IPC_STATISTICS_STRUCT;

/**
* @internal mvHwsIpcSendRequestAndGetReply function
* @endinternal
*
* @brief   Gets reply from HW Services to the Host
*
* @param[in] devNum                   - device number
* @param[in] requestMsg               - pointer to request message
* @param[in] replyData                - pointer to reply message
* @param[in] msgLength                -  message length
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
    GT_U8                        devNum,
    MV_HWS_IPC_CTRL_MSG_STRUCT   *requestMsg,
    MV_HWS_IPC_REPLY_MSG_STRUCT  *replyData,
    GT_U32                       msgLength,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgId
);

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
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsIpcDefs_H */



