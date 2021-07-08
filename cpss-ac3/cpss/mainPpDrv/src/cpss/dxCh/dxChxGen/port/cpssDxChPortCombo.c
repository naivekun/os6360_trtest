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
* @file cpssDxChPortCombo.c
*
* @brief CPSS implementation for Combo Port configuration and control facility.
*
*
* @version   1
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCombo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

/**
* @internal internal_cpssDxChPortSerdesComboModeSet function
* @endinternal
*
* @brief   Set Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*         Combo Serdes are active for the port, else regular Serdes are active for
*         the port.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, values 20..23.
* @param[in] mode                     - serdes mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, mode
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortSerdesComboModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_DXCH_PORT_SERDES_MODE_ENT  mode
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_XCAT_E | CPSS_LION_E
                                          | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_LION2_E
                                          | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if((portNum > 23) || (portNum < 20))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum);

    if(portMacType != PRV_CPSS_PORT_GE_E)
    {
        /* Combo SERDES supported for GE ports only */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    switch(mode)
    {
        case CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E:
            value = 0;
            break;
        case CPSS_DXCH_PORT_SERDES_MODE_COMBO_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portNum,portMacType,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    return prvCpssDrvHwPpPortGroupSetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
        regAddr, 15, 1, value);
}

/**
* @internal cpssDxChPortSerdesComboModeSet function
* @endinternal
*
* @brief   Set Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*         Combo Serdes are active for the port, else regular Serdes are active for
*         the port.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, values 20..23.
* @param[in] mode                     - serdes mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, mode
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortSerdesComboModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_DXCH_PORT_SERDES_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesComboModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChPortSerdesComboModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesComboModeGet function
* @endinternal
*
* @brief   Get Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*         Combo Serdes are active for the port, else regular Serdes are active for
*         the port.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, values 20..23.
*
* @param[out] modePtr                  -  pointer to Serdes mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChPortSerdesComboModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_SERDES_MODE_ENT  *modePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_STATUS   rc;      /* return status */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_XCAT_E | CPSS_LION_E
                                          | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_LION2_E
                                          | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if((portNum > 23) || (portNum < 20))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum);

    if(portMacType != PRV_CPSS_PORT_GE_E)
    {
        /* Combo SERDES supported for GE ports only */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portNum,portMacType,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
        regAddr, 15, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (value == 0) ? CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E :
                              CPSS_DXCH_PORT_SERDES_MODE_COMBO_E;

    return GT_OK;

}

/**
* @internal cpssDxChPortSerdesComboModeGet function
* @endinternal
*
* @brief   Get Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*         Combo Serdes are active for the port, else regular Serdes are active for
*         the port.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, values 20..23.
*
* @param[out] modePtr                  -  pointer to Serdes mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChPortSerdesComboModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_SERDES_MODE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesComboModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChPortSerdesComboModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortComboModeEnableSet function
* @endinternal
*
* @brief   Register/unregister port as combo in CPSS SW DB and set some specific
*         parameters. This configuration will be used by cpssDxChPortModeSpeedSet,
*         when port's physical interface will be actually configured and for
*         other system needs.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
* @param[in] enable                   -   GT_TRUE - port is combo;
*                                      GT_FALSE - port is regular.
* @param[in] paramsPtr                - (ptr to) parameters describing specific combo port options,
*                                      doesn't matter if enable == GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - paramsPtr == NULL
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortComboModeEnableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 enable,
    IN  const   CPSS_DXCH_PORT_COMBO_PARAMS_STC *paramsPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E
                                          | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    if (enable)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portNum] = *paramsPtr;
    }
    else
    {
        GT_U32                          i;  /* iterator */
        CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboPortsInfoArrayPtr; /* ptr to combo
                                ports array for shorten code and quicker access */

        comboPortsInfoArrayPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portNum];
        comboPortsInfoArrayPtr->preferredMacIdx = CPSS_DXCH_PORT_COMBO_NA_MAC_CNS;
        for(i = 0; i < CPSS_DXCH_PORT_MAX_MACS_IN_COMBO_CNS; i++)
        {
            comboPortsInfoArrayPtr->macArray[i].macNum = CPSS_DXCH_PORT_COMBO_NA_MAC_CNS;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortComboModeEnableSet function
* @endinternal
*
* @brief   Register/unregister port as combo in CPSS SW DB and set some specific
*         parameters. This configuration will be used by cpssDxChPortModeSpeedSet,
*         when port's physical interface will be actually configured and for
*         other system needs.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
* @param[in] enable                   -   GT_TRUE - port is combo;
*                                      GT_FALSE - port is regular.
* @param[in] paramsPtr                - (ptr to) parameters describing specific combo port options,
*                                      doesn't matter if enable == GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - paramsPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboModeEnableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 enable,
    IN  const   CPSS_DXCH_PORT_COMBO_PARAMS_STC *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortComboModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable, paramsPtr));

    rc = internal_cpssDxChPortComboModeEnableSet(devNum, portNum, enable, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortComboModeEnableGet function
* @endinternal
*
* @brief   Get if port registered as combo and its parameters.
*
* @note   APPLICABLE DEVICES:      Lion2;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
*
* @param[out] enablePtr                -   GT_TRUE - port is combo;
*                                      GT_FALSE - port is regular.
* @param[out] paramsPtr                - parameters describing specific combo port options
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - enablePtr or paramsPtr == NULL
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortComboModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_DXCH_PORT_COMBO_PARAMS_STC *paramsPtr
)
{
    GT_U32                  portMacMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E
                                          | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    *paramsPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portNum];
    if(CPSS_DXCH_PORT_COMBO_NA_MAC_CNS == paramsPtr->macArray[0].macNum)
    {
        *enablePtr = GT_FALSE;
    }
    else
    {
        *enablePtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortComboModeEnableGet function
* @endinternal
*
* @brief   Get if port registered as combo and its parameters.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
*
* @param[out] enablePtr                -   GT_TRUE - port is combo;
*                                      GT_FALSE - port is regular.
* @param[out] paramsPtr                - parameters describing specific combo port options
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - enablePtr or paramsPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_DXCH_PORT_COMBO_PARAMS_STC *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortComboModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr, paramsPtr));

    rc = internal_cpssDxChPortComboModeEnableGet(devNum, portNum, enablePtr, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortComboPortActiveMacSet function
* @endinternal
*
* @brief   Activate one of MAC's serving combo port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
* @param[in] macPtr                   - (ptr to) parameters describing one of MAC of given combo port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - macPtr == NULL
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortComboPortActiveMacSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  const   CPSS_DXCH_PORT_MAC_PARAMS_STC  *macPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      localPort;  /* number of port in GOP */
    GT_U32      portGroupId;/* iterator of port groups */
    GT_U32      regAddr;    /* address of register */
    GT_U32      fieldData;  /* data to write to register field */
    PRV_CPSS_DXCH_PORT_STATE_STC      portStateStc;  /* port state */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E
                                          | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
    if((localPort != 9) && (localPort != 11))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    fieldData = BOOL2BIT_MAC(localPort != macPtr->macNum);

    rc = prvCpssDxChPortStateDisableAndGet(devNum, portNum, &portStateStc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* use extended or regular MAC for RX DMA */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.rxdmaIfConfig[localPort/8];
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                            (localPort%8)*4+3, 1, fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* use extended or regular MAC for TX DMA */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaExtendedPortsConfig;
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                         ((9 == localPort) ? 0 : 1), 1, fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set TxQ to receive from local or extended MAC */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[8].macroControl;
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                         ((9 == localPort) ? 5 : 7), 1, fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
    rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, portNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum));
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
}

/**
* @internal cpssDxChPortComboPortActiveMacSet function
* @endinternal
*
* @brief   Activate one of MAC's serving combo port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
* @param[in] macPtr                   - (ptr to) parameters describing one of MAC of given combo port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - macPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboPortActiveMacSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  const   CPSS_DXCH_PORT_MAC_PARAMS_STC  *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortComboPortActiveMacSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macPtr));

    rc = internal_cpssDxChPortComboPortActiveMacSet(devNum, portNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortComboPortActiveMacGet function
* @endinternal
*
* @brief   Get current activate MAC of combo port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
*
* @param[out] macPtr                   - (ptr to) parameters describing active MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - macPtr == NULL
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortComboPortActiveMacGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_MAC_PARAMS_STC   *macPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      fieldData;  /* value of MUX field */
    GT_U32      localPort;  /* number of port in local core */
    GT_U32      portGroupId;/* iterator of port groups */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacMap; /* port Mac Map */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E
                                          | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaExtendedPortsConfig;
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                         ((9 == localPort) ? 0 : 1), 1, &fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    macPtr->macNum = localPort + fieldData*3; /* e.g. 9 + 3 * (extendedUsed==1) = 12 */
    macPtr->macPortGroupNum = portGroupId;

    return GT_OK;
}

/**
* @internal cpssDxChPortComboPortActiveMacGet function
* @endinternal
*
* @brief   Get current activate MAC of combo port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (not CPU port).
*
* @param[out] macPtr                   - (ptr to) parameters describing active MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - macPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboPortActiveMacGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_MAC_PARAMS_STC   *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortComboPortActiveMacGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macPtr));

    rc = internal_cpssDxChPortComboPortActiveMacGet(devNum, portNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* ----------------- debug ---------------------- */

GT_STATUS dbgComboModeEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  macNum0,
    IN  GT_U32                  macPortGroupNum0,
    IN  GT_U32                  macNum1,
    IN  GT_U32                  macPortGroupNum1,
    IN  GT_U32                  preferredMacIdx
)
{
    CPSS_DXCH_PORT_COMBO_PARAMS_STC params;

    params.macArray[0].macNum = macNum0;
    params.macArray[0].macPortGroupNum = macPortGroupNum0;
    params.macArray[1].macNum = macNum1;
    params.macArray[1].macPortGroupNum = macPortGroupNum1;
    params.preferredMacIdx = preferredMacIdx;

    return cpssDxChPortComboModeEnableSet(devNum, portNum, enable,
                                          &params);
}

GT_STATUS dbgComboModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;
    GT_BOOL     enable;
    CPSS_DXCH_PORT_COMBO_PARAMS_STC params;

    rc = cpssDxChPortComboModeEnableGet(devNum, portNum, &enable, &params);
    if (GT_OK == rc)
    {
        cpssOsPrintf("enable=%d,macNum0=%d,macPortGroupNum0=%d,macNum1=%d,"
                     "macPortGroupNum1=%d,preferredMacIdx=%d\n",
                     enable,
                     params.macArray[0].macNum,
                     params.macArray[0].macPortGroupNum,
                     params.macArray[1].macNum,
                     params.macArray[1].macPortGroupNum,
                     params.preferredMacIdx);
    }

    return rc;
}

GT_STATUS dbgComboActiveMacSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  macNum,
    IN  GT_U32                  macPortGroupNum
)
{
    CPSS_DXCH_PORT_MAC_PARAMS_STC  mac;

    mac.macNum = macNum;
    mac.macPortGroupNum = macPortGroupNum;

    return cpssDxChPortComboPortActiveMacSet(devNum, portNum, &mac);
}

GT_STATUS dbgComboActiveMacGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PORT_MAC_PARAMS_STC  mac;

    rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &mac);
    if (GT_OK == rc)
    {
        cpssOsPrintf("macNum=%d,macPortGroupNum=%d\n",
                     mac.macNum, mac.macPortGroupNum);
    }

    return rc;
}




