/********************************************************************************
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
* @file prvCpssPortManagerTypes.h
*
* @brief CPSS implementation for Port management types.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPortManagerTypes
#define __prvCpssPortManagerTypes

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/port/cpssPortManager.h>

/**
* @struct PRV_CPSS_PORT_MNG_SERDES_TXRX_PARAMS_STC
 *
 * @brief This structure contains serdes rxtx parameters
*/
typedef struct{

    /** rx and tx configuration structure */
    CPSS_PORT_SERDES_TUNE_STC serdesCfgDb;

    GT_BOOL rxValid;

    /** @brief whether or not rx parameters in the configuration are valid
     *  txValid - whether or not rx parameters in the configuration are valid
     *  Comments:
     *  None.
     */
    GT_BOOL txValid;

} PRV_CPSS_PORT_MNG_SERDES_TXRX_PARAMS_STC;

/**
* @struct PRV_CPSS_PORT_MNG_SERDES_POLARITY_PARAMS_STC
 *
 * @brief This structure contains serdes polarity configurations
*/
typedef struct{

    /** rx polarity */
    GT_BOOL rxInvert;

    /** @brief tx polarity
     *  Comments:
     *  None.
     */
    GT_BOOL txInvert;

} PRV_CPSS_PORT_MNG_SERDES_POLARITY_PARAMS_STC;

/**
* @struct PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC
 *
 * @brief This structure contains configurable fields for a serdes
*/
typedef struct{

    /** serdes rx and tx parameters */
    PRV_CPSS_PORT_MNG_SERDES_TXRX_PARAMS_STC portSerdesParams;

} PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC
 *
 * @brief This structure contains mandatory data a port need to have
 * in order for port manage to manage the port.
*/
typedef struct{

    /** interface mode */
    CPSS_PORT_INTERFACE_MODE_ENT ifModeDb;

    /** @brief speed
     *  Comments:
     *  None.
     */
    CPSS_PORT_SPEED_ENT speedDb;

} PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_AP_ATTRIBUTES_STC
 *
 * @brief This structure contains all the attributes an AP port can be configured with.
*/
typedef struct{

    /** FC pause (true/false) */
    GT_BOOL fcPause;

    /** FC assymetric direction (Annex 28B) */
    CPSS_PORT_AP_FLOW_CONTROL_ENT fcAsmDir;

    /** @brief GT_TRUE
     *  GT_FALSE - choose one of sides to be ceed according to protocol
     */
    GT_BOOL noneceDisable;

    /** @brief number of serdes lane of port where to run AP
     *  modesAdvertiseArr - array of pairs of (interface mode;speed) advertised by
     *  port for AP negotiation
     *  last entry must be
     *  (CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E)
     */
    GT_U32 laneNum;

    CPSS_PORT_FEC_MODE_ENT fecAbilityArr [CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];

    CPSS_PORT_FEC_MODE_ENT fecRequestedArr [CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];
    /*
       CPSS_PM_AP_PORT_ATTR_NONCE_E,
       CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E,
       CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E,
       CPSS_PM_AP_PORT_ATTR_LANE_NUM_E,
       CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E,
    */
    GT_U32  overrideAttrsBitMask;

} PRV_CPSS_PORT_MNG_AP_ATTRIBUTES_STC;

/**
* @struct PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC
 *
 * @brief This structure contains general port configuration parameters
*/
typedef struct{

    /** @brief loopback configuration for the port
     *  Comments:
     *  None.
     */
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC loopbackDbCfg;

} PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC
 *
 * @brief This structure contains all the attributes a port can be configured with.
*/
typedef struct{

    GT_U32 perPhyPortApNumOfModes;
    PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC perPhyPortDb[CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];
    /* 0-7 for mode=0, 1-8 for mode 1... index = lane + mode*PORT_MANAGER_MAX_LANES*/
    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC   perSerdesDb[CPSS_PORT_AP_IF_ARRAY_SIZE_CNS*PORT_MANAGER_MAX_LANES];

    /** @brief AP attributes of the port
     *  Comments:
     *  None.
     */
    PRV_CPSS_PORT_MNG_AP_ATTRIBUTES_STC apAttributesStc;

} PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_PORT_ATTRIBUTES_DB_STC
 *
 * @brief This structure contains all the attributes a port can be configured with.
*/
typedef struct{

    PRV_CPSS_PORT_MNG_PHY_PORT_SM_DB_STC **perPhyPortDbPtr;

    PRV_CPSS_PORT_MNG_SERDES_SM_DB_STC **perSerdesDbPtr;

    PRV_CPSS_PORT_MNG_GENERAL_PORT_CONFIGURATIONS_DB_STC **generalPortCfgPtr;

} PRV_CPSS_PORT_MNG_PORT_ATTRIBUTES_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_MAC_SM_DB_STC
 *
 * @brief This structure contains configurable fields for a serdes
*/
typedef struct{

    GT_U32 xlgMacDb[3];

    GT_U32 xlgMacDbFlag;

} PRV_CPSS_PORT_MNG_MAC_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_OPTICAL_CALIBRATION_DB_STC
 *
 * @brief This structure contains optical calibration port
 *        manager data base
*/
typedef struct
{
    CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT calibrationType;
    GT_U32 minLfThreshold;
    GT_U32 maxLfThreshold;
    GT_U32 minHfThreshold;
    GT_U32 maxHfThreshold;
    GT_U32 minEoThreshold;
    GT_U32 maxEoThreshold;
    GT_U32 confidenceBitMap;
    GT_BOOL opticalCalFinished[PORT_MANAGER_MAX_LANES];
    GT_BOOL overallOpticalCalFinished;
    GT_U32 opticalCalTimeoutSec;
    GT_U32 opticalCalTimerSecPrev;
    GT_U32 opticalCalTimerNsecNew;

} PRV_CPSS_PORT_MNG_CALIBRATION_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_PORT_SM_DB_STC
 *
 * @brief This structure contains all the data that port management use to manage to port state machine
*/
typedef struct{
    CPSS_PORT_MANAGER_PORT_TYPE_ENT portType;

    /* flag to store signal detected\lost indication */
    GT_BOOL signalDownHandled;

    /* train status per port */
    PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC trainCookiePtrEnclosingDB;

    /* signal stability algorithm timers and dbs */
    GT_U32 portTimeDiffFromLastTime;
    GT_U32 portTimeIntervalTimer;
    GT_U32 portTimerSecondsPrev;
    GT_U32 portTimerNsecPrev;
    GT_U32 portTotalTimeFromStart;
    GT_U32 portTotalTimeFromChange;

    /* train results timeout timers and dbs */
    GT_U32 portTrainResTimeDiffFromLastTime;
    GT_U32 portTrainResTimerSecondsPrev;
    GT_U32 portTrainResTimerNsecPrev;
    GT_U32 portTrainResTimeIntervalTimer;

    /* align-lock timers and dbs */
    GT_U32 portAlignTimeDiffFromLastTime;
    GT_U32 portAlignTimerSecondsPrev;
    GT_U32 portAlignTimerNsecPrev;
    GT_U32 portAlignTotalTimeFromStart;
    GT_U32 portAlignTimeIntervalTimer;
    GT_U32 portAlignTries;
    /* align-lock events port */
    GT_U32  Align_Num_Of_Events;

    /* flags per port to store signal stability and training statuses iteration process indicators */
    GT_U32 portDbStabilityDone;
    GT_U32 portDbTrainDone;
    GT_U32 portDbAlignLockDone;

    /* signal stabilization training timers and db */
    GT_U32 rxStabilityTrainDone;
    GT_U32 rxStabilityTrainTimeDiffFromLastTime;
    GT_U32 rxStabilityTrainTimerSecondsPrev;
    GT_U32 rxStabilityTrainTimerNsecPrev;
    GT_U32 rxStabilityTrainTotalTimeFromStart;
    GT_U32 rxStabilityTrainTries;

    GT_U8 userOverrideByteBitmap;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT userOverrideTrainMode;
    GT_BOOL userOverrideAdaptiveRxTrainSupported;
    GT_BOOL userOverrideEdgeDetectSupported;
    CPSS_PORT_FEC_MODE_ENT fecAbility;

    /* training results iteration timer per port */
    GT_U64 portWaitForTrainResultCounter;
    GT_U32 portWaitForTrainResultTries;

    /* whether port is under management */
    GT_BOOL portManaged;

    /* port state machine */
    CPSS_PORT_MANAGER_STATE_ENT portAdminSM;
    CPSS_PORT_MANAGER_FAILURE_ENT portLastFailureReason;
    /* operative enable\disable */
    GT_BOOL portOperDisableSM;
    /* override MAX_LF and MIN_LF */
    GT_U8 max_LF;
    GT_U8 min_LF;
    GT_BOOL overrideEtParams;
    /* whether port manager is responsible of unMasking events or not */
    CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskEventsMode;
    /* calibration parameters */
    PRV_CPSS_PORT_MNG_CALIBRATION_DB_STC calibrationMode;
    GT_BOOL disableMaskUnMask;
    GT_BOOL adaptiveStarted;

}PRV_CPSS_PORT_MNG_PORT_SM_DB_STC;

/**
* @struct PRV_CPSS_PORT_MNG_DB_STC
 *
 * @brief This structure contains all the data that port management use to manage it's ports
*/
typedef struct{

    PRV_CPSS_PORT_MNG_AP_PORT_ATTRIBUTES_DB_STC **portsApAttributedDb;

    /** @brief attributes of a ports. This structure contains customize fields
     *  that the user can configure on a port, for example: speed,
     *  interface mode, serdes polarity, etc.
     *  portMngSmDb     - this structure contains fields used to manage port state maching.
     *  portMngSmDbPerMac  - per mac database
     */
    PRV_CPSS_PORT_MNG_PORT_ATTRIBUTES_DB_STC portsAttributedDb;

    PRV_CPSS_PORT_MNG_PORT_SM_DB_STC **portMngSmDb;

    PRV_CPSS_PORT_MNG_MAC_SM_DB_STC **portMngSmDbPerMac;

    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC globalParamsCfg;

    /** @brief whether or not port manager API was used at least once.
     *  Comments:
     *  None.
     */
    GT_BOOL portManagerGlobalUseFlag;

} PRV_CPSS_PORT_MNG_DB_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPortManagerTypes */

