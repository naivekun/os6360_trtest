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
* @file prvCpssDxChPortCtrl.h
*
* @brief Includes types and values definition and initialization for the use of
* CPSS DxCh Port Control feature.
*
*
* @version   29
********************************************************************************
*/
#ifndef __prvCpssDxChPortCtrlh
#define __prvCpssDxChPortCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
/* get common defs */
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/common/port/private/prvCpssPortManagerTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>


extern GT_BOOL  prvCpssDrvTraceHwDelay[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* serdesSpeed1 register value for 2.5 gig mode */
#define PRV_SERDES_SPEED_2500_REG_VAL_CNS 0x0000264A

/* serdesSpeed1 register value for 1 gig mode*/
#define PRV_SERDES_SPEED_1000_REG_VAL_CNS 0x0000213A

/* at this moment max number of serdeses that could be occupied by port
   is 8 for XLG mode */
#define PRV_CPSS_DXCH_MAX_SERDES_NUM_PER_PORT_CNS 8

#define PRV_PER_PORT_LOOP_MAC for(localPort = startSerdes/2; localPort*2 < startSerdes+serdesesNum; localPort++)
#define PRV_PER_SERDES_LOOP_MAC for (i = startSerdes; i < startSerdes+serdesesNum; i++)

/* macro for HW wait time for configuration, revords time delays if enabled */
#ifndef ASIC_SIMULATION
    #define HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,miliSec) if (prvCpssDrvTraceHwDelay[devNum] == GT_TRUE) \
                                                    {cpssTraceHwAccessDelay(devNum,portGroupId,miliSec); } \
                                                    cpssOsTimerWkAfter(miliSec)
#else /*ASIC_SIMULATION*/
/* the simulation NOT need those 'sleeps' needed in HW */
    #define HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,miliSec) if (prvCpssDrvTraceHwDelay[devNum] == GT_TRUE) \
                                                    {cpssTraceHwAccessDelay(devNum,portGroupId,miliSec); }
#endif /*ASIC_SIMULATION*/

#define PRV_CPSS_DXCH_BC2_PORT_TXDMA_CONFIG_CALC_MAC(speedForCalc) (((GT_U32)((350+(speedForCalc-1))/speedForCalc))<<3)

/* Applicable to SIP_5_20 and up only */
#define PRV_CPSS_DXCH_EGF_CONVERT_MAC(portEgfLinkStatusState) (portEgfLinkStatusState==CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E?0:1)
#define PRV_CPSS_DXCH_HW_2_EGF_CONVERT_MAC(value) (value==0?CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E:CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E)

/* index in ...SerdesPowerUpSequence arrays of serdes registers defining
    serdes frequency */
#define PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_INDEX_CNS   1
#define PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_INDEX_CNS   2
#define PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_INDEX_E     11
#define PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E     12
#define PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG2_INDEX_E     13
#define PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E          16
#define PRV_CPSS_DXCH_PORT_SERDES_DFE_REG0_INDEX_E          17/* Lion specific */
#define PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_XCAT_INDEX_E  21
#define PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_LION_INDEX_E  22

/* in Lion2,3 traffic arbiter must share time between ports of GOP accordingly to number
   of ports in it */
#define PRV_CPSS_DXCH_LION2_3_THREE_MINI_GOPS_PIZZA_SLICES_NUM_CNS 12
#define PRV_CPSS_DXCH_LION2_3_FOUR_MINI_GOPS_PIZZA_SLICES_NUM_CNS 16

/**
* @internal prvCpssDxChPortFlowControlReceiveFramesModeSet function
* @endinternal
*
* @brief  Set mode of received PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] fcMode                   - receive FC frames mode - see comments in enum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortFlowControlReceiveFramesModeSet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
);

/**
* @internal prvCpssDxChPortFlowControlReceiveFramesModeGet function
* @endinternal
*
* @brief  Get mode of received PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[out] fcModePtr               - (pointer to)receive FC frames mode - see comments in enum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortFlowControlReceiveFramesModeGet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT *fcModePtr
);

/* array defining possible interfaces/ports modes configuration options */
/* APPLICABLE DEVICES:  DxCh */
/* extern GT_BOOL supportedPortsModes[PRV_CPSS_XG_PORT_OPTIONS_MAX_E][CPSS_PORT_INTERFACE_MODE_NA_E]; */
GT_BOOL prvCpssDxChPortTypeSupportedModeCheck
(
    IN PRV_CPSS_PORT_TYPE_OPTIONS_ENT portType,
    IN CPSS_PORT_INTERFACE_MODE_ENT   ifMode
);

GT_STATUS prvCpssDxChPortSupportedModeCheck
(
    IN  GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM          portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    OUT GT_BOOL                      *isSupportedPtr
);

typedef struct
{
    GT_BOOL                         valid;
    MV_HWS_INPUT_SRC_CLOCK          inputSrcClk;
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq;
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq;

} PRV_CPSS_DXCH_PORT_CPLL_CONFIG_STC;

/**
* @struct PRV_CPSS_DXCH_PORT_STATE_STC
 *
 * @brief A struct containing port attributes that should be
 * stored before certain port configurations and restored
 * after that.
*/
typedef struct{

    /** port enable state */
    GT_U32 portEnableState;

    /** @brief link port state in EGF
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT egfPortLinkStatusState;

} PRV_CPSS_DXCH_PORT_STATE_STC;

/* current max value is : PRV_CPSS_FALCON_12_8_SERDES_NUM_CNS */
#define PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM       258

extern PRV_CPSS_DXCH_PORT_CPLL_CONFIG_STC prvCpssDxChPortCpllConfigArr[PRV_CPSS_DXCH_PORT_SERDES_MAX_NUM];

/*******************************************************************************
* PRV_CPSS_PORT_MODE_SPEED_TEST_FUN
*
* DESCRIPTION:
*      The function checks if specific interface mode and speed compatible with
*       given port interface
*
* INPUTS:
*       devNum  - device number
*       portNum - port number
*
* OUTPUTS:
*       status - pointer to test result:
*         GT_TRUE - suggested mode/speed pair compatible with given port interface,
*         GT_FALSE - otherwise
*
*
* RETURNS:
*       GT_OK           - on success,
*       GT_BAD_PARAM    - bad devNum, smiInterface
*       GT_BAD_PTR      - pointer to place data is NULL
*       GT_HW_ERROR     - hw error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_PORT_MODE_SPEED_TEST_FUN)
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *status
);

/**
* @internal prvCpssDxChPortIfCfgInit function
* @endinternal
*
* @brief   Initialize port interface mode configuration method in device object
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - port object allocation failed
* @retval GT_FAIL                  - wrong devFamily
*/
GT_STATUS prvCpssDxChPortIfCfgInit
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] startSerdes              - first SERDES number
* @param[in] serdesesNum              - number of SERDESes
* @param[in] state                    - Reset state
*                                      GT_TRUE  - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32    startSerdes,
    IN  GT_U32    serdesesNum,
    IN  GT_BOOL   state
);

/**
* @internal prvCpssDxChPortStateDisableAndGet function
* @endinternal
*
* @brief   Disable port and get current port parameters that should be
*         restored after port configuration.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStateStcPtr          - (pointer to) the port state struct for saving parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortStateDisableAndGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT PRV_CPSS_DXCH_PORT_STATE_STC    *portStateStcPtr
);

/**
* @internal prvCpssDxChPortStateRestore function
* @endinternal
*
* @brief   Restore port parameters that was saved before port configuration.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portStateStcPtr          - (pointer to) the port state struct for restoring parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortStateRestore
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CPSS_DXCH_PORT_STATE_STC    *portStateStcPtr
);

/**
* @internal prvCpssDxChPortInbandAutonegMode function
* @endinternal
*
* @brief   Set inband autoneg mode accordingly to required ifMode
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] ifMode                   - port interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when <InBandAnEn> is set to 1.
*       Not relevant for the CPU port.
*       This field may only be changed when the port link is down.
*       In existing devices inband auto-neg. disabled by default.
*
*/
GT_STATUS prvCpssDxChPortInbandAutonegMode
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
);

/**
* @internal prvCpssDxChLpGetFirstInitSerdes function
* @endinternal
*
* @brief   Get first initialize serdes per port
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] startSerdes              - start SERDES number
*
* @param[out] initSerdesNumPtr         - pointer to first initialize serdes number per port
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpGetFirstInitSerdes
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 startSerdes,
    OUT GT_U32 *initSerdesNumPtr
);

/**
* @internal prvCpssDxChLpCheckSerdesInitStatus function
* @endinternal
*
* @brief   Check LP SERDES initialization status.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] serdesNum                - SERDES number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpCheckSerdesInitStatus
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               serdesNum
);

/**
* @internal prvGetLpSerdesSpeed function
* @endinternal
*
* @brief   Gets LP serdes speed.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - port group Id, support multi-port-groups device
* @param[in] serdesNum                - number of first serdes of configured port
*
* @param[out] serdesSpeedPtr           - (pointer to) serdes speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvGetLpSerdesSpeed
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               serdesNum,
    OUT CPSS_DXCH_PORT_SERDES_SPEED_ENT  *serdesSpeedPtr
);

/**
* @internal prvCpssDxCh3SerdesSpeedModeGet function
* @endinternal
*
* @brief   Gets speed mode for specified port serdes on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesSpeed_2500_EnPtr   - pointer to serdes speed mode.
*                                      GT_TRUE  - serdes speed is 2.5 gig.
*                                      GT_FALSE - other serdes speed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxCh3SerdesSpeedModeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *serdesSpeed_2500_EnPtr
);

/**
* @internal prvCpssDxChXcatPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Set power up or down state to port and serdes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - TX/RX/BOTH serdes  (ignored)
* @param[in] lanesBmp                 - lanes bitmap         (ignored)
* @param[in] powerUp                  - power up state:
*                                      GT_TRUE  - power up
*                                      GT_FALSE - power down
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChXcatPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

/**
* @internal serdesSpeedSet function
* @endinternal
*
* @brief   Configure serdes registers uniqiue for specific frequency
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - the speed is not supported on the port
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong speed
*/
GT_STATUS serdesSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal prvCpssDxChPortSpeedForCutThroughWaCalc function
* @endinternal
*
* @brief   Calculate port speed HW value and field offsets in RX and TX DMAs
*         for Cut Throw WA - packet from slow to fast port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] localPortNum             - local port number
* @param[in] speed                    - port speed
*
* @param[out] hwSpeedPtr               - pointer to 2-bit HW speed value.
* @param[out] rxRegAddrPtr             - pointer to address of relevant RX DMA register.
* @param[out] txRegAddrPtr             - pointer to address of relevant TX DMA register.
* @param[out] rxRegOffPtr              - pointer to bit offset of field in relevant RX DMA register.
* @param[out] txRegOffPtr              - pointer to bit offset of field in relevant TX DMA register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChPortSpeedForCutThroughWaCalc
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM localPortNum,
    IN  CPSS_PORT_SPEED_ENT  speed,
    OUT GT_U32               *hwSpeedPtr,
    OUT GT_U32               *rxRegAddrPtr,
    OUT GT_U32               *txRegAddrPtr,
    OUT GT_U32               *rxRegOffPtr,
    OUT GT_U32               *txRegOffPtr
);

/**
* @internal prvCpssDxChPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*                                       (For DxCh3, not XG ports only.)
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssDxChPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*       4.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvCpssDxChPortSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal prvCpssDxChPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS prvCpssDxChPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);

/**
* @internal prvCpssDxChPortGePortTypeSet function
* @endinternal
*
* @brief   Set port type and inband auto-neg. mode of GE MAC of port
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] ifMode                   - port interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGePortTypeSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
);

/**
* @internal prvCpssDxChPortForceLinkDownEnable function
* @endinternal
*
* @brief   Enable Force link down on a specified port on specified device and
*         read current force link down state of it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
*
* @param[out] linkDownStatusPtr        - (ptr to) current force link down state of port:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnable
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL *linkDownStatusPtr
);

/**
* @internal prvCpssDxChPortForceLinkDownDisable function
* @endinternal
*
* @brief   Disable Force link down on a specified port on specified device if
*         it was previously disabled.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] linkDownStatus           - previous force link down state:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownDisable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL linkDownStatus
);


/**
* @internal prvCpssDxChPortInterfaceModeHwGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port from HW.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*
* @note For Lion no possibility to recognize CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,
*       because it's SW term - function will return CPSS_PORT_INTERFACE_MODE_XGMII_E
*
*/
GT_STATUS prvCpssDxChPortInterfaceModeHwGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/**
* @internal prvCpssDxChPortSpeedHwGet function
* @endinternal
*
* @brief   Gets from HW speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Lion; xCat; Lion2; xCat2; Bobcat2; xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS prvCpssDxChPortSpeedHwGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);

/**
* @internal prvCpssDxChPortTypeSet function
* @endinternal
*
* @brief   Sets port type (mostly means which mac unit used) on a specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - Interface mode.
* @param[in] speed                    - port speed
*                                       None.
*/
GT_VOID prvCpssDxChPortTypeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT            speed
);

/**
* @internal prvCpssDxChSerdesRefClockTranslateCpss2Hws function
* @endinternal
*
* @brief   Get serdes referense clock from CPSS DB and translate it to HWS format
*
* @param[in] devNum                   - physical device number
*
* @param[out] refClockPtr              - (ptr to) serdes referense clock in HWS format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if value from CPSS DB not supported
*/
GT_STATUS prvCpssDxChSerdesRefClockTranslateCpss2Hws
(
    IN  GT_U8 devNum,
    OUT MV_HWS_REF_CLOCK_SUP_VAL *refClockPtr
);

/**
* @internal prvCpssDxChPortForceLinkDownEnableSetMac function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnableSetMac
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
);

/**
* @internal prvCpssDxChPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
* @param[in] modifyRemoteFaultSend    - whether or not to change
*                                       fault signals status
*                                       which cause partner link
*                                       to go down
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   modifyRemoteFaultSend
);

/**
* @internal prvCpssDxChPortLion2LinkFixWa function
* @endinternal
*
* @brief   For Lion2 A0 "no allignment lock WA"
*         For Lion2 B0 "40G connect/disconnect WA", "false link up WA"
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChPortLion2LinkFixWa
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal prvCpssDxChPortLion2RxauiLinkFixWa function
* @endinternal
*
* @brief   Run RXAUI link WA
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChPortLion2RxauiLinkFixWa
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal prvCpssDxChPortLion2GeLinkStatusWaEnableSet function
* @endinternal
*
* @brief   Enable/disable WA for FE-4933007 (In Lion2 port in tri-speed mode link
*         status doesn't change in some cases when cable is disconnected/connected.)
*         Must disable WA if any type of loopback defined on GE port to see link up.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  WA
*                                      If GT_FALSE, disable WA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortLion2GeLinkStatusWaEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal prvCpssDxChLion2PortTypeSet function
* @endinternal
*
* @brief   Sets port type (mostly means which mac unit used) on a specified port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - Interface mode.
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - the speed is not supported on the port
*/
GT_STATUS prvCpssDxChLion2PortTypeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);


/**
* @internal geMacUnitSpeedSet function
* @endinternal
*
* @brief   Configure GE MAC unit of specific device and port to required speed
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong speed
*/
GT_STATUS geMacUnitSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @enum PRV_DXCH_PORT_MAC_CNTR_READ_MODE_ENT
 *
 * @brief Enumeration of the Get MAC Counters function mode.
*/
typedef enum{

    /** usual mode for API call */
    PRV_DXCH_PORT_MAC_CNTR_READ_MODE_USUAL_E,

    /** @brief read MAC counters from HW
     *  and update shadow DB
     */
    PRV_DXCH_PORT_MAC_CNTR_READ_MODE_UPDATE_SHADOW_E,

    /** @brief reset counters in HW by read
     *  them. Do not update shadow DB.
     */
    PRV_DXCH_PORT_MAC_CNTR_READ_MODE_RESET_HW_E

} PRV_DXCH_PORT_MAC_CNTR_READ_MODE_ENT;

/**
* @internal prvCpssDxChPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number,
*                                      CPU port if getFromCapture is GT_FALSE
* @param[in] getFromCapture           -  GT_TRUE -  Gets the captured Ethernet MAC counter
*                                      GT_FALSE - Gets the Ethernet MAC counter
* @param[in] readMode                 - read counters procedure mode
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvCpssDxChPortMacCountersOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         getFromCapture,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN  PRV_DXCH_PORT_MAC_CNTR_READ_MODE_ENT readMode
);


/**
* @internal prvCpssDxChPortMacCountersSpecialShadowReset function
* @endinternal
*
* @brief   Reset Special Mib conters packet in Shadow to work arround hardware error.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacCountersSpecialShadowReset
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
);


/**
* @internal prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType
);


/**
* @internal prvCpssDxChPortLion2InterfaceModeHwGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port from HW.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS prvCpssDxChPortLion2InterfaceModeHwGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/* CPSS_PORT_SPEED_47200_E */
typedef struct prvCpssDxChLion2PortInfo_STC
{
    GT_U32 rxDmaIfWidth;                  /* 0 - 64bit       2 - 256 bit      */
    GT_U32 rxDmaSource;                   /* 0 - regular Mac 1 - extended Mac */
    GT_U32 mppmXlgMode[2];                /* 0 - !40G        1 - 40G          */
    GT_U32 txDmaThhresholdOverrideEnable; /* 0 = Disable     1 - enable       */
}prvCpssDxChLion2PortInfo_STC;

GT_STATUS lion2PortInfoGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  prvCpssDxChLion2PortInfo_STC * portInfoPtr
);

/**
* @internal prvCpssDxChBobkPortSerdesTuning function
* @endinternal
*
* @brief   Configure serdes tuning values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChBobkPortSerdesTuning
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal prvCpssDxChPortSerdesPolaritySet function
* @endinternal
*
* @brief   Configure the Polarity values on Serdeses if SW DB values initialized.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
* @param[in] sdVectorPtr              - array of numbers of serdeses occupied by port
* @param[in] sdVecSize                - size of serdeses array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortSerdesPolaritySet
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroup,
    IN  GT_U16      *sdVectorPtr,
    IN  GT_U8       sdVecSize
);

/**
* @internal prvCpssDxChPortPtpReset function
* @endinternal
*
* @brief   Reset/unreset PTP unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] resetTxUnit              - GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[in] resetRxUnit              - GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPtpReset
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         resetTxUnit,
    IN  GT_BOOL                         resetRxUnit
);

/**
* @internal prvCpssDxChPortSerdesPartialPowerDownSet function
* @endinternal
*
* @brief   Set power down/up of Tx and Rx on Serdeses.
*         INPUTS:
*         devNum   - system device number
*         portNum   - physical port number
*         powerDownRx - Status of Serdes Rx (TRUE - power down,
*         FALSE - power up).
*         powerDownTx - Status of Serdes Tx (TRUE - power
*         down, FALSE - power up).
*         OUTPUTS: None.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical port number
* @param[in] powerDownRx              - Status of Serdes Rx
*                                       (TRUE - power down,
*                                       FALSE - power up).
* @param[in] powerDownTx              - Status of Serdes Tx
*                                       (TRUE - power down,
*                                       FALSE - power up).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failed.
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortSerdesPartialPowerDownSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  powerDownRx,
    IN GT_BOOL                  powerDownTx
);

/**
* @internal prvCpssDxChPortLion2InternalLoopbackEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortLion2InternalLoopbackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal prvCpssDxChPortBc2PtpTimeStampFixWa function
* @endinternal
*
* @brief   Set the thresholds in ports Tx FIFO
*
* @note   APPLICABLE DEVICES:      Bobcat2
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note WA fix PTP timestamp problem
*
*/
GT_STATUS prvCpssDxChPortBc2PtpTimeStampFixWa
(
    IN  GT_U8                           devNum
);

/**
* @internal prvCpssDxChPortAnDisableFixWa function
* @endinternal
*
* @brief   Disable the masking of pcs_rx_er when disabling the Auto-Neg on 10M/100M port speed, CRC error will be reported.
*         Due to missing byte in the packets, CRC errors will be reported
*
* @note   APPLICABLE DEVICES:      xCat2; xCat3; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] physicalPortNum          - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set field rf_pcs_rx_er_mask_disable to 0 in register Gige_MAC_IP_Unit
*
*/
GT_STATUS prvCpssDxChPortAnDisableFixWa
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    physicalPortNum
);

/**
* @internal prvCpssDxChPortCpllConfig function
* @endinternal
*
* @brief   CPLL initialization
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssDxChPortCpllConfig
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssDxChPortHwReset function
* @endinternal
*
* @brief   HW port reset. This API reset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] qsgmiiFullDelete         - qsgmii full config boolean
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortHwReset
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    GT_BOOL                         qsgmiiFullDelete
);

/**
* @internal prvCpssDxChPortHwUnreset function
* @endinternal
*
* @brief   HW port unreset. This API unreset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortHwUnreset
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal prvCpssDxChPortFecModeSetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortFecModeSet
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFecModeSetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_FEC_MODE_ENT  mode
);

/**
* @internal prvCpssDxChPortFecModeGetWrapper function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] mode                     - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFecModeGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_FEC_MODE_ENT  *mode
);

/**
* @internal prvCpssDxChPortSerdesLaneTuningGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortSerdesLaneTuningGet
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortSerdesLaneTuningGetWrapper
(
    IN   GT_SW_DEV_NUM  devNum,
    IN   GT_U32         portGroupNum,
    IN   GT_U32         laneNum,
    IN   CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_PORT_SERDES_TUNE_STC  *tuneValuesPtr
);

/**
* @internal prvCpssDxChPortSerdesLaneTuningSetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortSerdesLaneTuningSet
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortSerdesLaneTuningSetWrapper
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_U32                  portGroupNum,
    IN GT_U32                  laneNum,
    IN CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN CPSS_PORT_SERDES_TUNE_STC  *tuneValuesPtr
);

/**
* @internal prvCpssDxChPortSerdesAutoTuneExtWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortSerdesAutoTune
*         with extended implementation in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortSerdesAutoTuneExtWrapper
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
);

/**
* @internal prvCpssDxChPortSerdesAutoTuneStatusGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function prvCpssDxChPortSerdesAutoTuneStatusGet
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] rxTuneStatusPtr          - (pointer to) rx tune status parameter
* @param[out] rxTuneStatusPtr          - (pointer to) tx tune status parameter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortSerdesAutoTuneStatusGetWrapper
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
);

/**
* @internal prvCpssDxChPortMacConfigurationClearWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function prvCpssDxChPortMacConfigurationClear
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacConfigurationClearWrapper
(
    INOUT PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
);

/**
* @internal prvCpssDxChPortMacConfigurationWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function prvCpssDxChPortMacConfiguration
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regDataArray             - (pointer to) registers data array to use.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacConfigurationWrapper
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  const PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
);

/**
* @internal prvCpssDxChLinkStatusChangedWAs function
* @endinternal
*
* @brief   Wrapper function for handling WAs related to link status change event.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is currently implementing nothing and is defined for future use.
*
*/
GT_U32 prvCpssDxChLinkStatusChangedWAs
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal prvCpssDxChMacDmaModeRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting XLG DMA mode register address
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portMacNum               - port mac number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChMacDmaModeRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portMacNum,
    OUT GT_U32                  *regAddrPtr
);

/**
* @internal prvCpssDxChMPCS40GRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting the MPCS 40G common status register address.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChMPCS40GRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
);

/**
* @internal prvCpssDxChPortManagerDbGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting PRV_CPSS_PORT_MNG_DB_STC structure pointer from
*         CPSS dxch pp structure.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2;
*
* @param[in] devNum                   - device number
*
* @param[out] portManagerDbPtr         - (pointer to) pointer to port manager DB.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortManagerDbGetWrapper
(
    IN  GT_SW_DEV_NUM            devNum,
    OUT PRV_CPSS_PORT_MNG_DB_STC **portManagerDbPtr
);

/**
* @internal prvCpssDxChCgConvertersRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCgConvertersRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
);

/**
* @internal prvCpssDxChCgConvertersStatus2RegAddrGetWrapper function
*
* @endinternal
*
* @brief   Wrapper function for CPSS macro
*         PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC in
*         order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCgConvertersStatus2RegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
);

/**
* @internal prvCpssDxChPortCheckAndGetMacNumberWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMacNumPtr            - (pointer to) mac number for the given port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortCheckAndGetMacNumberWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *portMacNumPtr
);

/**
* @internal prvCpssDxChPortLinkStatusGetWrapper function
* @endinternal
*
* @brief   Wrapper function for cpssDxChPortLinkStatusGet
*         in order to use GT_SW_DEV_NUM.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortLinkStatusGetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isLinkUpPtr
);

/**
* @internal prvCpssDxChPortFaultSendSetWrapper function
* @endinternal
*
* @brief   Configure the port to start or stop sending fault signals to partner.
*         When port is configured to send, link on both sides will be down.
*
* @note   APPLICABLE DEVICES:      Caelum; Bobcat3; Aldrin2; Falcon; Aldrin;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - or stop sending
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChPortFaultSendSetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    IN  CPSS_PORT_SPEED_ENT           speed,
    IN  GT_BOOL                       send
);

/**
* @internal prvCpssDxChPortForceLinkDownEnableSetWrapper function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnableSetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              state
);

/**
* @internal prvCpssDxChPortEnableSetWrapper function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEnableSetWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal prvCpssDxChPortSpeedGetWrapper function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvCpssDxChPortSpeedGetWrapper
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);

/**
* @internal prvCpssDxChPortSerdesGetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortSerdesGetWrapper
(
    IN  GT_SW_DEV_NUM         devNum
);

/**
* @internal prvCpssDxChPortInterfaceModeGetWrapper function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS prvCpssDxChPortInterfaceModeGetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/**
* @internal prvCpssDxChPortHwResetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortHwResetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    GT_BOOL                            qsgmiiFullDelete
);

/**
* @internal prvCpssDxChPortHwUnresetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortHwUnresetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
);

/**
* @internal prvCpssDxChPortModeSpeedSetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortModeSpeedSetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssDxChPortSerdesSignalDetectGetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortSerdesSignalDetectGetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT GT_BOOL                        *signalStatePtr

);

/**
* @internal prvCpssDxChPortIfModeSerdesNumGetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortIfModeSerdesNumGetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
);

/**
* @internal prvCpssDxChPortInternalLoopbackEnableSetWrapper function
* @endinternal
*
* @brief   Wrapper function for setting the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For port 25 in DX269 (XG/HX port) it's the application responsibility to
*       reconfigure the loopback mode after switching XG/HX mode.
*
*/
GT_STATUS prvCpssDxChPortInternalLoopbackEnableSetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

GT_STATUS prvCpssDxChPortIfFunctionsObjInit
(
    IN GT_U8 devNum,
    INOUT PRV_CPSS_DXCH_PORT_OBJ_STC *objPtr,
    IN    CPSS_PP_FAMILY_TYPE_ENT     devFamily
);

GT_STATUS prvCpssDxChPortRefClockUpdate
(
    IN  GT_U8                           devNum,
    MV_HWS_PORT_STANDARD                portMode,
    IN  GT_U16                           *serdesArrPtr,
    IN  GT_U8                           serdesSize,
    IN  MV_HWS_REF_CLOCK_SOURCE         refClockSource,
    OUT MV_HWS_REF_CLOCK_SUP_VAL        *refClockPtr
);

/**
* @internal cpssDxChPortEnableWaWithLinkStatusSet function
* @endinternal
*
* @brief   Enable/Disable unidirectional port according to found link status.
*          For bidirectional ports done nothing
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEnableWaWithLinkStatusSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  port
);

/**
* @internal prvCpssDxChPortXlgUnidirectionalEnableSet function
* @endinternal
*
* @brief   Enable/Disable the XLG port for unidirectional transmit.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
* @param[in] enable                   - GT_TRUE:   Enable the port for unidirectional transmit.
*                                       GT_FALSE:  Disable the port for unidirectional transmit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortXlgUnidirectionalEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    port,
    IN GT_BOOL  enable
);

/**
* @internal prvCpssDxChPortXlgBufferStuckWaEnableSet function
* @endinternal
*
* @brief   Enable/Disable the XLG port Link status WA using unidirectional transmit.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
* @param[in] enable                   - GT_TRUE:   Enable the port for unidirectional transmit.
*                                       GT_FALSE:  Disable the port for unidirectional transmit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortXlgBufferStuckWaEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    port,
    IN GT_BOOL  enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __prvCpssDxChPortCtrlh */


