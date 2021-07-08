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
* @file cpssDxChPortManager.h
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortManager
#define __cpssDxChPortManager

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/private/prvCpssPortManager.h>

/**
* @internal cpssDxChPortManagerEventSet function
* @endinternal
*
* @brief   Set the port according to the given event. This API performs actions based on
*         the given event and will transfer the port to a new state. For example, when port
*         is in PORT_MANAGER_STATE_RESET_E state and event PORT_MANAGER_EVENT_CREATE_E is called,
*         this API will perform port creation operations and upon success, port state will be
*         changed to PORT_MANAGER_STATE_LINK_DOWN_E state.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portEventStc             - (pointer to) structure containing the event data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerEventSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_STC   *portEventStc
);

/**
* @internal cpssDxChPortManagerStatusGet function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
);

/**
* @internal cpssDxChPortManagerPortParamsSet function
* @endinternal
*
* @brief   Set the given attributes to a port. Those configurations will take place
*         during the port management. This API is applicable only when port is in
*         reset (PORT_MANAGER_STATE_RESET_E state).
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerPortParamsSet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    IN  CPSS_PM_PORT_PARAMS_STC               *portParamsStcPtr
);

/**
* @internal cpssDxChPortManagerPortParamsGet function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssDxChPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, On devices below SIP_5_15, if RX parameters not set and
*       port is in reset, the RX values are not availble to be fetched, and
*       filled with zeros.
*
*/
GT_STATUS cpssDxChPortManagerPortParamsGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT CPSS_PM_PORT_PARAMS_STC               *portParamsStcPtr
);

/**
* @internal cpssDxChPortManagerPortParamsStructInit function
* @endinternal
*
* @brief  This API resets the Port Manager Parameter structure
*         so once the application update the structure with
*         relevant data - all other feilds are clean for sure.
*         Application must call this API before calling
*         cpssDxChPortManagerPortParamsSet.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in,out] portParamsStcPtr   - (pointer to) a structure
*       contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChPortManagerPortParamsStructInit
(
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    INOUT  CPSS_PM_PORT_PARAMS_STC *portParamsStcPtr
);

/**
* prvCpssDxChPortManagerParametersSetExt function
*
* @brief   extended parmaeter set port manager command
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                    - interface mode
* @param[in] speed                    - speed
* @param[in] adaptiveMode             - wheater to run adaptive
*       or not
* @param[in] trainMode                - training mode
* @param[in] overrideElectricalParams - override TX parameters
* @param[in] overridePolarityParams   - override polarity
*       parameters
* @param[in] -                        minLF
* @param[in] -                        maxLF
* @param[in] fecMode                  = fecMode
*
* @param[out] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssDxChPortManagerParametersSetExt
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         adaptiveMode,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT  trainMode,
    IN  GT_BOOL                              overrideElectricalParams,
    IN  GT_BOOL                              overridePolarityParams,
    IN  GT_U8                            minLF,
    IN  GT_U8                            maxLF,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
);


/**
* prvCpssDxChPortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerUnMaskModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
);

/**
* @internal cpssDxChPortManagerGlobalParamsOverride function
* @endinternal
*
* @brief  To use this function for overide, need to give it a
*         struct that include parameters to override, and to
*         enable override flag to 1, else it's will not override
*         and use defualts values.
*         Don't forget to fill the globalParamstType enum with
*         the match enum of what you want to override.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] globalParamsStc          - stc with the parameters
*                                       to override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To add other globals parameter to the struct, make sure
*       to add enum and enable flag for the set of values.
*       *See CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC as a
*        good format to add other parameters.
*
*/
GT_STATUS cpssDxChPortManagerGlobalParamsOverride
(
    IN  GT_U8                                  devNum,
    IN  CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC    *globalParamsStcPtr
);

/**
* @internal cpssDxChSamplePortManagerMandatoryParamsSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - port mode
* @param[in] speed                    - port speed
* @param[in] speed                    - port fec mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
);

/**
* @internal cpssDxChSamplePortManagerLoopbackSet function
* @endinternal
*
* @brief   set the Loopback params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] loopbackType             - loopback type
* @param[in] macLoopbackEnable        - MAC loopback
*       enable/disable
* @param[in] serdesLoopbackMode       - SerDes loopback type
* @param[in] enableRegularTrafic      - enable trafic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerLoopbackSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT     loopbackType,
    IN  GT_BOOL                                 macLoopbackEnable,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT      serdesLoopbackMode,
    IN  GT_BOOL                                 enableRegularTrafic
);


/**
* @internal cpssDxChSamplePortManagerSerdesTxParamsSet function
* @endinternal
*
* @brief   set the serdes tx params in the pm database
*
* @param[in] devNum               - device number
* @param[in] portNum              - port number
* @param[in] laneNum              - lane number
* @param[in] portParamsInputSt    - a structure contains port
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerSerdesTxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_TX_CONFIG_STC  *portParamsInputStcPtr
);

/**
* @internal cpssDxChSamplePortManagerSerdesRxParamsSet function
* @endinternal
*
* @brief   set the serdes rx params in the pm database
*
* @param[in] devNum               - device number
* @param[in] portNum              - port number
* @param[in] laneNum              - lane number
* @param[in] portParamsInputSt    - a structure contains port
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerSerdesRxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_RX_CONFIG_STC  *portParamsInputStcPtr
);

/**
* @internal cpssDxChSamplePortManagerTrainModeSet function
* @endinternal
*
* @brief   set the TrainMode attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] trainMode                - trainMode type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerTrainModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT     trainModeType
);

/**
* @internal cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet function
* @endinternal
*
* @brief   set the AdaptiveRxTrainSupp attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] adaptRxTrainSupp         - boolean whether adaptive rx tune is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 adaptRxTrainSupp
);

/**
* @internal cpssDxChSamplePortManagerEdgeDetectSuppSet function
* @endinternal
*
* @brief   set the EdgeDetectSupp attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] edgeDetectSupported      - boolean whether edge detect is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerEdgeDetectSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 edgeDetectSupported
);

/**
* @internal cpssDxChSamplePortManagerFecModeSet function
* @endinternal
*
* @brief   set the FecMode attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] fecMode                  - FEC mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerFecModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_FEC_MODE_ENT                  fecMode
);

/**
* cpssDxChSamplePortManagerEnhanceTuneOverrideSet function
*
* @brief   Enhanced Tune Override Params Set fn.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] minLF                    - minimum LF
* @param[in] maxLF                    - max LF
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChSamplePortManagerEnhanceTuneOverrideSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  minLF,
    IN  GT_U32                                  maxLF
);

/**
* cpssDxChSamplePortManagerOpticalCalibrationSet function
*
* @brief   pre defined calibration type set command for port
*          manager
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] calibrationType          - Cu or Optical
* @param[in] minEoThreshold           - minimum EO threshold
* @param[in] maxEoThreshold           - max EO threshold
* @param[in] bitMapEnable             - BM enable
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChSamplePortManagerOpticalCalibrationSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT  calibrationType,
    IN  GT_U32                                  minEoThreshold,
    IN  GT_U32                                  maxEoThreshold,
    IN  GT_U32                                  bitMapEnable
);

/**
* cpssDxChSamplePortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChSamplePortManagerUnMaskModeSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
);

/**
* @internal cpssDxChSamplePortManagerApMandatoryParamsSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port mode
* @param[in] speed                 - port speed
* @param[in] fecSupported          - fec supported
* @param[in] fecRequested          - fec requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
);

/**
* @internal cpssDxChSamplePortManagerApAddMultiSpeedParamsSet
*           function
* @endinternal
*
* @brief   set the multi speed params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port mode
* @param[in] speed                 - port speed
* @param[in] fecSupported          - fec supported
* @param[in] fecRequested          - fec requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* Hint: this is for multi speed set,
* cpssDxChSamplePortManagerApMandatoryParamsSet must be called
* first
*/
GT_STATUS cpssDxChSamplePortManagerApAddMultiSpeedParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
);

/**
* @internal cpssDxChSamplePortManagerApAttributeParamsSet
*           function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] nonceDisable          - nonceDisable
* @param[in] fcPause               - fcPause
* @param[in] fcAsmDir              - fcAsmDir
* @param[in] negotiationLaneNum    - negotiation lane number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApAttributeParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             nonceDisable,
    IN  GT_BOOL                             fcPause,
    IN  CPSS_PORT_AP_FLOW_CONTROL_ENT       fcAsmDir,
    IN  GT_U32                              negotiationLaneNum
);


/**
* @internal cpssDxChSamplePortManagerApOverrideTxLaneParamsSet
*           function
* @endinternal
*
* @brief   set the tx params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] laneNum               - lane number
* @param[in] laneSpeed             - lane speed
* @param[in] portParamsInputStc    - structe with attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApOverrideTxLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  CPSS_PM_AP_LANE_OVERRIDE_PARAM_STC  *portParamsInputStcPtr
);

/**
* @internal cpssDxChSamplePortManagerApOverrideRxLaneParamsSet
*           function
* @endinternal
*
* @brief   set the rx params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] laneNum               - lane number
* @param[in] laneSpeed             - lane speed
* @param[in] portParamsInputStc    - structe with attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApOverrideRxLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  CPSS_PM_AP_LANE_OVERRIDE_PARAM_STC  *portParamsInputStcPtr
);

/**
* @internal cpssDxChSamplePortManagerApOverrideEtlLaneParamsSet
*           function
* @endinternal
*
* @brief   set the etl params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] laneNum               - lane number
* @param[in] laneSpeed             - lane speed
* @param[in] portParamsInputStc    - structe with attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApOverrideEtlLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  CPSS_PM_AP_LANE_OVERRIDE_PARAM_STC  *portParamsInputStcPtr
);

/**
* @internal cpssDxChPortManagerInit function
* @endinternal
*
* @brief  Port Manager Init system - this function is
*         responsible on all configurations that till now was
*         application responsiblity.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS cpssDxChPortManagerInit
(
    IN  GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortManager */

