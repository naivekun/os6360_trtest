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
* @file prvCpssPortTypes.h
*
* @brief CPSS port related definitions
*
* @version   18
********************************************************************************
*/

#ifndef __prvCpssPortTypesh
#define __prvCpssPortTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
/* get CPSS definitions for port configurations */
#include <cpss/common/port/cpssPortCtrl.h>
/* include the phy types Info */
#include <cpss/common/phy/private/prvCpssPhy.h>
#ifdef CHX_FAMILY
#include <cpss/generic/extMac/cpssExtMacDrv.h>
#endif /*CHX_FAMILY*/

#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpss/common/port/private/prvCpssPortManagerTypes.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>

/* check if the erratum need to be WA by PSS
    devNum  - the device id of the cpss device
    portNum - port number
    FErNum  - erratum number
*/
#define PRV_CPSS_PHY_ERRATA_GET(devNum, portNum, FErNum) \
  ((PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].phyErrataWa >> FErNum) & 0x1)? \
      GT_TRUE : GT_FALSE)

/*  set that the erratum need to be WA by PSS
    devNum  - the device id of the cpss device
    portNum - port number
    FErNum  - erratum number
*/
#define PRV_CPSS_PHY_ERRATA_SET(devNum, portNum, FErNum) \
    (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].phyErrataWa |= (1 << FErNum))

/**
* @enum PRV_CPSS_PORT_TYPE_ENT
 *
 * @brief Port Type speed enumeration
*/
typedef enum{

    /** port not exists in the device. */
    PRV_CPSS_PORT_NOT_EXISTS_E,

    /** 10/100 Mbps - using GE MAC Unit */
    PRV_CPSS_PORT_FE_E,

    /** 10/100/1000 Mbps - using GE MAC Unit */
    PRV_CPSS_PORT_GE_E,

    /** using XG MAC Unit */
    PRV_CPSS_PORT_XG_E,

    /** using XLG MAC Unit */
    PRV_CPSS_PORT_XLG_E,

    /** using HGL MAC Unit */
    PRV_CPSS_PORT_HGL_E,

    /** using CG MAC Unit (100Gbps) */
    PRV_CPSS_PORT_CG_E,

    /** using Interlaken MAC Unit */
    PRV_CPSS_PORT_ILKN_E,

    /** using MTI MAC 64 Unit(1G to 100G) */
    PRV_CPSS_PORT_MTI_64_E,

    /** using MTI MAC 400 Unit(200G to 400G) */
    PRV_CPSS_PORT_MTI_400_E,

    /** @brief not applicable mode, for
     *  validity checks and loops
     */
    PRV_CPSS_PORT_NOT_APPLICABLE_E

} PRV_CPSS_PORT_TYPE_ENT;

/**
* @enum PRV_CPSS_PORT_TYPE_OPTIONS_ENT
 *
 * @brief Port type options
*/
typedef enum{

    /** The port works as XG port only */
    PRV_CPSS_XG_PORT_XG_ONLY_E,

    /** The port works as HX/QX port only */
    PRV_CPSS_XG_PORT_HX_QX_ONLY_E,

    /** The port can work as either XG or HX/QX */
    PRV_CPSS_XG_PORT_XG_HX_QX_E,

    /** The port works as GE port only */
    PRV_CPSS_GE_PORT_GE_ONLY_E,

    /** The port supporting modes from SGMII up to XLG */
    PRV_CPSS_XG_PORT_XLG_SGMII_E,

    /** The port supporting modes from SGMII up to 100G */
    PRV_CPSS_XG_PORT_CG_SGMII_E,

    /** for validity checks etc. */
    PRV_CPSS_XG_PORT_OPTIONS_MAX_E

} PRV_CPSS_PORT_TYPE_OPTIONS_ENT;

/**
* @struct PRV_CPSS_PORT_FC_THRESHOLD_STC
 *
 * @brief Port Flow Control Threshold parameters
*/
typedef struct{

    /** current port X */
    GT_U16 xonLimit;

    /** current port X */
    GT_U16 xoffLimit;

} PRV_CPSS_PORT_FC_THRESHOLD_STC;
/**
* @enum PRV_CPSS_PHY_ERRATA_WA_ENT
 *
 * @brief enum for Work Arounds of PHY erratum regarding IEEE test modes
*/
typedef enum{

    /** @brief WA of PHY 88E1149 C1 in the
     *  Giga mode. Allows hiddden
     *  register's update
     */
    PRV_CPSS_PHY_ERRATA_1149_REG_9_WRITE_WA_E ,

    /** last errata */
    PRV_CPSS_PHY_ERRATA_MAX_NUM_E

} PRV_CPSS_PHY_ERRATA_WA_ENT;


/**
* @struct PRV_CPSS_PORT_REF_CLOCK_SOURCE_STC
 *
 * @brief port reference clock source information structure
*/
typedef struct{

    /** @brief enable/disable override of default
     *  port refClock configuration
     */
    GT_BOOL enableOverride;

    /** port reference clock source value */
    CPSS_PORT_REF_CLOCK_SOURCE_ENT portRefClockSource;

} PRV_CPSS_PORT_REF_CLOCK_SOURCE_STC;


typedef struct
{
    GT_U32                              ledPosition;
    PRV_CPSS_PORT_TYPE_ENT              ledMacType;
}PRV_CPSS_PORT_LED_INFO_STC;

/**
* @struct PRV_CPSS_PORT_INFO_ARRAY_STC
 *
 * @brief Port information structure
*/
typedef struct
{
    PRV_CPSS_PORT_FC_THRESHOLD_STC      portFcParams;
    PRV_CPSS_PORT_TYPE_ENT              portType;
    PRV_CPSS_PHY_PORT_SMI_IF_INFO_STC   smiIfInfo;
    CPSS_PHY_XSMI_INTERFACE_ENT         xsmiInterface;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT      portTypeOptions;
    GT_U32                              phyErrataWa;
    GT_BOOL                             isFlexLink;
    GT_32                               vctLengthOffset;
    CPSS_PORT_INTERFACE_MODE_ENT        portIfMode;
    CPSS_PORT_SPEED_ENT                 portSpeed;
#ifdef CHX_FAMILY
    CPSS_MACDRV_OBJ_STC  *              portMacObjPtr;
#endif /*CHX_FAMILY*/
    GT_U32                              serdesOptAlgBmp;
    PRV_CPSS_PORT_REF_CLOCK_SOURCE_STC  portRefClock;
    PRV_CPSS_PORT_LED_INFO_STC          portLedInfo;
} PRV_CPSS_PORT_INFO_ARRAY_STC;


/*******************************************************************************
*  PRV_CPSS_PORT_CHECK_AND_GET_MAC_FUNC
*
*  DESCRIPTION:
*       .
*
*  APPLICABLE DEVICES: Dev1, Dev2.
*
*  INPUTS:
*       .
*
*  OUTPUTS:
*       .
*
*  RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - bad device number, or event convert type
*       GT_BAD_PTR  - evExtDataPtr is NULL pointer
*       GT_HW_ERROR - on hardware error
*       GT_NOT_SUPPORTED - not supported for current device family
*
*  COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_PORT_CHECK_AND_GET_MAC_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_U32               *portMacNum
);

typedef GT_STATUS (*PRV_CPSS_PORT_LINK_STATUS_GET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_BOOL              *linkUp
);

typedef GT_STATUS (*PRV_CPSS_PORT_HW_UNRESET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum
);

typedef GT_STATUS (*PRV_CPSS_PORT_REMOTE_FAULT_SET_FUNC)
(
    GT_SW_DEV_NUM           devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    CPSS_PORT_SPEED_ENT     speed,
    GT_BOOL                 state
);

typedef GT_STATUS (*PRV_CPSS_PORT_FORCE_LINK_DOWN_SET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_BOOL              state
);

typedef GT_STATUS (*PRV_CPSS_PORT_ENABLE_SET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_BOOL              enable
);

typedef GT_STATUS (*PRV_CPSS_PORT_AUTO_TUNE_STATUS_GET_FUNC)
(
    GT_SW_DEV_NUM           devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_SIG_DET_GET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_BOOL              *enable
);

typedef GT_STATUS (*PRV_CPSS_PORT_CG_CONVERTERS_GET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_U32               *regAddr
);

typedef GT_STATUS (*PRV_CPSS_PORT_CG_CONVERTERS_STATUS2_GET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_U32               *regAddr
);


typedef GT_STATUS (*PRV_CPSS_PORT_MODE_SPEED_SET_FUNC)
(
    GT_SW_DEV_NUM       devNum,
    CPSS_PORTS_BMP_STC  portsBmp,
    GT_BOOL             powerUp,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    CPSS_PORT_SPEED_ENT           speed
);

typedef GT_STATUS (*PRV_CPSS_PORT_TUNE_SET_FUNC)
(
    GT_SW_DEV_NUM                       devNum,
    GT_PHYSICAL_PORT_NUM                portNum,
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
);

typedef GT_STATUS (*PRV_CPSS_PORT_MAC_CONVERT_FUNC)
(
    GT_SW_DEV_NUM devNum,
    PRV_CPSS_EV_CONVERT_DIRECTION_ENT evConvertType,
    GT_U32 portNumFrom,
    GT_U32 *portNumToPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_SPEED_GET_FUNC)
(
    GT_SW_DEV_NUM         devNum,
    GT_PHYSICAL_PORT_NUM  portNum,
    CPSS_PORT_SPEED_ENT   *speedPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_INTERFACE_GET_FUNC)
(
    GT_SW_DEV_NUM                devNum,
    GT_PHYSICAL_PORT_NUM         portNum,
    CPSS_PORT_INTERFACE_MODE_ENT *ifModePtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_SERDES_NUM_GET_FUNC)
(
    GT_SW_DEV_NUM                devNum,
    GT_PHYSICAL_PORT_NUM         portNum,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    GT_U32                       *startSerdesPtr,
    GT_U32                       *numOfSerdesLanesPtr
);

typedef GT_U32 (*PRV_CPSS_PORT_NUM_OF_SERDES_GET_FUNC)
(
    GT_SW_DEV_NUM devNum
);

typedef GT_STATUS (*PRV_CPSS_PORT_MANAGER_DB_GET_FUNC)
(
    GT_SW_DEV_NUM            devNum,
    PRV_CPSS_PORT_MNG_DB_STC **portManagerDbPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_MPCS40G_REG_ADDR_GET_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    GT_U32               *regAddrPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_MAC_DMA_MODE_REG_ADDR_GET_FUNC)
(
    GT_SW_DEV_NUM   devNum,
    GT_U32          portMacNum,
    GT_U32          *regAddrPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_LINK_STATUS_CHANGED_NOTIFY_FUNC)
(
    GT_SW_DEV_NUM        devNum,
    GT_PHYSICAL_PORT_NUM portNum
);

typedef GT_STATUS (*PRV_CPSS_PORT_XLG_MAC_MASK_GET_FUNC)
(
    GT_SW_DEV_NUM   devNum,
    GT_U32          portMacNum,
    GT_U32          *regAddr
);

typedef GT_STATUS (*PRV_CPSS_PORT_HW_RESET_FUNC)
(
    GT_SW_DEV_NUM           devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    GT_BOOL                 qsgmiiFullDelete
);

typedef GT_STATUS (*PRV_CPSS_PORT_FEC_MODE_SET_FUNC)
(
    GT_SW_DEV_NUM           devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    CPSS_PORT_FEC_MODE_ENT  mode
);

typedef GT_STATUS (*PRV_CPSS_PORT_FEC_MODE_GET_FUNC)
(
    GT_SW_DEV_NUM           devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    CPSS_PORT_FEC_MODE_ENT  *mode
);

typedef GT_STATUS (*PRV_CPSS_PORT_LANE_TUNE_SET_FUNC)
(
    GT_SW_DEV_NUM   devNum,
    GT_U32          portGroupNum,
    GT_U32          laneNum,
    CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    CPSS_PORT_SERDES_TUNE_STC   *tuneValuesPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_LANE_TUNE_GET_FUNC)
(
   GT_SW_DEV_NUM           devNum,
   GT_U32                  portGroupNum,
   GT_U32                  laneNum,
   CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
   CPSS_PORT_SERDES_TUNE_STC  *tuneValuesPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_MAC_CONFIG_CLEAR_FUNC)
(
    INOUT PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
);

typedef GT_STATUS (*PRV_CPSS_PORT_MAC_CONFIGURATION_FUNC)
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  const PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
);

typedef GT_STATUS (*PRV_CPSS_PORT_AP_CONFIG_SET_FUNC)
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  GT_BOOL                    apEnable,
    IN  CPSS_PORT_AP_PARAMS_STC    *apParamsPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_AP_STATUS_GET_FUNC)
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT CPSS_PORT_AP_STATUS_STC   *apStatusPtr
);


typedef GT_STATUS (*PRV_CPSS_PORT_AP_SERDES_TX_PARAMS_OFFSETS_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *serdesTxOffsetsPtr
);

typedef GT_STATUS (*PRV_CPSS_PORT_MAC_LOOPBACK_MODE_SET_FUNC)
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

typedef GT_STATUS (*PRV_CPSS_PORT_PCS_LOOPBACK_MODE_SET_FUNC)
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_PCS_LOOPBACK_MODE_ENT     mode
);

typedef GT_STATUS (*PRV_CPSS_PORT_SERDES_LOOPBACK_MODE_SET_FUNC)
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneBmp,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT  mode
);

typedef GT_STATUS (*PRV_CPSS_PORT_SERDES_POLARITY_SET_FUNC)
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32      portGroupNum,
    IN  GT_U32      laneNum,
    IN  GT_BOOL     invertTx,
    IN  GT_BOOL     invertRx
);

typedef GT_STATUS (*PRV_CPSS_PORT_HIGH_SPEED_SET_FUNC)
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_BOOL             enable
);

typedef GT_STATUS (*PRV_CPSS_PORT_SERDES_TUNE_RESULT_GET_FUNC)
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PORT_SERDES_TUNE_STC      *serdesTunePtr
);

typedef GT_STATUS (*PRV_CPSS_AUTO_NEG_1G_SGMII_FUNC)
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_BOOL                        enable
);

typedef GT_STATUS (*PRV_CPSS_PM_INIT_FUNC)
(
    IN  GT_SW_DEV_NUM       devNum
);

/**
* @struct PRV_CPSS_PORT_FUNC_PTRS_STC
 *
 * @brief Commong port functions.
*/
typedef struct{
    PRV_CPSS_PORT_CHECK_AND_GET_MAC_FUNC            ppCheckAndGetMacFunc;
    PRV_CPSS_PORT_LINK_STATUS_GET_FUNC              ppLinkStatusGetFunc;
    PRV_CPSS_PORT_HW_UNRESET_FUNC                   ppPortHwUnresetFunc;
    PRV_CPSS_PORT_HW_RESET_FUNC                     ppPortHwResetFunc;
    PRV_CPSS_PORT_REMOTE_FAULT_SET_FUNC             ppRemoteFaultSetFunc;
    PRV_CPSS_PORT_FORCE_LINK_DOWN_SET_FUNC          ppForceLinkDownSetFunc;
    PRV_CPSS_PORT_ENABLE_SET_FUNC                   ppPortMacEnableFunc;
    PRV_CPSS_PORT_AUTO_TUNE_STATUS_GET_FUNC         ppTuneStatusGetFunc;
    PRV_CPSS_PORT_SIG_DET_GET_FUNC                  ppSigDetGetFunc;
    PRV_CPSS_PORT_CG_CONVERTERS_GET_FUNC            ppCgConvertersGetFunc;
    PRV_CPSS_PORT_CG_CONVERTERS_STATUS2_GET_FUNC    ppCgConvertersStatus2GetFunc;
    PRV_CPSS_PORT_MODE_SPEED_SET_FUNC               ppPortModeSpeedSetFunc;
    PRV_CPSS_PORT_TUNE_SET_FUNC                     ppTuneExtSetFunc;
    PRV_CPSS_PORT_MAC_CONVERT_FUNC                  ppMacConvertFunc;
    PRV_CPSS_PORT_SPEED_GET_FUNC                    ppSpeedGetFromExtFunc;
    PRV_CPSS_PORT_INTERFACE_GET_FUNC                ppIfModeGetFromExtFunc;
    PRV_CPSS_PORT_SERDES_NUM_GET_FUNC               ppSerdesNumGetFunc;
    PRV_CPSS_PORT_NUM_OF_SERDES_GET_FUNC            ppNumOfSerdesGetFunc;
    PRV_CPSS_PORT_MANAGER_DB_GET_FUNC               ppPortManagerDbGetFunc;
    PRV_CPSS_PORT_MPCS40G_REG_ADDR_GET_FUNC         ppMpcs40GCommonStatusRegGetFunc;
    PRV_CPSS_PORT_MAC_DMA_MODE_REG_ADDR_GET_FUNC    ppMacDmaModeRegAddrGetFunc;
    PRV_CPSS_PORT_LINK_STATUS_CHANGED_NOTIFY_FUNC   ppLinkStatusChangedNotifyFunc;
    PRV_CPSS_PORT_XLG_MAC_MASK_GET_FUNC             ppXlgMacMaskAddrGetFunc;
    PRV_CPSS_PORT_FEC_MODE_SET_FUNC                 ppFecModeSetFunc;
    PRV_CPSS_PORT_FEC_MODE_GET_FUNC                 ppFecModeGetFunc;
    PRV_CPSS_PORT_LANE_TUNE_SET_FUNC                ppLaneTuneSetFunc;
    PRV_CPSS_PORT_LANE_TUNE_GET_FUNC                ppLaneTuneGetFunc;
    PRV_CPSS_PORT_MAC_CONFIG_CLEAR_FUNC             ppMacConfigClearFunc;
    PRV_CPSS_PORT_MAC_CONFIGURATION_FUNC            ppMacConfigurationFunc;
    PRV_CPSS_PORT_AP_CONFIG_SET_FUNC                ppPortApConfigSetFunc;
    PRV_CPSS_PORT_AP_STATUS_GET_FUNC                ppPortApStatusGetFunc;
    PRV_CPSS_PORT_AP_SERDES_TX_PARAMS_OFFSETS_SET_FUNC   ppPortApSerdesTxParamsOffsetSetFunc;
    PRV_CPSS_PORT_MAC_LOOPBACK_MODE_SET_FUNC        ppPortMacLoopbackModeSetFunc;
    PRV_CPSS_PORT_PCS_LOOPBACK_MODE_SET_FUNC        ppPortPcsLoopbackModeSetFunc;
    PRV_CPSS_PORT_SERDES_LOOPBACK_MODE_SET_FUNC     ppPortSerdesLoopbackModeSetFunc;
    PRV_CPSS_PORT_SERDES_POLARITY_SET_FUNC          ppPortSerdesPolaritySetFunc;
    PRV_CPSS_PORT_HIGH_SPEED_SET_FUNC               ppPortProprietaryHighSpeedPortsSetFunc;
    PRV_CPSS_PORT_SERDES_TUNE_RESULT_GET_FUNC       ppPortSerdesTuneResultGetFunc;
    PRV_CPSS_AUTO_NEG_1G_SGMII_FUNC                 ppAutoNeg1GSgmiiFunc;
    PRV_CPSS_PM_INIT_FUNC                           ppPortMgrInitFunc;

}PRV_CPSS_PORT_FUNC_PTRS_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPortTypesh */


