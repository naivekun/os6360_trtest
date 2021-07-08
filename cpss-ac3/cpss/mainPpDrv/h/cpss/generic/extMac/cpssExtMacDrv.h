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
* @file cpssExtMacDrv.h
*
* @brief
* CPSS API that that will have PHY MAC extensions.
*
* ------------------------------------------------------------------------
* 1  cpssDxChPortSpeedSet
* 2  cpssDxChPortSpeedGet
* 3  cpssDxChPortDuplexAutoNegEnableSet
* 4  cpssDxChPortDuplexAutoNegEnableGet
* 5  cpssDxChPortFlowCntrlAutoNegEnableSet
* 6  cpssDxChPortFlowCntrlAutoNegEnableGet
* 7  cpssDxChPortSpeedAutoNegEnableSet
* 8  cpssDxChPortSpeedAutoNegEnableGet
* 9  cpssDxChPortFlowControlEnableSet
* 10  cpssDxChPortFlowControlEnableGet
* 11  cpssDxChPortPeriodicFcEnableSet
* 12  cpssDxChPortPeriodicFcEnableGet
* 13  cpssDxChPortBackPressureEnableSet
* 14  cpssDxChPortBackPressureEnableGet
* 15  cpssDxChPortLinkStatusGet
* 16  cpssDxChPortDuplexModeSet
* 17  cpssDxChPortDuplexModeGet
* 18  cpssDxChPortEnableSet
* 19  cpssDxChPortEnableGet
* 20  cpssDxChPortExcessiveCollisionDropEnableSet
* 21  cpssDxChPortExcessiveCollisionDropEnableSet
* 22  cpssDxChPortPaddingEnableSet
* 23  cpssDxChPortPaddingEnableGet
* 24  cpssDxChPortPreambleLengthSet
* 25  cpssDxChPortPreambleLengthGet
* 26  cpssDxChPortCrcCheckEnableSet
* 27  cpssDxChPortCrcCheckEnableGet
* 28  cpssDxChPortMruSet
* 29  cpssDxChPortMruGet
*
* ------------------------------------------------------------------------
*
* @version   4
********************************************************************************
*/
#ifndef __cpssExtMacDrvH
#define __cpssExtMacDrvH

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/generic/cscd/cpssGenCscd.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortEee.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>


#define PRV_CPSS_PHY_MAC_OBJ(devNum,portNum) \
       (portNum == CPSS_CPU_PORT_NUM_CNS && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) ? \
         NULL : PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portMacObjPtr

/**
* @enum CPSS_MACDRV_STAGE_ENT
 *
 * @brief Enumeration of port callback function stages
*/
typedef enum{

    /** first position - before switch mac code run */
    CPSS_MACDRV_STAGE_PRE_E,

    /** second position - after switch mac code run */
    CPSS_MACDRV_STAGE_POST_E

} CPSS_MACDRV_STAGE_ENT;


/*
 * typedef: GT_STATUS (*CPSS_MACDRV_MAC_SPEED_SET_FUNC)
 *
 * Description: defines speed set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   CPSS_PORT_SPEED_ENT     speed   - speed value
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - (pointer to) the parameter defines if the switch MAC
 *                                                    will be configured
 *   CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr - speed value for switch MAC
 *
 *  COMMENTS: (1) Speed set callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_MAC_SPEED_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT     speed,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_MAC_SPEED_GET_FUNC)
 *
 * Description: defines speed get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   OUT CPSS_PORT_SPEED_ENT *speedPtr           - poiter to return speed value
 *   GT_BOOL         *doPpMacConfigPtr              - (pointer to) the parameter defines if the switch MAC
 *                                                    will be configured
 *   CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr   - speed value for switch MAC
 *
 *  COMMENTS: (2) Speed get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_MAC_SPEED_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_SPEED_ENT     *speedPtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_DUPLEX_AN_SET_FUNC)
 *
 * Description: defines AN duplex set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   GT_BOOL                 state   - duplex state
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                 *doPpMacConfigPtr     - (pointer to)the parameter defines if the switch MAC
 *                                                   will be configurated
 *   CPSS_PORT_SPEED_ENT     *switchDuplexANSetPtr - duplex value for switch MAC
 *
 *  COMMENTS: (3) Duplex AN Set callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_DUPLEX_AN_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL              *  doPpMacConfigPtr,
    OUT GT_BOOL              *  switchDuplexANSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_DUPLEX_AN_GET_FUNC)
 *
 * Description: defines AN duplex get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                 *statePtr         - pointer to return duplex value
 *   GT_BOOL                 *doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC
 *                                               will be configurated
 *   CPSS_PORT_SPEED_ENT     *switchDuplexANGetPtr  - duplex value for switch MAC
 *
 *  COMMENTS: (4) Duplex AN Get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_DUPLEX_AN_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL                 *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchDuplexANGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FLOW_CNTL_AN_SET_FUNC)
 *
 * Description: defines AN flow control set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   GT_BOOL                 state   - flow control state
 *   GT_BOOL                 pauseAdvertise  - pause advertise state
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                 *targetState           - pointer to return flow control state
 *   GT_BOOL                 *targetPauseAdvertise  - pointer to return pause advertise state
 *   GT_BOOL                 *doPpMacConfigPtr      - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (5) Flow control AN Set
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FLOW_CNTL_AN_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *targetState,
    OUT  GT_BOOL                *targetPauseAdvertise
);
/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FLOW_CNTL_AN_GET_FUNC)
 *
 * Description: defines AN flow control get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                 *statePtr             - FC state pointer to return the value
 *   GT_BOOL                 *pauseAdvertisePtr    - pause advertise pointer to return the value
 *   GT_BOOL                 *targetState          - pointer to return switch MAC flow control state
 *   GT_BOOL                 *targetPauseAdvertise - pointer to return switch MAC pause advertise state
 *   GT_BOOL                 *doPpMacConfigPtr     - pointer to the parameter defines if the switch MAC
 *                                                   will be configurated
 *  COMMENTS: (6) Flow control AN Get
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FLOW_CNTL_AN_GET_FUNC)
(

    IN   GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL                *statePtr,
    OUT  GT_BOOL                *pauseAdvertisePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    IN GT_BOOL                 *doPpMacConfigPtr,
    OUT  GT_BOOL                *targetStatePtr,
    OUT  GT_BOOL                *targetPauseAdvertisePtr
);
/*
 *  typedef GT_STATUS (*CPSS_MACDRV_SPEED_AN_SET_FUNC)
 *
 * Description: defines AN speed set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   GT_BOOL                 state   - speed AN state
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *switchSpeedANSetPtr - AN speed pointer to value to set in switch MAC
 *   GT_BOOL                 *doPpMacConfigPtr - pointer to the parameter defines if the switch MAC
 *                                               will be configurated
 *
 *  COMMENTS: (7) Speed AN Set callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_SPEED_AN_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_SPEED_AN_GET_FUNC)
 *
 * Description: defines AN speed get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum  - device number
 *   GT_U8                   portNum - port number
 *   GT_BOOL                 state   - speed AN state
 *   CPSS_MACDRV_STAGE_ENT    stage  - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
  *  GT_BOOL                *statePtr            - pointer to return value
 *   GT_BOOL                *switchSpeedANGetPtr - AN speed pointer to value from switch MAC
 *   GT_BOOL                 *doPpMacConfigPtr - pointer to the parameter defines if the switch MAC
 *                                               will be configurated
 *
 *  COMMENTS: (8) Speed AN Get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_SPEED_AN_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL                *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANGetPtr
);
/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FC_ENABLE_SET_FUNC)
 *
 * Description: defines AN FC set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_PORT_FLOW_CONTROL_ENT  state - AN FC state
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *switchFlowCntlSetPtr   - AN FC pointer to value for switch MAC
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (9)  Flow control Enable Set callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FC_ENABLE_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT  *switchFlowCntlSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FC_ENABLE_GET_FUNC)
 *
 * Description: defines AN FC get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_PORT_FLOW_CONTROL_ENT  statePtr           - AN FC state pointer to return value
 *   GT_BOOL                *switchFlowCntlGetPtr   - AN FC pointer to value from switch MAC
 *   GT_BOOL                 *doPpMacConfigPtr      - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (10)  Flow control Enable Get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FC_ENABLE_GET_FUNC)
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr,
    IN CPSS_MACDRV_STAGE_ENT            stage,
    OUT GT_BOOL                         *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *switchFlowCntlGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_CN_FC_TIMER_SET_FUNC)
 *
 * Description: defines CN FC timer set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_U32                 index - SPD index (0..3)
 *   GT_U32                 timer    - timer value (1..0xfffe)
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_CN_FC_TIMER_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      index,
    IN  GT_U32                      timer,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_CN_FC_TIMER_GET_FUNC)
 *
 * Description: defines CN FC timer get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_U32                 index - SPD index (0..3)
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_U32                 timerPtr    - (Pointer to) timer value (1..0xfffe)
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_CN_FC_TIMER_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      index,
    OUT GT_U32                      *timerPtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FC_MODE_SET_FUNC)
 *
 * Description: defines FC mode set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_DXCH_PORT_FC_MODE_ENT    fcMode - Flow Control Mode (either 802.3x or PFC)
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FC_MODE_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT  fcMode,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FC_MODE_GET_FUNC)
 *
 * Description: defines FC mode get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_DXCH_PORT_FC_MODE_ENT    fcModePtr - Flow Control Mode (either 802.3x or PFC)
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FC_MODE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT *fcModePtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FC_PARAMS_SET_FUNC)
 *
 * Description: defines FC parameters set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_U32    xOffThreshold - Flow Control Xoff threshold per port
 *   GT_U32    xOnThreshold - Flow Control Xon threshold per port
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FC_PARAMS_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      xOffThreshold,
    IN  GT_U32                      xOnThreshold,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_FC_PARAMS_GET_FUNC)
 *
 * Description: defines FC parameters get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_U32    *xOffThresholdPtr - Flow Control Xoff threshold per port
 *   GT_U32    *xOnThresholdPtr - Flow Control Xon threshold per port
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_FC_PARAMS_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      *xOffThresholdPtr,
    IN  GT_U32                      *xOnThresholdPtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PERIODIC_FC_ENABLE_SET_FUNC)
 *
 * Description: defines periodic FC enable set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum   - device number
 *   GT_U8                   portNum  - port number
 *   CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT                 enable   - enable/disable value
 *   CPSS_MACDRV_STAGE_ENT   stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *switchPeriodicFlowCntlSetPtr  - periodic FC pointer to value from switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (11)  Periodic Flow control Enable Set
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PERIODIC_FC_ENABLE_SET_FUNC)
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT    enable,
    IN  CPSS_MACDRV_STAGE_ENT                       stage,
    OUT GT_BOOL                                    *doPpMacConfigPtr,
    OUT GT_BOOL                                    *switchPeriodicFlowCntlSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PERIODIC_FC_ENABLE_GET_FUNC)
 *
 * Description: defines periodic FC enable get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum   - device number
 *   GT_U8                   portNum  - port number
 *   CPSS_MACDRV_STAGE_ENT   stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *enablePtr                     - enable/disable return value pointer
 *   GT_BOOL         *switchPereodicFlowCntlGetPtr  - periodic FC pointer to value of switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (12) Periodic Flow control Enable Get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PERIODIC_FC_ENABLE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                     *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchPereodicFlowCntlGetPtr
);
/*
 *  typedef GT_STATUS (*CPSS_MACDRV_BP_ENABLE_SET_FUNC)
 *
 * Description: defines back pressure enable set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_BOOL                 state     - enable/disable value
 *   CPSS_MACDRV_STAGE_ENT   stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *switchBPGetPtr                - AN speed pointer to value from switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (13)  BackPressure Enable Set callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_BP_ENABLE_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                     state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPSetPtr
);
/*
 *  typedef GT_STATUS (*CPSS_MACDRV_BP_ENABLE_GET_FUNC)
 *
 * Description: defines Back pressure enable/disable get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *statePtr                     - enable/disable return value pointer
 *   GT_BOOL         *switchBPGetPtr                - AN speed pointer to value from switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (14) Back pressure enable/disable Get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_BP_ENABLE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                     *statePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_LINK_STATUS_GET_FUNC)
 *
 * Description: defines link status get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *isLinkUpPtr                   - link status return value pointer
 *   GT_BOOL         *switchLinkStatusGetPtr        - link status pointer to value of switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (15) Port Link Status Get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_LINK_STATUS_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL                    *isLinkUpPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchLinkStatusGetPtr
);
/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_DUPLEX_SET_FUNC)
 *
 * Description: defines port duplex status set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_PORT_DUPLEX_ENT    dMode     - duplex mode
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *switchDuplexSetPtr            - link status pointer to value from switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (16) Set Port Duplex Mode callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_DUPLEX_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DUPLEX_ENT        dMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
);
/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_DUPLEX_GET_FUNC)
 *
 * Description: defines port duplex status get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_PORT_DUPLEX_ENT    *dModePtr              - pointer to duplex mode value
 *   GT_BOOL         *switchDuplexGetPtr            - duplex mode pointer to value from switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (17) Get Port Duplex Mode callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_DUPLEX_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_DUPLEX_ENT        *dModePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_ENABLE_SET_FUNC)
 *
 * Description: defines port enable/disable set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_BOOL                 enable    - enable/disable state
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *targetEnableSetPtr            - port status pointer to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (18) Set Port enable/disable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_ENABLE_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                     enable,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *targetEnableSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_ENABLE_GET_FUNC)
 *
 * Description: defines port enable/disable set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *statePtr                      - state pointer
 *   GT_BOOL         *switchPortEnableGetPtr        - port status pointer to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (19) Get Port enable/disable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_ENABLE_GET_FUNC)
(
    IN   GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL               *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchPortEnableGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_EXCL_COL_DROP_ENABLE_SET_FUNC)
 *
 * Description: defines port excessive collisions drop set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_BOOL                 enable    - enable/disable state
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *switchExcColDropSetPtr        - port excessive collisions drop
 *                                                    pointer to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (20) Set Port Excessive Collisions Drop Enable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_EXCL_COL_DROP_ENABLE_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchExcColDropSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_EXCL_COL_DROP_ENABLE_GET_FUNC)
 *
 * Description: defines port excessive collisions drop get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *enablePtr                     - pointer to return value
 *   GT_BOOL         *switchExcColDropGetPtr        - port excessive collisions drop
 *                                                    pointer to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (21) Get Port Excessive Collisions Drop Enable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_EXCL_COL_DROP_ENABLE_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchExcColDropGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PADDING_ENABLE_SET_FUNC)
 *
 * Description: defines padding enable/disable set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_BOOL                 enable    - set value
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *switchPaddingSetPtr           - padding enable/disable pointer
 *                                                    to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (22) Set Padding Enable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PADDING_ENABLE_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchPaddingSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PADDING_ENABLE_SET_FUNC)
 *
 * Description: defines padding enable/disable set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         * enablePtr                    - pointer to return value
 *   GT_BOOL         *switchPaddingGetPtr           - padding enable/disable pointer
 *                                                    to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (23) Get padding enable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PADDING_ENABLE_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchPaddingGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PREAMBLE_LENGTH_SET_FUNC)
 *
 * Description: defines padding enable/disable set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                    devNum    - device number
 *   GT_U8                    portNum   - port number
 *   CPSS_PORT_DIRECTION_ENT  direction - Rx or Tx or Both
 *   GT_U32                   length    - value
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_PORT_DIRECTION_ENT *targetDirection       - pointer to return value
 *
 *   GT_BOOL         *switchPreambleLengthPtr       - preamble length pointer
 *                                                    to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (24) Set Preample length callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PREAMBLE_LENGTH_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT *targetDirection,
    OUT GT_U32                 *switchPreambleLengthPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PREAMBLE_LENGTH_GET_FUNC)
 *
 * Description: defines get preamble length callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                    devNum    - device number
 *   GT_U8                    portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_PORT_DIRECTION_ENT  *direction           - pointer to direction value
 *   GT_U32                   *length              - pointer to preamble length value
 *
 *   CPSS_PORT_DIRECTION_ENT *targetDirection       - pointer to switch MAC value
 *
 *   GT_BOOL         *switchPreambleLengthPtr       - pointer to switch MAC preamble length
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (25) Get Preample length callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PREAMBLE_LENGTH_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DIRECTION_ENT     direction,
    OUT GT_U32                      *lengthPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT     *targetDirection,
    OUT GT_U32                      *switchPreambleLengthPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_CRC_CHECK_ENABLE_SET_FUNC)
 *
 * Description: defines set CRC callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                    devNum    - device number
 *   GT_U8                    portNum   - port number
 *   GT_BOOL                  enable    - set value
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *switchCRCCheckSetPtr          - CRC value pointer
 *                                                    to switch MAC value
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (26) Set CRC check enable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_CRC_CHECK_ENABLE_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckSetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_CRC_CHECK_ENABLE_GET_FUNC)
 *
 * Description: defines get CRC callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                    devNum    - device number
 *   GT_U8                    portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *enablePtr                     - value pointer
 *   GT_BOOL         *switchCRCCheckGetPtr          - CRC value pointer for switch MAC value
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (27) Get CRC check enable callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_CRC_CHECK_ENABLE_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_MRU_SET_FUNC)
 *
 * Description: defines set MRU callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                    devNum    - device number
 *   GT_U8                    portNum   - port number
 *   GT_U32                   mruSize   - MRU size
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *switchMRUSetPtr               - switch MAC MRU size pointer
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (28) Set MRU callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_MRU_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32                  mruSize,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_U32                 *switchMRUSetPtr
);


/*
 *  typedef GT_STATUS (*CPSS_MACDRV_MRU_GET_FUNC)
 *
 * Description: defines get MRU size callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                    devNum    - device number
 *   GT_U8                    portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_U32          *mruSizePtr                    - MRU size value pointer
 *
 *   GT_BOOL         *switchMRUGetPtr               - switch MAC MRU size pointer
 *                                                    to value for switch MAC
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configurated
 *
 *  COMMENTS: (29) Get MRU callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_MRU_GET_FUNC)
(
    IN   GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32                 *mruSizePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_U32                 *switchMRUGetPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_POWER_DOWN_SET_FUNC)
 *
 * Description: defines port power up/down set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_BOOL               powerDown    - GT_FALSE: power up/GT_TRUE: power down
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr  - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS:
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_POWER_DOWN_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     powerDown,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_ATTRIBUTES_GET_FUNC)
 *
 * Description: defines link status / speed / duplex get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_PORT_ATTRIBUTES_STC  *portAttributesPtr
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: Port Link Status / speed / duplex Get callback function
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_ATTRIBUTES_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributesPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_CSCD_PORT_TYPE_SET_FUNC)
 *
 * Description: defines Cascade Type set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   CPSS_PORT_DIRECTION_ENT      portDirection,
 *   CPSS_CSCD_PORT_TYPE_ENT      port cascade Type - currently only Network type
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS:
 *        None.
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_CSCD_PORT_TYPE_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_CSCD_PORT_TYPE_GET_FUNC)
 *
 * Description: defines Cascade Type get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   CPSS_PORT_DIRECTION_ENT      portDirection,
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *   CPSS_CSCD_PORT_TYPE_ENT      *portTypePtr - currently only Network type supported
 *
 *  COMMENTS:
 *        None.
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_CSCD_PORT_TYPE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    OUT CPSS_CSCD_PORT_TYPE_ENT     *portTypePtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_MAC_SA_LSB_SET_FUNC)
 *
 * Description: defines MAC SA LSB set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   GT_U8                  macSaLsb - The least significant byte of the MAC SA
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS:
 *        None.
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_MAC_SA_LSB_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN GT_U8                        macSaLsb
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_MAC_SA_LSB_GET_FUNC)
 *
 * Description: defines MAC SA LSB get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *   GT_U8                  *macSaLsbPtr - The least significant byte of the MAC SA
 *
 *  COMMENTS:
 *        None.
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_MAC_SA_LSB_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_U8                       *macSaLsbPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_MAC_SA_BASE_SET_FUNC)
 *
 * Description: defines MAC SA BASE set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portMacNum  - port MAC number (Note: not the remote
 *                                          port but the MAC number through which a
 *                                          group of remote ports are connected)
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   GT_ETHERADDR                  macPtr - The MAC SA
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS:
 *        None.
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_MAC_SA_BASE_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMacNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN  GT_ETHERADDR                *macPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_PORT_INTERFACE_MODE_GET_FUNC)
 *
 * Description: defines port interface mode get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - (pointer to) the parameter defines if the switch MAC
 *                                                    will be configured
 *   CPSS_PORT_INTERFACE_MODE_ENT                  *ifModePtr - Port interface mode
 *
 *  COMMENTS:
 *        None.
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_PORT_INTERFACE_MODE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_VCT_LENGTH_OFFSET_SET_FUNC)
 *
 * Description: defines VCT length offset set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   GT_32                   vctLengthOffset - offset in millimeters
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_VCT_LENGTH_OFFSET_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_32                       vctLengthOffset,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_VCT_LENGTH_OFFSET_GET_FUNC)
 *
 * Description: defines VCT length offset get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_32                   vctLengthOffsetPtr - (Pointer to) offset in millimeters
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_VCT_LENGTH_OFFSET_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_32                       *vctLengthOffsetPtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

typedef struct
{
    GT_BOOL                                 enable;
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  mode;
    GT_U32                                  twLimit;
    GT_U32                                  liLimit;
    GT_U32                                  tsLimit;
} CPSS_MACDRV_EEE_CONFIG_STC;

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_EEE_CONFIG_SET_FUNC)
 *
 * Description: defines EEE configuration set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_EEE_CONFIG_STC  *eeeConfigPtr - EEE configuration to be set on this port
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_EEE_CONFIG_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_MACDRV_EEE_CONFIG_STC  *eeeConfigPtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_EEE_CONFIG_GET_FUNC)
 *
 * Description: defines EEE configuration get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_MACDRV_EEE_CONFIG_STC  *eeeConfigPtr - EEE configuration to be set on this port
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_EEE_CONFIG_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_MACDRV_EEE_CONFIG_STC  *eeeConfigPtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_EEE_STATUS_GET_FUNC)
 *
 * Description: defines EEE status get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *eeeStatusPtr - EEE status on this port
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_EEE_STATUS_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *eeeRxStatusPtr,
    OUT GT_BOOL                     *eeeTxStatusPtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_LED_GLOBAL_CONFIG_SET_FUNC)
 *
 * Description: Set global (per-PHY) LED configuration.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  cascadePortNum   - physical port number connected to remote ports.
 *   CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC    ledGlobalConfigPtr  - (pointer to) global configuration
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_LED_GLOBAL_CONFIG_SET_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_LED_GLOBAL_CONFIG_GET_FUNC)
 *
 * Description: Get global (per-PHY) LED configuration.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  cascadePortNum   - physical port number connected to remote ports.
 *   CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC    ledGlobalConfigPtr  - (pointer to) global configuration
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC    ledGlobalConfigPtr  - (pointer to) global configuration
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_LED_GLOBAL_CONFIG_GET_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    OUT CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_LED_PER_PORT_SET_FUNC)
 *
 * Description: Set per-port LED configuration.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  portNum   - physical port number.
 *   CPSS_DXCH_LED_PHY_PORT_CONF_STC    ledPortConfigPtr  - (pointer to) per-port configuration
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_LED_PER_PORT_CONFIG_SET_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPortConfigPtr,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_LED_PER_PORT_GET_FUNC)
 *
 * Description: Get per-port LED configuration.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  portNum   - physical port number.
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_DXCH_LED_PHY_PORT_CONF_STC    ledPortConfigPtr  - (pointer to) per-port configuration
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_LED_PER_PORT_CONFIG_GET_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPortConfigPtr,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_GPIO_CONFIG_SET_FUNC)
 *
 * Description: Set GPIO configuration.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  cascadePortNum   - physical port number connected to remote ports.
 *   GT_U32                 directionBitmap     - For bits 0..15:
 *                                       bit#i set means GPIO#i is input,
 *                                       bit#i not set means GPIO#i is output
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_GPIO_CONFIG_SET_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  GT_U32                              directionBitmap,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_GPIO_CONFIG_GET_FUNC)
 *
 * Description: Get GPIO configuration.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  cascadePortNum   - physical port number connected to remote ports.
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_U32                 *modeBitmapPtr     - (Pointer to) For bits 0..15:
 *                                       bit#i set means GPIO#i pin can be used as GPIO,
 *                                       bit#i not set means GPIO#i pin cannot be used as GPIO
 *   GT_U32                 *directionBitmapPtr     - (Pointer to) For bits 0..15:
 *                                       bit#i set means GPIO#i is input,
 *                                       bit#i not set means GPIO#i is output
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_GPIO_CONFIG_GET_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    OUT GT_U32                             *modeBitmapPtr,
    OUT GT_U32                             *directionBitmapPtr,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_GPIO_DATA_READ_FUNC)
 *
 * Description: Read GPIO data.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  cascadePortNum   - physical port number connected to remote ports.
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_U32                  dataBitmapPtr     - (Pointer to) Bits 0..15 reflect the input value on
 *                                       GPIO pins enabled and configured for input.
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_GPIO_DATA_READ_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    OUT GT_U32                             *dataBitmapPtr,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*
 *  typedef GT_STATUS (*CPSS_MACDRV_GPIO_DATA_WRITE_FUNC)
 *
 * Description: Write GPIO data.
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_PHYSICAL_PORT_NUM  cascadePortNum   - physical port number connected to remote ports.
 *   GT_U32                  dataBitmap     - Bits 0..15 reflect the input value on
 *                                       GPIO pins enabled and configured for input.
 *   GT_U32                  dataBitmapMask    - For bits 0..15:
 *                                       bit#i set means corresponding bit at dataBitmap will be used,
 *                                       bit#i not set means corresponding bit at dataBitmap will be discarded.
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL                *doPpMacConfigPtr       - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: 
 *
 */
typedef GT_STATUS (*CPSS_MACDRV_GPIO_DATA_WRITE_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  GT_U32                              dataBitmap,
    IN  GT_U32                              dataBitmapMask,
    IN  CPSS_MACDRV_STAGE_ENT               stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTER_GET_FUNC
*
* DESCRIPTION:
*       Gets Ethernet MAC counter for a specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*       cntrName - specific counter name
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       cntrValuePtr - (pointer to) current counter value.
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_BAD_PARAM - on wrong port number, device or counter name
*       GT_HW_ERROR  - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     1. The following counters are not supported:
*        Tri-Speed Ports:
*          CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*          CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*        XG / HyperG.Stack / XLG Potrs:
*          CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*          CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*          CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*          CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*     2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*          not using SDMA interface.
*        When using SDMA interface the following APIs are relevant:
*          CPSS_DXCH_NetIfSdmaRxCountersGet, CPSS_DXCH_NetIfSdmaRxErrorCountGet.
*     3. The following counters are supported for CPU port:
*           CPSS_GOOD_PKTS_SENT_E, CPSS_MAC_TRANSMIT_ERR_E, CPSS_DROP_EVENTS_E,
*           CPSS_GOOD_OCTETS_SENT_E, CPSS_GOOD_PKTS_RCV_E, CPSS_BAD_PKTS_RCV_E,
*           CPSS_GOOD_OCTETS_RCV_E, CPSS_BAD_OCTETS_RCV_E.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTER_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr

);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_ON_PORT_GET_FUNC
*
* DESCRIPTION:
*       Gets Ethernet MAC counter for a particular Port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     1. The following counters are not supported:
*        Tri-Speed Ports:
*          badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*        XG / HyperG.Stack / XLG Potrs:
*          badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*          excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*     2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*          not using SDMA interface.
*        When using SDMA interface the following APIs are relevant:
*          CPSS_DXCH_NetIfSdmaRxCountersGet, CPSS_DXCH_NetIfSdmaRxErrorCountGet.
*     3. The following counters are supported for CPU port:
*           goodPktsSent, macTransmitErr, goodOctetsSent, dropEvents,
*           goodPktsRcv, badPktsRcv, goodOctetsRcv, badOctetsRcv.
*
*******************************************************************************/
typedef GT_STATUS (*   CPSS_MACDRV_PORT_MAC_COUNTERS_ON_PORT_GET_FUNC)
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN CPSS_MACDRV_STAGE_ENT            stage,
    OUT GT_BOOL                         *doPpMacConfigPtr

);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTER_CAPTURE_GET_FUNC
*
* DESCRIPTION:
*       Gets the captured Ethernet MAC counter for a specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       cntrName - specific counter name
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       cntrValuePtr - (pointer to) current counter value.
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*       GT_BAD_PARAM - on wrong port number, device or counter name
*       GT_HW_ERROR      - on hardware error
*       GT_BAD_PTR       - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     1. For Tri-speed ports, each MAC counter capture performed on ports of
*        the same group overrides previous capture made on port of that group.
*         Ports 0-5 belong to group 0.
*         Ports 6-11 belong to group 1.
*         Ports 12-17 belong to group 2.
*         Ports 18-23 belong to group 3.
*     2. The following counters are not supported:
*        Tri-Speed Ports:
*          CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*          CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*        XG / HyperG.Stack / XLG Potrs:
*          CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*          CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*          CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*          CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTER_CAPTURE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_ON_PORT_GET_FUNC
*
* DESCRIPTION:
*       Gets captured Ethernet MAC counter for a particular Port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_BAD_PARAM - on wrong port number or device
*       GT_HW_ERROR  - on hardware error
*       GT_BAD_PTR   - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     1. For Tri-speed ports, each MAC counter capture performed on ports of
*        the same group overrides previous capture made on port of that group.
*         Ports 0-5 belong to group 0.
*         Ports 6-11 belong to group 1.
*         Ports 12-17 belong to group 2.
*         Ports 18-23 belong to group 3.
*     2. The following counters are not supported:
*        Tri-Speed Ports:
*          badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*        XG / HyperG.Stack / XLG Potrs:
*          badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*          excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*
*******************************************************************************/
typedef GT_STATUS (*   CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_ON_PORT_GET_FUNC)
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN CPSS_MACDRV_STAGE_ENT            stage,
    OUT GT_BOOL                         *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_TRIGGER_SET_FUNC
*
* DESCRIPTION:
*     The function triggers a capture of MIB counters for specific port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number, whose counters are to be captured.
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       None.
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_BAD_PARAM - on wrong port number or device
*       GT_HW_ERROR  - on hardware error
*       GT_BAD_STATE - previous capture operation isn't finished.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*     1. For Tri-speed ports, each MAC counter capture performed on ports of
*        the same group overrides previous capture made on port of that group.
*         Ports 0-5 belong to group 0.
*         Ports 6-11 belong to group 1.
*         Ports 12-17 belong to group 2.
*         Ports 18-23 belong to group 3.
*     2. To get the counters, see
*          cpssDxChPortMacCountersCaptureOnPortGet,
*          cpssDxChPortMacCounterCaptureGet.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_TRIGGER_SET_FUNC)
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_TRIGGER_GET_FUNC
*
* DESCRIPTION:
*    The function gets status of a capture of MIB counters for specific port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number.
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       captureIsDonePtr     - pointer to status of Capture counter Trigger
*                              - GT_TRUE  - capture is done.
*                              - GT_FALSE - capture action is in proccess.
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_BAD_PARAM - on wrong device number
*       GT_HW_ERROR  - on hardware error
*       GT_BAD_PTR   - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     1. For Tri-speed ports, each MAC counter capture performed on ports of
*        the same group overrides previous capture made on port of that group.
*         Ports 0-5 belong to group 0.
*         Ports 6-11 belong to group 1.
*         Ports 12-17 belong to group 2.
*         Ports 18-23 belong to group 3.
*     2. To get the counters, see
*          cpssDxChPortMacCountersCaptureOnPortGet,
*          cpssDxChPortMacCounterCaptureGet.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_TRIGGER_GET_FUNC)
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL  *captureIsDonePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);


/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_ENABLE_FUNC
*
* DESCRIPTION:
*       Enable or disable MAC Counters update for this port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number.
*       portNum  - physical port number (or CPU port)
*       enable   -  enable update of MAC counters
*                   GT_FALSE = MAC counters update for this port is disabled.
*                   GT_TRUE = MAC counters update for this port is enabled.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None;
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_ENABLE_FUNC)(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_ENABLE_GET_FUNC
*
* DESCRIPTION:
*       Get Enable or disable MAC Counters status for this port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number.
*       portNum  - physical port number (or CPU port)
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       enablePtr   -  (Pointer to) enable status of MAC counters
*                   GT_FALSE = MAC counters update for this port is disabled.
*                   GT_TRUE = MAC counters update for this port is enabled.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None;
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_ENABLE_GET_FUNC)(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL * enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);


/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_CLEAR_ON_READ_SET_FUNC
*
* DESCRIPTION:
*       Enable or disable MAC Counters Clear on read per group of ports
*       for Tri-Speed Ports or per port for HyperG.Stack ports.
*       For Tri-Speed Ports groups are:
*       - Ports 0 through 5
*       - Ports 6 through 11
*       - Ports 12 through 17
*       - Ports 18 through 23
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number (or CPU port)
*       enable      - enable clear on read for MAC counters
*                       GT_FALSE - Counters are not cleared.
*                       GT_TRUE - Counters are cleared.
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_CLEAR_ON_READ_SET_FUNC)(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_CLEAR_ON_READ_GET_FUNC
*
* DESCRIPTION:
*       Get "Clear on read" status of MAC Counters per group of ports
*       for Tri-Speed Ports or per port for HyperG.Stack ports.
*       For Tri-Speed Ports groups are:
*       - Ports 0 through 5
*       - Ports 6 through 11
*       - Ports 12 through 17
*       - Ports 18 through 23
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number (or CPU port)
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       enablePtr   - (pointer to) "Clear on read" status for MAC counters
*                     GT_FALSE - Counters are not cleared.
*                     GT_TRUE - Counters are cleared.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_FAIL                     - on error.
*       GT_HW_ERROR                 - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number
*       GT_BAD_PTR                  - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
* COMMENTS:
*       None
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_CLEAR_ON_READ_GET_FUNC)(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL  *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_RX_HISTOGRAM_ENABLE_FUNC
*
* DESCRIPTION:
*       Enable/disable updating of the RMON Etherstat histogram
*       counters for received packets per group of ports
*       for Tri-Speed Ports or per port for HyperG.Stack ports.
*       For Tri-Speed Ports groups are:
*           - Ports 0 through 5
*           - Ports 6 through 11
*           - Ports 12 through 17
*           - Ports 18 through 23
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number
*       enable      - enable updating of the counters for received packets
*                   GT_TRUE - Counters are updated.
*                   GT_FALSE - Counters are not updated.
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       switchEnableSetPtr - (pointer to) switch enable state
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_RX_HISTOGRAM_ENABLE_FUNC)(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL  enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_TX_HISTOGRAM_ENABLE_FUNC
*
* DESCRIPTION:
*       Enable/disable updating of the RMON Etherstat histogram
*       counters for transmitted packets per group of ports
*       for Tri-Speed Ports or per port for HyperG.Stack ports.
*       For Tri-Speed Ports groups are:
*           - Ports 0 through 5
*           - Ports 6 through 11
*           - Ports 12 through 17
*           - Ports 18 through 23
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number
*       enable      - enable updating of the counters for transmitted packets
*                       GT_TRUE - Counters are updated.
*                       GT_FALSE - Counters are not updated.
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       switchEnableSetPtr = (pointer to) switch enable state
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_TX_HISTOGRAM_ENABLE_FUNC)(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_HISTOGRAM_ENABLE_GET_FUNC
*
* DESCRIPTION:
*       Get Enable/disable status of the RMON Etherstat histogram
*       counters for transmitted packets per group of ports
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number
*       direction   - Whether to query Tx or Rx direction.
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       enablePtr   - (pointer to) Histogram count status for MAC counters on specified direction.
        doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_HISTOGRAM_ENABLE_GET_FUNC)(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_DIRECTION_ENT     direction,
    OUT GT_BOOL                     *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_SET_FUNC
*
* DESCRIPTION:
*       Sets MIB counters behavior for oversized packets on given port.
*
* APPLICABLE DEVICES:
*        xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* INPUTS:
*       devNum      - device number
*       portNum     - physical port number
*       counterMode - oversized packets counter mode
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - on wrong port number, device or counter mode
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_SET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *switchcounterModeSetPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_GET_FUNC
*
* DESCRIPTION:
*       Gets MIB counters behavior for oversized packets on given port.
*
* APPLICABLE DEVICES:
*        xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* INPUTS:
*       devNum         - device number
*       portNum        - physical port number
*       stage          - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       counterModePtr - (pointer to) oversized packets counter mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTER_ON_PHY_SIDE_GET_FUNC
*
* DESCRIPTION:
*       Gets Ethernet MAC counter on PHY side of connecting link to remote ports.
*
* APPLICABLE DEVICES:
*        AC3X.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* INPUTS:
*       devNum   - physical device number
*       cascadePortNum  - connecting physical port number connected to remote ports
*       cntrName - specific counter name
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       cntrValuePtr - (pointer to) current counter value.
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_BAD_PARAM - on wrong port number, device or counter name
*       GT_HW_ERROR  - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Applicable only to remote ports on AC3X.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTER_ON_PHY_SIDE_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        cascadePortNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr

);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_COUNTERS_ON_PHY_SIDE_PORT_GET_FUNC
*
* DESCRIPTION:
*       Gets all port Ethernet MAC counters on PHY side of connecting link to remote Port.
*
* APPLICABLE DEVICES:
*        AC3X.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* INPUTS:
*       devNum   - physical device number
*       cascadePortNum  - connecting physical port number connected to remote ports
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Applicable only to remote ports on AC3X.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_COUNTERS_ON_PHY_SIDE_PORT_GET_FUNC)
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            cascadePortNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN CPSS_MACDRV_STAGE_ENT            stage,
    OUT GT_BOOL                         *doPpMacConfigPtr

);

/*
 *  typedef: struct CPSS_MACDRV_COUNTERS_OBJ_STC
 *
 * Description: the structure holds the "MAC PHY driver interface" functions for
 *              counters.
 *          NOTE: each and every callback function in the structure CAN be NULL.
 *                meaning it is ignored and the function is activated only on the
 *               port of the 'hosting' device.
 *  COMMENTS:
 *  ----------------------------------------------------------------------------------------
 *             API                                             MACPHY function
 *------------------------------------------------------------------------------------------
 *   1    cpssDxChMacCounterGet                               macDrvMacPortMacCounterGet
 *   2    cpssDxChPortMacCountersOnPortGet                    macDrvMacPortMacCountersOnPortGet
 *   3    cpssDxChPortMacCounterCaptureGet                    macDrvMacPortMacCounterCaptureGet
 *   4    cpssDxChPortMacCountersCaptureOnPortGet             macDrvMacPortMacCountersCaptureOnPortGet
 *   5    cpssDxChPortMacCountersCaptureTriggerSet            macDrvMacPortMacCountersCaptureTriggerSet
 *   6    cpssDxChPortMacCountersCaptureTriggerGet            macDrvMacPortMacCountersCaptureTriggerGet
 *   7    cpssDxChPortMacCountersEnable                       macDrvMacPortMacCountersEnable
 *   8    cpssDxChPortMacCountersEnableGet                    macDrvMacPortMacCountersEnableGet
 *   9    cpssDxChPortMacCountersClearOnReadSet               macDrvMacPortMacCountersClearOnReadSet
 *  10    cpssDxChPortMacCountersClearOnReadGet               macDrvMacPortMacCountersClearOnReadGet
 *  11    cpssDxChPortMacCountersRxHistogramEnable            macDrvMacPortMacCountersRxHistogramEnable
 *  12    cpssDxChPortMacCountersTxHistogramEnable            macDrvMacPortMacCountersTxHistogramEnable
 *  13    cpssDxChPortMacCountersHistogramEnableGet         macDrvMacPortMacCountersTxHistogramEnableGet
 *  14    cpssDxChPortMacOversizedPacketsCounterModeSet       macDrvMacPortMacOversizedPacketsCounterModeSet
 *  15    cpssDxChPortMacOversizedPacketsCounterModeGet       macDrvMacPortMacOversizedPacketsCounterModeGet
 *---------------------------------------------------------------------------------------------
 */

typedef struct{
/* 1*/CPSS_MACDRV_PORT_MAC_COUNTER_GET_FUNC                            macDrvMacPortMacCounterGet;
/* 2*/CPSS_MACDRV_PORT_MAC_COUNTERS_ON_PORT_GET_FUNC                   macDrvMacPortMacCountersOnPortGet;
/* 3*/CPSS_MACDRV_PORT_MAC_COUNTER_CAPTURE_GET_FUNC                    macDrvMacPortMacCounterCaptureGet;
/* 4*/CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_ON_PORT_GET_FUNC           macDrvMacPortMacCountersCaptureOnPortGet;
/* 5*/CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_TRIGGER_SET_FUNC           macDrvMacPortMacCountersCaptureTriggerSet;
/* 6*/CPSS_MACDRV_PORT_MAC_COUNTERS_CAPTURE_TRIGGER_GET_FUNC           macDrvMacPortMacCountersCaptureTriggerGet;
/* 7*/CPSS_MACDRV_PORT_MAC_COUNTERS_ENABLE_FUNC                        macDrvMacPortMacCountersEnable;
/* 8*/CPSS_MACDRV_PORT_MAC_COUNTERS_ENABLE_GET_FUNC                    macDrvMacPortMacCountersEnableGet;
/* 9*/CPSS_MACDRV_PORT_MAC_COUNTERS_CLEAR_ON_READ_SET_FUNC             macDrvMacPortMacCountersClearOnReadSet;
/*10*/CPSS_MACDRV_PORT_MAC_COUNTERS_CLEAR_ON_READ_GET_FUNC             macDrvMacPortMacCountersClearOnReadGet;
/*11*/CPSS_MACDRV_PORT_MAC_COUNTERS_RX_HISTOGRAM_ENABLE_FUNC           macDrvMacPortMacCountersRxHistogramEnable;
/*12*/CPSS_MACDRV_PORT_MAC_COUNTERS_TX_HISTOGRAM_ENABLE_FUNC           macDrvMacPortMacCountersTxHistogramEnable;
/*13*/CPSS_MACDRV_PORT_MAC_COUNTERS_HISTOGRAM_ENABLE_GET_FUNC          macDrvMacPortMacCountersHistogramEnableGet;
/*14*/CPSS_MACDRV_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_SET_FUNC     macDrvMacPortMacOversizedPacketsCounterModeSet;
/*15*/CPSS_MACDRV_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_GET_FUNC     macDrvMacPortMacOversizedPacketsCounterModeGet;

      CPSS_MACDRV_PORT_MAC_COUNTER_ON_PHY_SIDE_GET_FUNC                macDrvMacPortMacCounterOnPhySideGet;
      CPSS_MACDRV_PORT_MAC_COUNTERS_ON_PHY_SIDE_PORT_GET_FUNC          macDrvMacPortMacCountersOnPhySidePortGet;
}CPSS_MACDRV_COUNTERS_OBJ_STC;


/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_TYPE_GET_FUNC
*
* DESCRIPTION:
*       Get port MAC type.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical port number
*
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       portMacTypePtr - (pointer to) port MAC type
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_MAC_TYPE_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_LOOPBACK_SET_FUNC
*
* DESCRIPTION:
*       Set the internal Loopback state in the packet processor MAC port.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       enable    - If GT_TRUE, enable loopback
*                   If GT_FALSE, disable loopback
*       stage     - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_LOOPBACK_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_LOOPBACK_GET_FUNC
*
* DESCRIPTION:
*       Get Internal Loopback
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       enablePtr  - Pointer to the Loopback state.
*                    If GT_TRUE, loopback is enabled
*                    If GT_FALSE, loopback is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not relevant for the CPU port.
*
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_LOOPBACK_GET_FUNC)
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr,
    IN  CPSS_MACDRV_STAGE_ENT  stage,
    OUT GT_BOOL                *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_INTERFACE_SPEED_SUPPORT_GET_FUNC
*
* DESCRIPTION:
*       Check if given pair ifMode and speed supported by given port on
*        given device
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number (not CPU port)
*       ifMode    - interface mode
*       speed     - port data speed
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       supportedPtr ? GT_TRUE ? (ifMode; speed) supported
*                      GT_FALSE ? (ifMode; speed) not supported
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - if one of input parameters wrong
*       GT_BAD_PTR        - if supportedPtr is NULL
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_INTERFACE_SPEED_SUPPORT_GET_FUNC)
(
     IN  GT_U8                           devNum,
     IN  GT_PHYSICAL_PORT_NUM            portNum,
     IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
     IN  CPSS_PORT_SPEED_ENT             speed,
     OUT GT_BOOL                         *supportedPtr,
     IN  CPSS_MACDRV_STAGE_ENT           stage,
     OUT GT_BOOL                         *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_FORCE_LINK_DOWN_SET_FUNC
*
* DESCRIPTION:
*       Enable/disable Force Link Down on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*       state      - GT_TRUE for force link down, GT_FALSE otherwise
*       stage      - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_FORCE_LINK_DOWN_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_FORCE_LINK_DOWN_GET_FUNC
*
* DESCRIPTION:
*       Get Force Link Down state on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       statePtr - Pointer to the force link down state.
*                  GT_TRUE for force link down, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_FORCE_LINK_DOWN_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_FORCE_LINK_PASS_SET_FUNC
*
* DESCRIPTION:
*       Enable/disable Force Link Pass on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*       state      - GT_TRUE for force link pass, GT_FALSE otherwise
*       stage      - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_FORCE_LINK_PASS_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_FORCE_LINK_PASS_GET_FUNC
*
* DESCRIPTION:
*       Get Force Link Pass state on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*       stage       - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*       statePtr   - Pointer to the force link pass state.
*                    GT_TRUE for force link pass, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_FORCE_LINK_PASS_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_BOOL                *statePtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_HW_SMI_REGISTER_SET_FUNC
*
* DESCRIPTION:
*       Write value to specified SMI Register on a specified port on
*       specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical port number
*       phyReg      - SMI register
*       data        - data to write
*       stage       - port callback function stage
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to) port MAC configuaration condition.
*           GT_TRUE - continue with port MAC configuration
*           GT_FALSE - bypass port MAC configuration
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_HW_SMI_REGISTER_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   phyReg,
    IN  GT_U16                  data,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_HW_SMI_REGISTER_GET_FUNC
*
* DESCRIPTION:
*       Read specified SMI Register on a specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       phyReg    - SMI register
*       stage     - port callback function stage
*
* OUTPUTS:
*       dataPtr   - (pointer to) the read data.
*       doPpMacConfigPtr - (pointer to) port MAC configuaration condition.
*           GT_TRUE - continue with port MAC configuration
*           GT_FALSE - bypass port MAC configuration
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_HW_SMI_REGISTER_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   phyReg,
    OUT GT_U16                  *dataPtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_HW_10G_SMI_REGISTER_SET_FUNC
*
* DESCRIPTION:
*       Write value to specified 10G SMI Register on a specified port on
*       specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical port number
*       laneNum     - Serdes lane number when accessing multi lane device
*       phyReg      - SMI register
*       phyDev      - SMI device
*       data        - data to write
*       stage       - port callback function stage
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to) port MAC configuaration condition.
*           GT_TRUE - continue with port MAC configuration
*           GT_FALSE - bypass port MAC configuration
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_HW_10G_SMI_REGISTER_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   laneNum,
    IN  GT_U16                  phyReg,
    IN  GT_U8                   phyDev,
    IN  GT_U16                  data,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_HW_10G_SMI_REGISTER_GET_FUNC
*
* DESCRIPTION:
*       Read specified 10G SMI Register on a specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       laneNum     - Serdes lane number when accessing multi lane device
*       phyReg    - SMI register
*       phyDev      - SMI device
*       stage     - port callback function stage
*
* OUTPUTS:
*       dataPtr   - (pointer to) the read data.
*       doPpMacConfigPtr - (pointer to) port MAC configuaration condition.
*           GT_TRUE - continue with port MAC configuration
*           GT_FALSE - bypass port MAC configuration
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_HW_10G_SMI_REGISTER_GET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   laneNum,
    IN  GT_U16                  phyReg,
    IN  GT_U8                   phyDev,
    OUT GT_U16                  *dataPtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_TPID_PROFILE_SET_FUNC
*
* DESCRIPTION:
*       Function sets TPID to remote port.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum     - device number
*       portNum    - remote port number.
*       ethMode    - TAG0/TAG1 selector
*       profile    - TPID profile. (APPLICABLE RANGE: 0..7)
*       stage     - port callback function stage
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to) port MAC configuaration condition.
*           GT_TRUE - continue with port MAC configuration
*           GT_FALSE - bypass port MAC configuration
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong devNum, portNum, ethMode
*       GT_OUT_OF_RANGE          - profile > 7
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
* relevant to 'etherType_0' only ! (no limit on 'etherType_1')
* 88e1690 port : cpssDxChBrgVlanPortIngressTpidProfileSet : profile.
*     1. cpss get bmp of the profile.
*             a. bmp of '0' - treated as TPID 0x8100
*     2. if the bmp hold pointer to to '0x8100'
*             a. set the 88e1690 port as : <Frame Mode> = Normal Network (value 0)
*        else -- the bmp is NOT '0x8100' --
*             use 'first TPID' in bmp
*             a. set the 88e1690 port as : <PortEType> = etherType0 (register "Port E Type")
*             b. set the 88e1690 port as : <Frame Mode> = Provider       (value 2)
*     3. set the profile in the Aldrin (for this port)
*
* cpssDxChBrgVlanIngressTpidProfileSet     : no 88e1690 change.
* cpssDxChBrgVlanPortIngressTpidProfileSet : no 88e1690 change.
*
*******************************************************************************/
typedef GT_STATUS (* CPSS_MACDRV_PORT_TPID_PROFILE_SET_FUNC)
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               profile,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr
);

/*
*  CPSS_MACDRV_PORT_MAC_STATUS_GET_FUNC
*
* DESCRIPTION:
*       Reads bits that indicate different problems on specified port.
*
* INPUTS:
*       devNum              - physical device number
*       portNum             - physical port number
*       stage               - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       portMacStatusPtr    - (pointer to) info about port MAC
*       doPpMacConfigPtr    - (pointer to) the parameter defines if the switch MAC
*                                                    will be configurated
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*
* COMMENTS:
*       None.
*
*/
typedef GT_STATUS (*CPSS_MACDRV_PORT_MAC_STATUS_GET_FUNC)
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_MAC_STATUS_STC    *portMacStatusPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
);

/*******************************************************************************
* CPSS_MACDRV_PORT_MAC_DEFAULT_UP_SET_FUNC
*
* DESCRIPTION:
*       Set default user priority (VPT) for untagged packet to a given port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - packet processor device number.
*       portNum      - port number, CPU port.
*                      In eArch devices portNum is default ePort.
*       defaultUserPrio  - default user priority (VPT) (APPLICABLE RANGES: 0..7).
*       stage    - stage for callback run (PRE or POST)
*
* OUTPUTS:
*       doPpMacConfigPtr - (pointer to)the parameter defines if the switch MAC will be configured
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_MACDRV_PORT_MAC_DEFAULT_UP_SET_FUNC)
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U8        defaultUserPrio,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr
);

/* STUB function to be 'place holder' */
typedef GT_STATUS (* CPSS_MACDRV___STUB__FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    IN const char*    funcNamePtr
);

/* place holder for the remote physical ports that need MAC-in-PHY support */
#define  PRV_CPSS_PHY_MAC___STUB__OBJ(devNum,portNum,portMacNum)               \
{                                                                              \
    GT_STATUS   rc;                                                            \
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;                                       \
    GT_BOOL doPpMacConfig;                                                     \
    /* Get PHY MAC object pnt */                                               \
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);                   \
                                                                               \
    /* check if MACPHY callback should run */                                  \
    if (portMacObjPtr != NULL && portMacObjPtr->objStubFunc) {                 \
                                                                               \
        rc = portMacObjPtr->objStubFunc(devNum,portNum,CPSS_MACDRV_STAGE_PRE_E,\
                                  &doPpMacConfig,__FUNCNAME__);                \
        if(rc != GT_OK)                                                        \
        {                                                                      \
            return rc;                                                         \
        }                                                                      \
                                                                               \
        if(doPpMacConfig == GT_FALSE)                                          \
        {                                                                      \
            return GT_OK;                                                      \
        }                                                                      \
    }                                                                          \
}


/*
 *  typedef: struct CPSS_MACDRV_OBJ_STC
 *
 * Description: the structure holds the "MAC PHY driver interface" functions
 *
 *      macCounters - hold callback functions that relate to 'MAC counters'
 *
 *  COMMENTS:
 *  ----------------------------------------------------------------------------------------
 *             API                                             MACPHY function
 *------------------------------------------------------------------------------------------
 *   1    cpssDxChPortSpeedSet                               macDrvMacSpeedSetFunc
 *   2    cpssDxChPortSpeedGet                               macDrvMacSpeedGetFunc
 *   3    cpssDxChPortDuplexAutoNegEnableSet                 macDrvMacDuplexANSetFunc
 *   4    cpssDxChPortDuplexAutoNegEnableGet                 macDrvMacDuplexANGetFunc
 *   5    cpssDxChPortFlowCntrlAutoNegEnableSet              macDrvMacFlowCntlANSetFunc
 *   6    cpssDxChPortFlowCntrlAutoNegEnableGet              macDrvMacFlowCntlANGetFunc
 *   7    cpssDxChPortSpeedAutoNegEnableSet                  macDrvMacSpeedANSetFunc
 *   8    cpssDxChPortSpeedAutoNegEnableGet                  macDrvMacSpeedANGetFunc
 *   9    cpssDxChPortFlowControlEnableSet                   macDrvMacFlowCntlSetFunc
 *  10    cpssDxChPortFlowControlEnableGet                   macDrvMacFlowCntlGetFunc
 *  11    cpssDxChPortPeriodicFcEnableSet                    macDrvMacPeriodFlowCntlSetFunc
 *  12    cpssDxChPortPeriodicFcEnableGet                    macDrvMacPeriodFlowCntlGetFunc
 *  13    cpssDxChPortBackPressureEnableSet                  macDrvMacBackPrSetFunc
 *  14    cpssDxChPortBackPressureEnableGet                  macDrvMacBackPrGetFunc
 *  15    cpssDxChPortLinkStatusGet                          macDrvMacPortlinkGetFunc
 *  16    cpssDxChPortDuplexModeSet                          macDrvMacDuplexSetFunc
 *  17    cpssDxChPortDuplexModeGet                          macDrvMacDuplexGetFunc
 *  18    cpssDxChPortEnableSet                              macDrvMacPortEnableSetFunc
 *  19    cpssDxChPortEnableGet                              macDrvMacPortEnableGetFunc
 *  20    cpssDxChPortExcessiveCollisionDropEnableSet        macDrvMacExcessiveCollisionDropSetFunc
 *  21    cpssDxChPortExcessiveCollisionDropEnableSet        macDrvMacExcessiveCollisionDropGetFunc
 *  22    cpssDxChPortPaddingEnableSet                       macDrvMacPaddingEnableSetFunc
 *  23    cpssDxChPortPaddingEnableGet                       macDrvMacPaddingEnableGetFunc
 *  24    cpssDxChPortPreambleLengthSet                      macDrvMacPreambleLengthSetFunc
 *  25    cpssDxChPortPreambleLengthGet                      macDrvMacPreambleLengthGetFunc
 *  26    cpssDxChPortCrcCheckEnableSet                      macDrvMacCRCCheckSetFunc
 *  27    cpssDxChPortCrcCheckEnableGet                      macDrvMacCRCCheckGetFunc
 *  28    cpssDxChPortMruSet                                 macDrvMacMRUSetFunc
 *  29    cpssDxChPortMruGet                                 macDrvMacMRUGetFunc
 *        cpssDxChPortAttributesOnPortGet                    macDrvMacPortAttributesGetFunc
 *        cpssDxChCscdPortTypeSet                            macDrvMacCscdPortTypeSetFunc
 *        cpssDxChCscdPortTypeGet                            macDrvMacCscdPortTypeGetFunc
 *        cpssDxChPortMacSaLsbSet                            macDrvMacSaLsbSetFunc
 *        cpssDxChPortMacSaLsbGet                            macDrvMacSaLsbGetFunc
 *        cpssDxChPortInterfaceModeGet                       macDrvMacPortInterfaceModeGetFunc
 *---------------------------------------------------------------------------------------------
 */
typedef struct{
/* 1*/CPSS_MACDRV_MAC_SPEED_SET_FUNC              macDrvMacSpeedSetFunc;
/* 2*/CPSS_MACDRV_MAC_SPEED_GET_FUNC              macDrvMacSpeedGetFunc;
/* 3*/CPSS_MACDRV_DUPLEX_AN_SET_FUNC              macDrvMacDuplexANSetFunc;
/* 4*/CPSS_MACDRV_DUPLEX_AN_GET_FUNC              macDrvMacDuplexANGetFunc;
/* 5*/CPSS_MACDRV_FLOW_CNTL_AN_SET_FUNC           macDrvMacFlowCntlANSetFunc;
/* 6*/CPSS_MACDRV_FLOW_CNTL_AN_GET_FUNC           macDrvMacFlowCntlANGetFunc;
/* 7*/CPSS_MACDRV_SPEED_AN_SET_FUNC               macDrvMacSpeedANSetFunc;
/* 8*/CPSS_MACDRV_SPEED_AN_GET_FUNC               macDrvMacSpeedANGetFunc;
/* 9*/CPSS_MACDRV_FC_ENABLE_SET_FUNC              macDrvMacFlowCntlSetFunc;
/*10*/CPSS_MACDRV_FC_ENABLE_GET_FUNC              macDrvMacFlowCntlGetFunc;
/*11*/CPSS_MACDRV_PERIODIC_FC_ENABLE_SET_FUNC     macDrvMacPeriodFlowCntlSetFunc;
/*12*/CPSS_MACDRV_PERIODIC_FC_ENABLE_GET_FUNC     macDrvMacPeriodFlowCntlGetFunc;
/*13*/CPSS_MACDRV_BP_ENABLE_SET_FUNC              macDrvMacBackPrSetFunc;
/*14*/CPSS_MACDRV_BP_ENABLE_GET_FUNC              macDrvMacBackPrGetFunc;
/*15*/CPSS_MACDRV_PORT_LINK_STATUS_GET_FUNC       macDrvMacPortlinkGetFunc;
/*16*/CPSS_MACDRV_PORT_DUPLEX_SET_FUNC            macDrvMacDuplexSetFunc;
/*17*/CPSS_MACDRV_PORT_DUPLEX_GET_FUNC            macDrvMacDuplexGetFunc;
/*18*/CPSS_MACDRV_PORT_ENABLE_SET_FUNC            macDrvMacPortEnableSetFunc;
/*19*/CPSS_MACDRV_PORT_ENABLE_GET_FUNC            macDrvMacPortEnableGetFunc;
/*20*/CPSS_MACDRV_EXCL_COL_DROP_ENABLE_SET_FUNC   macDrvMacExcessiveCollisionDropSetFunc;
/*21*/CPSS_MACDRV_EXCL_COL_DROP_ENABLE_GET_FUNC   macDrvMacExcessiveCollisionDropGetFunc;
/*22*/CPSS_MACDRV_PADDING_ENABLE_SET_FUNC         macDrvMacPaddingEnableSetFunc;
/*23*/CPSS_MACDRV_PADDING_ENABLE_GET_FUNC         macDrvMacPaddingEnableGetFunc;
/*24*/CPSS_MACDRV_PREAMBLE_LENGTH_SET_FUNC        macDrvMacPreambleLengthSetFunc;
/*25*/CPSS_MACDRV_PREAMBLE_LENGTH_GET_FUNC        macDrvMacPreambleLengthGetFunc;
/*26*/CPSS_MACDRV_CRC_CHECK_ENABLE_SET_FUNC       macDrvMacCRCCheckSetFunc;
/*27*/CPSS_MACDRV_CRC_CHECK_ENABLE_GET_FUNC       macDrvMacCRCCheckGetFunc;
/*28*/CPSS_MACDRV_MRU_SET_FUNC                    macDrvMacMRUSetFunc;
/*29*/CPSS_MACDRV_MRU_GET_FUNC                    macDrvMacMRUGetFunc;
    CPSS_MACDRV_PORT_POWER_DOWN_SET_FUNC          macDrvMacPortPowerDownSetFunc;

    CPSS_MACDRV_COUNTERS_OBJ_STC                  macCounters;

    CPSS_MACDRV_PORT_MAC_TYPE_GET_FUNC            macDrvMacTypeGetFunc;
    CPSS_MACDRV_PORT_LOOPBACK_SET_FUNC            macDrvMacPortLoopbackSetFunc;
    CPSS_MACDRV_PORT_LOOPBACK_GET_FUNC            macDrvMacPortLoopbackGetFunc;
    CPSS_MACDRV_PORT_FORCE_LINK_DOWN_SET_FUNC     macDrvMacPortForceLinkDownSetFunc;
    CPSS_MACDRV_PORT_FORCE_LINK_DOWN_GET_FUNC     macDrvMacPortForceLinkDownGetFunc;
    CPSS_MACDRV_PORT_FORCE_LINK_PASS_SET_FUNC     macDrvMacPortForceLinkPassEnableSetFunc;
    CPSS_MACDRV_PORT_FORCE_LINK_PASS_GET_FUNC     macDrvMacPortForceLinkPassEnableGetFunc;
    CPSS_MACDRV_HW_SMI_REGISTER_SET_FUNC          macDrvMacHwSmiRegisterSetFunc;
    CPSS_MACDRV_HW_SMI_REGISTER_GET_FUNC          macDrvMacHwSmiRegisterGetFunc;
    CPSS_MACDRV_HW_10G_SMI_REGISTER_SET_FUNC      macDrvMacHw10GSmiRegisterSetFunc;
    CPSS_MACDRV_HW_10G_SMI_REGISTER_GET_FUNC      macDrvMacHw10GSmiRegisterGetFunc;
    CPSS_MACDRV_PORT_ATTRIBUTES_GET_FUNC          macDrvMacPortAttributesGetFunc;
    CPSS_MACDRV_CSCD_PORT_TYPE_SET_FUNC           macDrvMacCscdPortTypeSetFunc;
    CPSS_MACDRV_CSCD_PORT_TYPE_GET_FUNC           macDrvMacCscdPortTypeGetFunc;
    CPSS_MACDRV_PORT_INTERFACE_SPEED_SUPPORT_GET_FUNC
                                                  macDrvMacPortInterfaceSpeedSupportGetFunc;
    CPSS_MACDRV_MAC_SA_LSB_SET_FUNC               macDrvMacSaLsbSetFunc;
    CPSS_MACDRV_MAC_SA_LSB_GET_FUNC               macDrvMacSaLsbGetFunc;
    CPSS_MACDRV_MAC_SA_BASE_SET_FUNC              macDrvMacSaBaseSetFunc;
    CPSS_MACDRV_PORT_INTERFACE_MODE_GET_FUNC      macDrvMacPortInterfaceModeGetFunc;
    CPSS_MACDRV_PORT_TPID_PROFILE_SET_FUNC        macDrvMacPortTpidProfileSetFunc;
    CPSS_MACDRV_PORT_MAC_STATUS_GET_FUNC          macDrvMacPortMacStatusGetFunc;
    CPSS_MACDRV_PORT_MAC_DEFAULT_UP_SET_FUNC      macDrvMacPortDefaultUPSetFunc;

    CPSS_MACDRV_CN_FC_TIMER_SET_FUNC              macDrvMacCnFcTimerSetFunc;
    CPSS_MACDRV_CN_FC_TIMER_GET_FUNC              macDrvMacCnFcTimerGetFunc;
    CPSS_MACDRV_FC_MODE_SET_FUNC                  macDrvMacFcModeSetFunc;
    CPSS_MACDRV_FC_MODE_GET_FUNC                  macDrvMacFcModeGetFunc;
    CPSS_MACDRV_FC_PARAMS_SET_FUNC                macDrvMacFcParamsSetFunc;
    CPSS_MACDRV_FC_PARAMS_GET_FUNC                macDrvMacFcParamsGetFunc;

    CPSS_MACDRV_VCT_LENGTH_OFFSET_SET_FUNC        macDrvMacVctLengthOffsetSetFunc;
    CPSS_MACDRV_VCT_LENGTH_OFFSET_GET_FUNC        macDrvMacVctLengthOffsetGetFunc;

    CPSS_MACDRV_EEE_CONFIG_SET_FUNC               macDrvMacEeeConfigSetFunc;
    CPSS_MACDRV_EEE_CONFIG_GET_FUNC               macDrvMacEeeConfigGetFunc;
    CPSS_MACDRV_EEE_STATUS_GET_FUNC               macDrvMacEeeStatusGetFunc;

    CPSS_MACDRV_LED_GLOBAL_CONFIG_SET_FUNC        macDrvMacLedGlobalConfigSetFunc;
    CPSS_MACDRV_LED_GLOBAL_CONFIG_GET_FUNC        macDrvMacLedGlobalConfigGetFunc;
    CPSS_MACDRV_LED_PER_PORT_CONFIG_SET_FUNC      macDrvMacLedPerPortConfigSetFunc;
    CPSS_MACDRV_LED_PER_PORT_CONFIG_GET_FUNC      macDrvMacLedPerPortConfigGetFunc;

    CPSS_MACDRV_GPIO_CONFIG_SET_FUNC              macDrvMacGpioConfigSetFunc;
    CPSS_MACDRV_GPIO_CONFIG_GET_FUNC              macDrvMacGpioConfigGetFunc;
    CPSS_MACDRV_GPIO_DATA_READ_FUNC               macDrvMacGpioDataReadFunc;
    CPSS_MACDRV_GPIO_DATA_WRITE_FUNC              macDrvMacGpioDataWriteFunc;

    CPSS_MACDRV___STUB__FUNC                      objStubFunc;
} CPSS_MACDRV_OBJ_STC;



#ifdef __cplusplus
}
#endif


#endif  /* __cpssExtMacDrvH */



