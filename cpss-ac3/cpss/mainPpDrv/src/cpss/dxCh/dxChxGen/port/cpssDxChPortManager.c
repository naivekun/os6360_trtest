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
* @file cpssDxChPortManager.c
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

/**
* @internal cpssDxChPortManagerEventSet function
* @endinternal
*
* @brief   Configure the port according to the given event. This API performs actions based on
*         the given event and will possible transfer the port to a new state. For example, when
*         port is in PORT_MANAGER_STATE_RESET_E state and event PORT_MANAGER_EVENT_CREATE_E is
*         called, this API will perform port creation operations and upon success, port state
*         will be changed to PORT_MANAGER_STATE_LINK_DOWN_E state.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portEventStcPtr          - (pointer to) structure containing the event data
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
    IN  CPSS_PORT_MANAGER_STC   *portEventStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerEventSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portEventStcPtr));

    rc = internal_cpssPortManagerEventSet(devNum, portNum, portEventStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portEventStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortManagerStatusGet function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port is currently
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
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerStatusGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portStagePtr));

    rc = internal_cpssPortManagerStatusGet(devNum, portNum, portStagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portStagePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PM_PORT_PARAMS_STC                *portParamsStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerPortParamsSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portParamsStcPtr));

    rc = internal_cpssPortManagerPortParamsSet(devNum, portNum, portParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portParamsStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerPortParamsGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portParamsStcPtr));

    rc = internal_cpssPortManagerPortParamsGet(devNum, portNum, portParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portParamsStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerPortParamsStructInit);
    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, portParamsStcPtr));

    rc = internal_cpssPortManagerInitParamsStruct(portType, portParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, portParamsStcPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);


    return rc;
}

/**
* cpssDxChPortManagerParametersSetExt function
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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    CPSS_PM_PORT_PARAMS_STC portParamsFromDb;

    GT_STATUS    rc;
    GT_U32       laneNum;

    GT_BOOL      overrideElec;

    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct :rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParamsFromDb);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct :rc=%d\n", rc);
        return rc;
    }

    /* standard */
    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    /* init get strctures */
    portParamsFromDb.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParamsFromDb.portParamsType.regPort.ifMode = ifMode;
    portParamsFromDb.portParamsType.regPort.speed = speed;

    /* set Adaptive mode */
/*

  CPSS_PM_ATTR_TRAIN_MODE_E,
  CPSS_PM_ATTR_RX_TRAIN_SUPP_E,
  CPSS_PM_ATTR_EDGE_DETECT_SUPP_E,
  CPSS_PM_ATTR_LOOPBACK_E,
  CPSS_PM_ATTR_ET_OVERRIDE_E,
  CPSS_PM_ATTR_FEC_MODE_E,
  CPSS_PM_ATTR_CALIBRATION_E,
  CPSS_PM_ATTR_UNMASK_EV_MODE_E,
*/
    portParams.portParamsType.regPort.portAttributes.adaptRxTrainSupp = adaptiveMode;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_RX_TRAIN_SUPP_E);

    /* set train mode */
    portParams.portParamsType.regPort.portAttributes.trainMode = trainMode;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);

    /* ETL values */
    portParams.portParamsType.regPort.portAttributes.etOverride.minLF = minLF;
    portParams.portParamsType.regPort.portAttributes.etOverride.maxLF = maxLF;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_ET_OVERRIDE_E);

    /* insert additionals non-AP parameters to parameters structure */
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_FEC_MODE_E);

    if (overrideElectricalParams == GT_TRUE || overridePolarityParams == GT_TRUE)
    {
        rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParamsFromDb);
        if (rc!=GT_OK)
        {
            cpssOsPrintf("calling cpssDxChPortManagerPortParamsGet returned=%d, portNum=%d", rc, portNum);
            return rc;
        }

    }

    for (laneNum=0; laneNum<CPSS_PM_MAX_LANES; laneNum++)
    {
           overrideElec = CPSS_PM_LANE_PARAM_IS_VALID(&portParamsFromDb, laneNum, CPSS_PM_LANE_PARAM_TX_E);

           if (overrideElectricalParams && overrideElec)
           {
               CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_TX_E);
               portParams.portParamsType.regPort.laneParams[laneNum].txParams.emph0 =
                                                   portParamsFromDb.portParamsType.regPort.laneParams[laneNum].txParams.emph0;
               portParams.portParamsType.regPort.laneParams[laneNum].txParams.emph1 =
                                                   portParamsFromDb.portParamsType.regPort.laneParams[laneNum].txParams.emph1;
               portParams.portParamsType.regPort.laneParams[laneNum].txParams.txAmp =
                                                   portParamsFromDb.portParamsType.regPort.laneParams[laneNum].txParams.txAmp;
           }
    }


    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    if (rc!=GT_OK)
    {
        cpssOsPrintf("calling cpssDxChPortManagerParametersSetExt returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerParamsSet function
*
* @brief  parmaeters set command for port manager
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
GT_STATUS prvCpssDxChPortManagerParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT        ifMode,
    IN  CPSS_PORT_SPEED_ENT                 speed
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling PortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerStatusGet function
*
* @brief  Get the Status of a port
*
* @note   APPLICABLE DEVICES:      Aldrin; Bobcat3; Aldrin2;
*         Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum
)
{
    CPSS_PORT_MANAGER_STATUS_STC portStatus;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portStatus);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChPortManagerStatusGet (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }

    cpssOsPrintf("Status for port %d:\n", portNum);
    cpssOsPrintf("  State           %d\n", portStatus.portState);
    cpssOsPrintf("  Oper Mode       %d\n", portStatus.portUnderOperDisable);
    cpssOsPrintf("  Failure state   %d\n", portStatus.failure);
    if (portStatus.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
    {
        cpssOsPrintf("  IfMode          %d:\n", portStatus.ifMode);
        cpssOsPrintf("  Speed           %d:\n", portStatus.speed);
        cpssOsPrintf("  FEC             %d:\n", portStatus.fecType);
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerOpticalCalibrationSet function
*
* @brief   pre defined calibration type set command for port
*          manager
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
GT_STATUS prvCpssDxChPortManagerOpticalCalibrationSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT  calibrationType,
    IN  GT_U32                                  minEoThreshold,
    IN  GT_U32                                  maxEoThreshold,
    IN  GT_U32                                  bitMapEnable
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR (&portParams,CPSS_PM_ATTR_CALIBRATION_E);

    /* in order to select optinal calibration need to call the function with CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.calibrationType = calibrationType;
    /* setting confidence interval parameters */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMinThreshold = 12;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMaxThreshold = 15;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMinThreshold = 0;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMaxThreshold = 4;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMinThreshold = minEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMaxThreshold = maxEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.calibrationTimeOutSec = 10;

    /* for now the requirment is to use only EO as filtring until CI data will be collected */
    /* portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30; */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = bitMapEnable;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling PortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }


    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR (&portParams,CPSS_PM_ATTR_UNMASK_EV_MODE_E);
    portParams.portParamsType.regPort.portAttributes.unMaskEventsMode = unMaskMode;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling PortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerGlobalParamsOverride);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, globalParamsStcPtr));

    rc = internal_cpssPortManagerGlobalParamsOverride(devNum, globalParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, globalParamsStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* prvCpssDxChPortManagerGlobalParamsSet function
*
* @brief   Rx parmaeters set command for port manager
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Caelum; Aldrin; DxCh1;
*         DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2;
*         Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssDxChPortManagerGlobalParamsSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   changeInterval,
    IN  GT_U32   changeWindow,
    IN  GT_U32   checkTimeExpired,
    IN  GT_BOOL  highSpeed
)
{
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC globalParams;
    GT_STATUS rc;
    GT_BOOL stabilityConfig = GT_FALSE;

    globalParams.globalParamsBitmapType = 0;
    globalParams.signalDetectDbCfg.sdChangeInterval = 5;
    globalParams.signalDetectDbCfg.sdChangeMinWindowSize = 50;
    globalParams.signalDetectDbCfg.sdCheckTimeExpired = 500;
    globalParams.propHighSpeedPortEnabled = GT_FALSE;

    if (changeInterval != 0)
    {
        globalParams.signalDetectDbCfg.sdChangeInterval = changeInterval;
        stabilityConfig = GT_TRUE;
    }
    if (changeWindow != 0)
    {
        globalParams.signalDetectDbCfg.sdChangeMinWindowSize = changeWindow;
        stabilityConfig = GT_TRUE;
    }
    if (checkTimeExpired != 0)
    {
        globalParams.signalDetectDbCfg.sdCheckTimeExpired = checkTimeExpired;
        stabilityConfig = GT_TRUE;
    }

    globalParams.propHighSpeedPortEnabled = highSpeed;
    globalParams.globalParamsBitmapType |= CPSS_PORT_MANAGER_GLOBAL_PARAMS_HIGH_SPEED_PORTS_E;

    if (stabilityConfig)
    {
        globalParams.globalParamsBitmapType |= CPSS_PORT_MANAGER_GLOBAL_PARAMS_SIGNAL_DETECT_E;
    }


    rc = cpssDxChPortManagerGlobalParamsOverride(devNum, &globalParams);
    if (rc!=GT_OK)
    {
        cpssOsPrintf("calling cpssDxChPortManagerGlobalParamsOverride returned=%d", rc);
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerInit);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPortManagerInit(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/****************************
           Samples
 ****************************/

/*-----Regular-Samples-----*/

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;

    /* sample code that configured the port manager to ensure the EO parameters are within pre defined range,
       this to ensure stable signal, note that the threshold may vary depending on board */
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_CALIBRATION_E);
        /* in order to select optinal calibration need to call the function with CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.calibrationType = CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E;
        /* set min Eo Threshold */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMinThreshold = 130;
        /* set max Eo Threshold */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMaxThreshold = 800;
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.calibrationTimeOutSec = 10;

        /* portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30; */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30;
    }

    if (speed == CPSS_PORT_SPEED_1000_E)
    {
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);
        portParams.portParamsType.regPort.portAttributes.trainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E;
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_LOOPBACK_E);

    portParams.portParamsType.regPort.portAttributes.loopback.loopbackType = loopbackType;

    switch (loopbackType)
    {
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
            portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable = macLoopbackEnable;
            break;
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
            portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode = serdesLoopbackMode;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portParams.portParamsType.regPort.portAttributes.loopback.enableRegularTrafic = enableRegularTrafic;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC         portParams;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,portParams.portParamsType.regPort.ifMode,portParams.portParamsType.regPort.speed, &portMode);
    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (laneNum >= curPortParams.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_TX_E);

    portParams.portParamsType.regPort.laneParams[laneNum].txParams.emph0 =
            portParamsInputStcPtr->emph0;
    portParams.portParamsType.regPort.laneParams[laneNum].txParams.emph1 =
            portParamsInputStcPtr->emph1;
    portParams.portParamsType.regPort.laneParams[laneNum].txParams.txAmp =
            portParamsInputStcPtr->txAmp;
    portParams.portParamsType.regPort.laneParams[laneNum].txParams.txAmpAdjEn =
            portParamsInputStcPtr->txAmpAdjEn;
    portParams.portParamsType.regPort.laneParams[laneNum].txParams.txAmpShft =
            portParamsInputStcPtr->txAmpShft;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,portParams.portParamsType.regPort.ifMode,portParams.portParamsType.regPort.speed, &portMode);
    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (laneNum >= curPortParams.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_RX_E);

    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.sqlch =
            portParamsInputStcPtr->sqlch;
    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.ffeRes =
            portParamsInputStcPtr->ffeRes;
    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.ffeCap =
            portParamsInputStcPtr->ffeCap;
    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.align90 =
            portParamsInputStcPtr->align90;
    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.dcGain =
            portParamsInputStcPtr->dcGain;
    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.bandWidth =
            portParamsInputStcPtr->bandWidth;
    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.loopBandwidth =
            portParamsInputStcPtr->loopBandwidth;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    if (trainModeType >=  CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);
    portParams.portParamsType.regPort.portAttributes.trainMode = trainModeType;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    if ((adaptRxTrainSupp != GT_TRUE) && (adaptRxTrainSupp != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_RX_TRAIN_SUPP_E);
    portParams.portParamsType.regPort.portAttributes.adaptRxTrainSupp = adaptRxTrainSupp;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}


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
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    if ((edgeDetectSupported != GT_TRUE) && (edgeDetectSupported != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_EDGE_DETECT_SUPP_E);
    portParams.portParamsType.regPort.portAttributes.edgeDetectSupported = edgeDetectSupported;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    if (fecMode >= CPSS_PORT_FEC_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_ET_OVERRIDE_E);
    portParams.portParamsType.regPort.portAttributes.etOverride.minLF = minLF;
    portParams.portParamsType.regPort.portAttributes.etOverride.maxLF = maxLF;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    if (CPSS_PORT_MANAGER_CALIBRATION_TYPE_LAST_E <= calibrationType)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* standard */
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_CALIBRATION_E);

    /* in order to select optinal calibration need to call the function with CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.calibrationType = calibrationType;
    /* setting confidence interval parameters */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMinThreshold = 12;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMaxThreshold = 15;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMinThreshold = 0;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMaxThreshold = 4;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMinThreshold = minEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMaxThreshold = maxEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.calibrationTimeOutSec = 10;

    /* for now the requirment is to use only EO as filtring until CI data will be collected */
    /* portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30; */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = bitMapEnable;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssDxChSamplePortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] unMaskMode               - unmask mode
*
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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    if (CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_LAST_E <= unMaskMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }


    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_UNMASK_EV_MODE_E);
    portParams.portParamsType.regPort.portAttributes.unMaskEventsMode = unMaskMode;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/*-----AP-Samples-----*/

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
    rc = cpssDxChPortManagerPortParamsStructInit(portParams.portType, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    portParams.portParamsType.apPort.numOfModes++;

    /* configure the first port mode and speed */
    portParams.portParamsType.apPort.modesArr[0].ifMode = ifMode;
    portParams.portParamsType.apPort.modesArr[0].speed = speed;
    portParams.portParamsType.apPort.modesArr[0].fecRequested = fecRequested;
    portParams.portParamsType.apPort.modesArr[0].fecSupported = fecSupported;

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
    (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;
    GT_U32 numOfModes;

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    numOfModes = portParams.portParamsType.apPort.numOfModes;
    if (numOfModes == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (numOfModes >= CPSS_PM_AP_MAX_MODES)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portParams.portParamsType.apPort.modesArr[numOfModes].ifMode = ifMode;
    portParams.portParamsType.apPort.modesArr[numOfModes].speed = speed;
    portParams.portParamsType.apPort.modesArr[numOfModes].fecRequested = fecRequested;
    portParams.portParamsType.apPort.modesArr[numOfModes].fecSupported = fecSupported;

    portParams.portParamsType.apPort.numOfModes++;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_NONCE_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_NUM_E);

    portParams.portParamsType.apPort.apAttrs.nonceDisable = nonceDisable;
    portParams.portParamsType.apPort.apAttrs.fcPause = fcPause;
    portParams.portParamsType.apPort.apAttrs.fcAsmDir = fcAsmDir;
    portParams.portParamsType.apPort.apAttrs.negotiationLaneNum = negotiationLaneNum;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((CPSS_PM_MAX_LANES <= laneNum) || (CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E <= laneSpeed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);

    CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, laneNum, CPSS_PM_AP_LANE_OVERRIDE_TX_OFFSET_E);

    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txAmpOffset =
        portParamsInputStcPtr->txAmpOffset;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txEmph0Offset =
        portParamsInputStcPtr->txEmph0Offset;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txEmph1Offset =
        portParamsInputStcPtr->txEmph1Offset;


    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((CPSS_PM_MAX_LANES <= laneNum) || (CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E <= laneSpeed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);

    CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, laneNum, CPSS_PM_AP_LANE_OVERRIDE_RX_E);

    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].squelch =
        portParamsInputStcPtr->squelch;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].lowFrequency =
        portParamsInputStcPtr->lowFrequency;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].highFrequency =
        portParamsInputStcPtr->highFrequency;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].dcGain =
        portParamsInputStcPtr->dcGain;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].bandWidth =
        portParamsInputStcPtr->bandWidth ;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].loopBandwidth =
        portParamsInputStcPtr->loopBandwidth;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((CPSS_PM_MAX_LANES <= laneNum) || (CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E <= laneSpeed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);

    CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, laneNum, CPSS_PM_AP_LANE_OVERRIDE_ETL_E);

    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].etlMinDelay =
        portParamsInputStcPtr->etlMinDelay;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].etlMaxDelay =
        portParamsInputStcPtr->etlMaxDelay;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/****************************
      Samples ends here
 ****************************/

