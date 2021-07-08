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
* @file cpssPortManager.h
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPortManager
#define __cpssPortManager

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>

#define PORT_MANAGER_ALL_PORT_LANES 0xFF
#define PORT_MANAGER_MAX_LANES      0x8

/*
        ** Port Manager State Machine Transitions
        ** +============================+=============+==================+=============+===============+=============+
        ** + Event \ State              +    Reset    +    Link_Down     + Init_In_    +    Link_Up    + Mac_Link_   +
        ** +                            +             +                  +   Progress  +               +     Down    +
        ** +============================+=============+==================+=============+===============+=============+
        ** +============================+=============+==================+=============+===============+=============+
        ** + Delete                     +    Reset    +      Reset       +    Reset    +     Reset     +    Reset    +
        ** +============================+=============+==================+=============+===============+=============+
        ** + Create                     +  Link_Down  +         -        +      -      +       -       +      -      +
        ** +============================+=============+==================+=============+===============+=============+
        ** + Admin Disable              +      -      +    Link_Down     +  Link_Down  +   Link_Down   +  Link_Down  +
        ** +============================+=============+==================+=============+===============+=============+
        ** + Admin Enable               +      -      +    Link_Down     +      -      +       -       +      -      +
        ** +============================+=============+==================+=============+===============+=============+
        ** + Low Level Up (SerDes\PCS)  +      -      + Init_In_progress +      -      +       -       +      -      +
        ** +============================+=============+==================+=============+===============+=============+
        ** + Low Level Down (SerDes\PCS)+      -      +    Link_Down     +  Link_Down  +  Link_Down    +  Link_Down  +
        ** +============================+=============+==================+=============+===============+=============+
        ** + Mac_Link_Up                +      -      +         -        +      -      +       -       +  Link_UP    +
        ** +============================+=============+==================+=============+===============+=============+
        ** + Mac_Link_Down              +      -      +         -        +      -      + Mac_Link_Down +      -      +
        ** +============================+=============+==================+=============+===============+=============+
*/

/**
* @enum CPSS_PORT_MANAGER_EVENT_ENT
 *
 * @brief Enumeration of the port management events which controls all port operations
*/
typedef enum{

    /** @brief Create the port based on the parameters configured with
     *  cpssPortMngPortParamsSet API
     */
    CPSS_PORT_MANAGER_EVENT_CREATE_E,   /* 0 */

    /** @brief Delete the port, all port units will be reset and port
     *  will be deleted from CPSS database
     */
    CPSS_PORT_MANAGER_EVENT_DELETE_E,   /* 1 */

    /** @brief Enable the port (port is enabled by default), all port
     *  units will be unreset if previous CPSS_PORT_MNG_EVENT_DISABLE_E
     *  was called
     */
    CPSS_PORT_MANAGER_EVENT_ENABLE_E,   /* 2 */

    /** @brief Disable the port, all port units will be reset but port will
     *  still exist in CPSS database. Applicable events after this
     *  event are only PORT_MANAGER_EVENT_ENABLE_E and
     *  PORT_MANAGER_EVENT_DELETE_E
     */
    CPSS_PORT_MANAGER_EVENT_DISABLE_E,   /* 3 */

    /** @brief Initialize the port. The port initialization operations are
     *  divided to segments and each segment is performed when this
     *  event is called. This event will cause the port to be in
     *  CPSS_PORT_MNG_STATE_INIT_IN_PROGRESS_E state. Port will
     *  exit this state when all initialization segments are finished,
     *  achieved by continues calls of CPSS_PORT_MNG_EVENT_INIT_E
     *  event
     */
    CPSS_PORT_MANAGER_EVENT_INIT_E,   /* 4 */

    /** @brief Notify CPSS on low level units (SerDes\PCS) status
     *  change. CPSS will update the port state accordingly
     *  and perform proper needed operations
     */
    CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E,   /* 5 */

    /** @brief Notify the port manager on MAC level status change.
     *  CPSS will update the port state accordingly and perform
     *  needed operations
     */
    CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E,   /* 6 */

    /** Notify CPSS on AP port resolution found */
    CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E,   /* 7 */

    CPSS_PORT_MANAGER_EVENT_TYPE_LAST_E

} CPSS_PORT_MANAGER_EVENT_ENT;

/**
* @enum CPSS_PORT_MANAGER_STATE_ENT
 *
 * @brief Enumeration of the port state within the port manager state machine (managed internally by CPSS).
*/
typedef enum{

    /** @brief Port is in reset state, all of the low level units (SerDes\PCS)
     *  and MAC units are in reset
     */
    CPSS_PORT_MANAGER_STATE_RESET_E,                /* 0 */

    /** @brief Port is in link down state, both the MAC unit and the low level
     *  units (SerDes\PCS) are unreset but down (for example, SerDes
     *  is in unreset but signal is not yet detected from a peer)
     */
    CPSS_PORT_MANAGER_STATE_LINK_DOWN_E,            /* 1 */

    /** @brief Port is in init state. In this state the port units HW states
     *  can be changed. The port will complete this state when sufficient
     *  calls to CPSS_PORT_MNG_EVENT_INIT_E event will be made and
     *  all initialization segments will be completed
     */
    CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E,     /* 2 */

    /** @brief Port is in link up state, all of the port units, both MAC and
     *  low level (SerDes\PCS) are up
     */
    CPSS_PORT_MANAGER_STATE_LINK_UP_E,              /* 3 */

    /** Port MAC link is down, but low level units (SerDes\PCS) are up */
    CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E,        /* 4 */

    /** @brief Port is under failure state, as a result of a failure occurred
     *  in PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state
     */
    CPSS_PORT_MANAGER_STATE_FAILURE_E,              /* 5 */

    CPSS_PORT_MANAGER_STATE_LAST_E

} CPSS_PORT_MANAGER_STATE_ENT;

/**
* @enum CPSS_PORT_MANAGER_FAILURE_ENT
 *
 * @brief Enumeration of the different port failure causes.
*/
typedef enum{

    /** No failure happened in port */
    CPSS_PORT_MANAGER_FAILURE_NONE_E,

    /** @brief Port stability filter failed. This failure means that the signal is
     *  not stable enough in order to establish a proper link
     */
    CPSS_PORT_MANAGER_FAILURE_SIGNAL_STABILITY_FAILED_E,

    /** @brief Port training failed, can be caused by loss of signal and other
     *  hardware faults
     */
    CPSS_PORT_MANAGER_FAILURE_TRAINING_FAILED_E,

    /** @brief Port lanes alignment not reached in the limit of the alignment timer
     *  this is relevant for multilane port modes, in which all of the
     *  port lanes should be aligned
     */
    CPSS_PORT_MANAGER_FAILURE_ALIGNMENT_TIMER_EXPIRED_E,
     /** @brief Port confidence interval timer expired, can be caused
     *   by putting incorrect data or the optical module is not
     *   calibrated
     */
    CPSS_PORT_MANAGER_FAILURE_CONFIDENCE_INTERVAL_TIMER_EXPIRED_E,

    CPSS_PORT_MANAGER_FAILURE_LAST_E

} CPSS_PORT_MANAGER_FAILURE_ENT;

/**
* @struct CPSS_PORT_MANAGER_STATUS_STC
 *
 * @brief This structure contains the information of the port current status
*/
typedef struct{

    /** @brief Current state of the port
     *  portAdministrativeDisabled - Whether or not the port is disabled (will be GT_TRUE if CPSS_PORT_MNG_EVENT_DISABLE_E
     *  event was called and no CPSS_PORT_MNG_EVENT_ENABLE_E event was called afterwards)
     */
    CPSS_PORT_MANAGER_STATE_ENT portState;

    GT_BOOL portUnderOperDisable;

    /** @brief Failure status of the port
     *  Comments:
     *  None.
     */
    CPSS_PORT_MANAGER_FAILURE_ENT failure;

    /** @brief next parameters defines the port configuration (in
     *  Link up state
     *  Comments: None.
     */

    /** @brief port interface mode
     *  Comments:
     *  None.
     */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;

    /** @brief port speed
     *  Comments:
     *  None.
     */
    CPSS_PORT_SPEED_ENT speed;

    /** @brief FEC type FC/RS/None
     *  Comments:
     *  None.
     */
    CPSS_PORT_FEC_MODE_ENT fecType;

} CPSS_PORT_MANAGER_STATUS_STC;

/**
* @struct CPSS_PORT_MANAGER_STC
 *
 * @brief This structure contains the port event parameters used to operate the port.
*/
typedef struct{

    /** @brief Event which is used to operate the port within the
     *  port manager state machine
     *  Comments:
     *  None.
     */
    CPSS_PORT_MANAGER_EVENT_ENT portEvent;

} CPSS_PORT_MANAGER_STC;

/**********************************************
        Port Parameters Management APIs
  ********************************************/

/**
* @enum CPSS_PORT_MANAGER_PORT_TYPE_ENT
 *
 * @brief Enumeration of different port types in CPSS.
*/
typedef enum{

    /** @brief Non 802.3AP port type
     *  CPSS_PORT_MANAGER_PORT_TYPE_AP_E    802.3AP port type
     */
    CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E,

    CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E,

    CPSS_PORT_MANAGER_PORT_TYPE_LAST_E

} CPSS_PORT_MANAGER_PORT_TYPE_ENT;


/**
* @enum CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT
 *
 * @brief Enumeration of different port loopback types
*/
typedef enum{

    /** MAC loopback type */
    CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E = 0,

    /** SERDES loopback type */
    CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E,

    CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E

} CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT;

/**
* @struct CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC
 *
 * @brief This structure contains port loopback configuration
*/
typedef struct{
    CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT     loopbackType;

    union {
        GT_BOOL                             macLoopbackEnable;
        CPSS_PORT_PCS_LOOPBACK_MODE_ENT     pcsLoopbackMode;
        CPSS_PORT_SERDES_LOOPBACK_MODE_ENT  serdesLoopbackMode;

    } loopbackMode;

    GT_BOOL                                 enableRegularTrafic;
} CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC;

/**
* @enum CPSS_PORT_MANAGER_CONFIDENCE_DATA_OFFSET_ENT
 *
 * @brief Enumeration of offsets in confidenceEnableBitMap
*/
typedef enum
{
    CPSS_PORT_MANAGER_MIN_LF_OFFSET_E,
    CPSS_PORT_MANAGER_MAX_LF_OFFSET_E,
    CPSS_PORT_MANAGER_MIN_HF_OFFSET_E,
    CPSS_PORT_MANAGER_MAX_HF_OFFSET_E,
    CPSS_PORT_MANAGER_MIN_EO_OFFSET_E,
    CPSS_PORT_MANAGER_MAX_EO_OFFSET_E

} CPSS_PORT_MANAGER_CONFIDENCE_DATA_OFFSET_ENT;

/**
* @enum CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT
 *
 * @brief Enumeration of different port loopback types
*/
typedef enum
{
    /* Default calibration - DAC,Copper */
    CPSS_PORT_MANAGER_DEFAULT_CALIBRATION_TYPE_E = 0,
    /* Optical calibration */
    CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E,

    CPSS_PORT_MANAGER_CALIBRATION_TYPE_LAST_E

} CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT;

/**
* @struct CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC
 *
 * @brief This structure contains port confidence interval data
 *        configuration
*/
typedef struct
{
    /* 0x1 to enable */
    GT_U32  lfMinThreshold;
    /* 0x2 to enable */
    GT_U32  lfMaxThreshold;
    /* 0x4 to enable */
    GT_U32  hfMinThreshold;
    /* 0x8 to enable */
    GT_U32  hfMaxThreshold;
    /* 0x10 to enable */
    GT_U32  eoMinThreshold;
    /* 0x20 to enable */
    GT_U32  eoMaxThreshold;
    /* bitMap enable */
    GT_U32  confidenceEnableBitMap;
    /* calibration time out [Seconds] */
    GT_U32 calibrationTimeOutSec;

} CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC;

/**
* @struct CPSS_PORT_MANAGER_CALIBRATION_MODE_STC
 *
 * @brief This structure contains port calibration configuration
*/
typedef struct
{
    CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT        calibrationType;
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC  confidenceCfg;

} CPSS_PORT_MANAGER_CALIBRATION_MODE_STC;

/**
* @enum CPSS_PORT_MANAGER_UNMASK_EVENTS_TYPE_ENT
 *
 * @brief Enumeration of unmask operation per port treatment,
 * defines whether to allow port manager unMask:
 * 1. MAC level EVENTS - CPSS_PP_PORT_LINK_STATUS_CHANGED_E.
 * 2. LOW level EVENTS - CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
 *                       CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
 *                       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E
*/
typedef enum
{
    /* Default calibration - Port Manager is responsible for unmasking LOW level and MAC level events */
    CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_ENABLE_MODE_E = 0,

    /* Port Manager is not responsible for unmasking any events, select this only if application handles the unmask operation */
    CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_DISABLE_MODE_E,

    /* Port Manager is not responsible to unmask only MAC level events, select this only if application handles the unmask operation of MAC level events */
    CPSS_PORT_MANAGER_UNMASK_MAC_LEVEL_EVENTS_DISABLE_MODE_E,

    /* Port Manager is not responsible to unmask only LOW level events, select this only if application handles the unmask operation of LOW level events*/
    CPSS_PORT_MANAGER_UNMASK_LOW_LEVEL_EVENTS_DISABLE_MODE_E,

    CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_LAST_E

}CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT;

/**
 * @enum CPSS_PORT_MANAGER_GLOBAL_PARAMS_TYPE_ENT
 *
 * @brief offset in CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC.globalParamsBitmapType
*/
typedef enum {

    /* signalDetectDbCfg 's offset */
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_SIGNAL_DETECT_E    =(1<<0),

    /* propHighSpeedPortEnabled 's offset */
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_HIGH_SPEED_PORTS_E =(1<<1)
} CPSS_PORT_MANAGER_GLOBAL_PARAMS_TYPE_ENT;

/**
 * @struct CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC
 *
 * @brief This structure contains signal detect configuration
*/
typedef struct{

        GT_U32  sdChangeInterval;
        GT_U32  sdChangeMinWindowSize;
        GT_U32  sdCheckTimeExpired;

} CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC;

typedef struct{
    /** @brief Bitmap to declare which structures are valid
    *  Comments:
    *  None.
    */
     GT_U32              globalParamsBitmapType;
     /** @brief signal detect configuration
     *  Comments:
     *  None.
     */
     CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC signalDetectDbCfg;
     /* indicates if 102G/107G ports are supported (valid for SIP_5_20 devices) */
     GT_BOOL                                    propHighSpeedPortEnabled;
} CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC;

#define CPSS_PORT_MANAGER_GLOBAL_PARAMETER_TYPE_CHECK_MAC(type, value) (type & value)

/* DEFINES */
#define CPSS_PM_AP_MAX_MODES                10
#define CPSS_PM_MAX_LANES                   8
#define CPSS_PM_MAGIC                       0x1a2bc3d4

/* MACROS for use with CPSS_PM_PORT_PARAMS_STC
 * -------------------------------------------
 * Use the below macros to tell PortManager that a particular attribute/parameter is valid,
 * in a call to ParamsSet:
 *
 * CPSS_PM_SET_VALID_<LANE_PARAM | LANE_AP_PARAM | ATTR | AP_ATTR >[_BITMASK]
 *
 * The _BITMASK macros accept a pointer to the GT_U32 bitmask variable
 * and the enum value corresponding to the attr/param type.
 *
 * The non _BITMASK macros accept a pointer to CPSS_PM_PORT_PARAMS_STC variable, (lane number if applicable),
 * and the enum value corresponding to the attr/param type.
 * ----------------------------------------------------------------------------------------------------------
 *
 * Use the below macros to check if a param/attr is set (i.e. valid) in the output of ParamsGet:
 *
 * CPSS_PM_<LANE_PARAM | LANE_AP_PARAM | ATTR | AP_ATTR>_IS_VALID[_BITMASK]
 *
 * The _BITMASK macros accept a GT_U32 variable on which the checks will be done
 * and the enum value corresponding to the attr/param type.
 *
 * The non _BITMASK macros accept a pointer to CPSS_PM_PORT_PARAMS_STC variable, (lane number if applicable),
 * and the enum value corresponding to the attr/param type.
 */

/* LANE PARAMS - NON-AP */
#define CPSS_PM_LANE_PARAM_IS_VALID(portParamsPtr, laneNum, laneParamType) \
    ((portParamsPtr)->portParamsType.regPort.laneParams[laneNum].validLaneParamsBitMask & laneParamType)

#define CPSS_PM_LANE_PARAM_IS_VALID_BITMASK(bitMask, laneParamType) \
    ((bitMask) & laneParamType)

#define CPSS_PM_SET_VALID_LANE_PARAM(portParamsPtr, laneNum, laneParamType) \
    ((portParamsPtr)->portParamsType.regPort.laneParams[laneNum].validLaneParamsBitMask |= laneParamType)

#define CPSS_PM_SET_VALID_LANE_PARAM_BITMASK(bitMaskPtr, laneParamType) \
    ((*bitMaskPtr) |= laneParamType)


/* LANE PARAMS - AP */
#define CPSS_PM_LANE_AP_PARAM_IS_VALID(portParamsPtr, mode, laneNum, laneParamType) \
    ((portParamsPtr)->portParamsType.apPort.modesArr[mode].laneParams[laneNum].validLaneParamsBitMask & laneParamType)

#define CPSS_PM_LANE_AP_PARAM_IS_VALID_BITMASK(bitMask, laneParamType) \
    ((bitMask) & (laneParamType))

#define CPSS_PM_SET_VALID_LANE_AP_PARAM(portParamsPtr, mode, laneNum, laneParamType) \
    ((portParamsPtr)->portParamsType.apPort.modesArr[mode].laneParams[laneNum].validLaneParamsBitMask |= laneParamType)

#define CPSS_PM_SET_VALID_LANE_AP_PARAM_BITMASK(bitMaskPtr, laneParamType) \
    ((*bitMaskPtr) |= laneParamType)


/* ATTRS - NON-AP */
#define CPSS_PM_ATTR_IS_VALID(portParamsPtr, attrType) \
    ((portParamsPtr)->portParamsType.regPort.portAttributes.validAttrsBitMask & attrType)

#define CPSS_PM_ATTR_IS_VALID_BITMASK(bitMask, attrType) \
    (bitMask & attrType)

#define CPSS_PM_SET_VALID_ATTR(portParamsPtr, attrType) \
    ((portParamsPtr)->portParamsType.regPort.portAttributes.validAttrsBitMask |= attrType)

#define CPSS_PM_SET_VALID_ATTR_BITMASK(bitMaskPtr, attrType) \
    ((*bitMaskPtr) |=  attrType)


/* ATTRS - AP */
#define CPSS_PM_AP_ATTR_IS_VALID(portParamsPtr, attrType) \
    ((portParamsPtr)->portParamsType.apPort.apAttrs.validAttrsBitMask & attrType)

#define CPSS_PM_SET_VALID_AP_ATTR(portParamsPtr, attrType) \
    ((portParamsPtr)->portParamsType.apPort.apAttrs.validAttrsBitMask |=  attrType)

#define CPSS_PM_AP_ATTR_IS_VALID_BITMASK(bitMask, attrType) \
    (bitMask & attrType)

/* ATTRS (OVERRIDE)- AP */
#define CPSS_PM_AP_ATTR_OVERRIDE_IS_VALID(portParamsPtr, speedLane, lane, attrType) \
    ((portParamsPtr)->portParamsType.apPort.apAttrs.overrideLaneParams[speedLane][lane].validOverrideBitMask & attrType)

#define CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(portParamsPtr, speedLane, lane, attrType) \
    ((portParamsPtr)->portParamsType.apPort.apAttrs.overrideLaneParams[speedLane][lane].validOverrideBitMask |=  attrType)

#define CPSS_PM_AP_ATTR_OVERRIDE_IS_VALID_BITMASK(bitMask, attrType) \
    (bitMask & attrType)


/* Regular Port Params */

/*
 * @brief bit-to-param mapping for CPSS_PM_MODE_LANE_PARAM_STC.validLaneParamsBitMask
 */
typedef enum {

  CPSS_PM_LANE_PARAM_GLOBAL_LANE_NUM_E      =0x1,
  CPSS_PM_LANE_PARAM_TX_E                   =0x2,
  CPSS_PM_LANE_PARAM_RX_E                   =0x4

} CPSS_PM_LANE_PARAM_TYPE_ENT;

typedef struct {
  /*  Bitmask which tells which all of the below params are valid (1 bit per param) */
  /*  Governed by CPSS_PM_LANE_PARAM_TYPE_ENT */
  GT_U32                                      validLaneParamsBitMask;

  GT_U16                                      globalLaneNum;
  CPSS_PORT_SERDES_TX_CONFIG_STC              txParams;
  CPSS_PORT_SERDES_RX_CONFIG_STC              rxParams;

} CPSS_PM_MODE_LANE_PARAM_STC;

typedef struct {

  GT_U8    minLF;
  GT_U8    maxLF;

} CPSS_PM_ET_OVERRIDE_CFG_STC;

/*
 * @brief bit-to-attr mapping for CPSS_PM_PORT_ATTR_STC.validAttrsBitMask
 */
typedef enum {
  /* Which tune mode should be used in place of the CPSS default tune */
  CPSS_PM_ATTR_TRAIN_MODE_E             =0x1,

  /* Whether Adaptive Rx Tuning is supported or not */
  CPSS_PM_ATTR_RX_TRAIN_SUPP_E          =0x2,

  /* Whether edge detect is supported or not */
  CPSS_PM_ATTR_EDGE_DETECT_SUPP_E       =0x4,

  /* Loopback mode - one of MAC, PCS or SerDes loopback mode. */
  CPSS_PM_ATTR_LOOPBACK_E               =0x8,

  /* Params to override the default ones during Enhanced Tuning */
  CPSS_PM_ATTR_ET_OVERRIDE_E            =0x10,

  /* Forward Error Correction mode : Reed-Solomon(clause 91)/ Base-R(clause 74)/ None/ Both */
  CPSS_PM_ATTR_FEC_MODE_E               =0x20,

  /* Calibration - Cu or Optical */
  CPSS_PM_ATTR_CALIBRATION_E            =0x40,

  /* Whether to allow PortManager to unMask MAC and/or LOW level events */
  CPSS_PM_ATTR_UNMASK_EV_MODE_E         =0x80

} CPSS_PM_PORT_ATTR_TYPE_ENT;

typedef struct {

  /*  Bitmask which tells which all of the below attributes are valid (1 bit per param) */
  /*  Governed by CPSS_PM_PORT_ATTR_TYPE_ENT */
  GT_U32                                        validAttrsBitMask;

  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT           trainMode;
  GT_BOOL                                       adaptRxTrainSupp;
  GT_BOOL                                       edgeDetectSupported;
  CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC         loopback;
  CPSS_PM_ET_OVERRIDE_CFG_STC                   etOverride;
  CPSS_PORT_FEC_MODE_ENT                        fecMode;
  CPSS_PORT_MANAGER_CALIBRATION_MODE_STC        calibrationMode;
  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT      unMaskEventsMode;

} CPSS_PM_PORT_ATTR_STC;


/* AP Port Params */


typedef struct {

  /* Advertised ifMode */
  CPSS_PORT_INTERFACE_MODE_ENT            ifMode;

  /* Advertised speed */
  CPSS_PORT_SPEED_ENT                     speed;

  /* Forward Error Correction supported by this port */
  CPSS_PORT_FEC_MODE_ENT                  fecSupported;

  /* Forward Error Correction requested to the peer */
  CPSS_PORT_FEC_MODE_ENT                  fecRequested;

} CPSS_PM_AP_PORT_ADV_STC;

/**
 * @brief enum used as 1st index into CPSS_PM_AP_PORT_ATTR_STC.overrideLaneParams[][]
 *        to store the parameters corres. to the serdes speeds - 10G and 25G
 *
 */
typedef enum
{
    CPSS_PM_AP_LANE_SERDES_SPEED_10000_E, /* 0 */
    CPSS_PM_AP_LANE_SERDES_SPEED_25000_E, /* 1 */
    CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E   /* 2 */
} CPSS_PM_AP_LANE_SERDES_SPEED;

/*
 * @brief bit-to-param mapping for CPSS_PM_AP_LANE_OVERRIDE_PARAM_STC.validOverrideBitMask
 */
typedef enum {

  CPSS_PM_AP_LANE_OVERRIDE_TX_OFFSET_E      =0x1,
  CPSS_PM_AP_LANE_OVERRIDE_RX_E             =0x2,
  CPSS_PM_AP_LANE_OVERRIDE_ETL_E            =0x4

#if 0
  /* txAmpOffset */
  CPSS_PM_AP_LANE_OVERRIDE_TX_AMP_OFFSET_E        =0x1,

  /* txEmph0Offset */
  CPSS_PM_AP_LANE_OVERRIDE_TX_EMPH0_OFFSET_E      =0x2,

  /* txEmph1Offset */
  CPSS_PM_AP_LANE_OVERRIDE_TX_EMPH1_OFFSET_E      =0x4,

  /* squelch */
  CPSS_PM_AP_LANE_OVERRIDE_RX_SQUELCH_E           =0x8,

  /* lowFrequency */
  CPSS_PM_AP_LANE_OVERRIDE_RX_LF_E                =0x10,

  /* highFrequency*/
  CPSS_PM_AP_LANE_OVERRIDE_RX_HF_E                =0x20,

  /* dcGain */
  CPSS_PM_AP_LANE_OVERRIDE_RX_DC_GAIN_E           =0x40,

  /* bandWidth */
  CPSS_PM_AP_LANE_OVERRIDE_RX_BW_E                =0x80,

  /* loopBandwidth */
  CPSS_PM_AP_LANE_OVERRIDE_RX_LOOP_BW_E           =0x100,

  /* etlMinDelay - Enhanced Tune Lite */
  CPSS_PM_AP_LANE_OVERRIDE_ETL_MIN_DELAY_E          =0x200,

  /* etlMaxDelay - Enhanced Tune Lite */
  CPSS_PM_AP_LANE_OVERRIDE_ETL_MAX_DELAY_E          =0x400
#endif

} CPSS_PM_AP_LANE_OVERRIDE_PARAM_TYPE_ENT;

typedef struct {

  /*  Bitmask which tells which all of the below params are valid (1 bit per param) */
  /*  Governed by CPSS_PM_AP_LANE_OVERRIDE_PARAM_TYPE_ENT */
  GT_U32    validOverrideBitMask;

  GT_8      txAmpOffset;
  GT_8      txEmph0Offset;
  GT_8      txEmph1Offset;

  GT_U16    squelch;
  GT_U8     lowFrequency;
  GT_U8     highFrequency;
  GT_U8     dcGain;
  GT_U8     bandWidth;
  GT_U8     loopBandwidth;

  GT_U8     etlMinDelay;
  GT_U8     etlMaxDelay;

} CPSS_PM_AP_LANE_OVERRIDE_PARAM_STC;


/*
 * @brief bit-to-param mapping for CPSS_PM_AP_PORT_ATTR_STC.validAttrsBitMask
 */
typedef enum {

  /* nonceDisable - whether to disable the NONCE number send and reception during AP */
  CPSS_PM_AP_PORT_ATTR_NONCE_E            =0x01,

  /* fcPause - IEEE 802.3X Ethernet PAUSE is enabled or not */
  CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E         =0x02,

  /* fcAsmDir - IEEE 802.3X Ethernet PAUSE - symmetric or asymetric flow control */
  CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E       =0x04,

  /* negotiationLaneNum */
  CPSS_PM_AP_PORT_ATTR_LANE_NUM_E         =0x08,

  /*
   *  Lane Properties (Tx/Rx/Polarity) which have to be overridden for certain serdesSpeeds alone
   *  2D array - 1st index tells which serdes speed (enum:CPSS_PM_AP_LANE_SERDES_SPEED)
   *             2nd index which serdes lane's params
   */
  CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E  =0x10

} CPSS_PM_AP_PORT_ATTR_TYPE_ENT;


typedef struct {

  /*  Bitmask which tells which all of the below attrs  are valid (1 bit per param) */
  /*  Governed by CPSS_PM_AP_PORT_ATTR_TYPE_ENT */
  GT_U32                                  validAttrsBitMask;

  GT_BOOL                                 nonceDisable;
  GT_BOOL                                 fcPause;
  CPSS_PORT_AP_FLOW_CONTROL_ENT           fcAsmDir;
  GT_U32                                  negotiationLaneNum;
  CPSS_PM_AP_LANE_OVERRIDE_PARAM_STC      overrideLaneParams [CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E][CPSS_PM_MAX_LANES];
} CPSS_PM_AP_PORT_ATTR_STC;

/**

  CPSS_PM_PORT_PARAMS_STC - the structure to be used by application to tell PortManager,
  all configurable properties of a port.

  The two APIs that has this structure as a parameter (and to be used by application) are -
  1) GT_STATUS cpss[DxCh | Px]PortManagerPortParamsSet
     (
        IN  GT_U8                                  devNum,
        IN  GT_PHYSICAL_PORT_NUM                   portNum,
        IN  CPSS_PM_PORT_PARAMS_STC               *portParamsStcPtr
      );

  2) GT_STATUS cpss[DxCh | Px]PortManagerPortParamsGet
     (
       IN  GT_U8                                  devNum,
       IN  GT_PHYSICAL_PORT_NUM                   portNum,
       INOUT  CPSS_PM_PORT_PARAMS_STC            *portParamsStcPtr
     );

  NOTE:  The structure is monolithic. i.e. no pointers inside. So you can either malloc/ define a variable
         and no further allocations are needed. All members are accessible.

         Before passing a pointer of the struct to ..ParamsSet, the user has to pass it to

         GT_STATUS cpss[DxCh | Px]PortManagerPortParamsStructInit
         (
            INOUT  CPSS_PM_PORT_PARAMS_STC *portParamsStcPtr,
            IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType
         );

         to make sure it's initialised properly. Else, the call to ...ParamsSet will fail.
         The user should populate the structure with the desired values, !!ONLY!! after the above call.


  Structure of the structure!
  ===========================
  Typical/ Eg. usage:

  Please refer:
  GT_STATUS cpss[DxCh | Px]SamplePortManagerMandatoryParamsSet
  (
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
  );

  > Regular Port Params
  ----------------------
  typedef struct {
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_SPEED_ENT                     speed;

    CPSS_PM_MODE_LANE_PARAM_STC             laneParams[CPSS_PM_MAX_LANES];
    CPSS_PM_PORT_ATTR_STC                   portAttributes;
  } CPSS_PM_PORT_REG_PARAMS_STC;

  A regular port's params are of three kinds -
  1) mandatory
     a) ifMode
     b) speed

  2) non-mandatory
     a) port properties
        portAttributes

     b) SerDes lane properties
        Array with individual entry for each lane - laneParams[CPSS_PM_MAX_LANES]
        Per port, the max possible number of lanes is CPSS_PM_MAX_LANES.
        All 8 may not be used for all ifMode/speed combinations.

  > Autoneg (AP) Port Params
  ---------------------------
  typedef struct {
     GT_U32                                numOfModes;
     CPSS_PM_AP_PORT_ADV_STC               modesArr[CPSS_PM_AP_MAX_MODES];
     CPSS_PM_AP_PORT_ATTR_STC              apAttrs;
   } CPSS_PM_AP_PORT_PARAMS_STC;

  The AP Port params are
  1) The different (ifMode + speed) modes that the port supports and has to advertise.
     We can advertise a maximum of CPSS_PM_AP_MAX_MODES modes.
     The i-th advertised mode's params are accessible via - modesArr[i].

     The actual number of advertised modes is stored in "numOfModes".

  2) The properties which are independent of the advertised mode.
     Accessible via apAttrs

*/

typedef struct {

  /* number of modes advertised */
  GT_U32                                numOfModes;

  /* array of advertised modes. Max advertisements - CPSS_PM_AP_MAX_MODES */
  /* i-th index has the i-th advertisement details */
  CPSS_PM_AP_PORT_ADV_STC               modesArr[CPSS_PM_AP_MAX_MODES];

  /* struct of AP attrs (indep. of advertised modes) */
  CPSS_PM_AP_PORT_ATTR_STC              apAttrs;

} CPSS_PM_AP_PORT_PARAMS_STC;

typedef struct {

  /* interface mode of the port */
  CPSS_PORT_INTERFACE_MODE_ENT            ifMode;

  /* speed of the port */
  CPSS_PORT_SPEED_ENT                     speed;

  /* array to store the params specific to each serdes lanes. max lanes = CPSS_PM_MAX_LANES */
  CPSS_PM_MODE_LANE_PARAM_STC             laneParams[CPSS_PM_MAX_LANES];

  /* struct of port attrs/properties (not related to serdes lanes) */
  CPSS_PM_PORT_ATTR_STC                   portAttributes;

} CPSS_PM_PORT_REG_PARAMS_STC;

typedef union {

  /* to be used if portType is regular */
  CPSS_PM_PORT_REG_PARAMS_STC             regPort;

  /* to be used if portType is AP */
  CPSS_PM_AP_PORT_PARAMS_STC              apPort;

} CPSS_PM_PORT_PARAMS_TYPE_UNION;

typedef struct {

  /* regular or AP */
  CPSS_PORT_MANAGER_PORT_TYPE_ENT           portType;

  /* magic field used for validation */
  GT_U32                                    magic;

  /* union to be used based on portType */
  CPSS_PM_PORT_PARAMS_TYPE_UNION            portParamsType;

} CPSS_PM_PORT_PARAMS_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortManager */


