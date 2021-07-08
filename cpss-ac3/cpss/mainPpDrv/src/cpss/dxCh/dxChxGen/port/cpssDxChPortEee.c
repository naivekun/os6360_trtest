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
* @file cpssDxChPortEee.c
*
* @brief CPSS DxCh Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
*
* @note 1. The device support relevant when the PHY works in EEE slave mode.
* When the PHY works in EEE master mode, the device is not involved in
* EEE processing.
* 2. The feature not relevant when port is not 'tri-speed'
* 3. LPI is short for 'Low Power Idle'
*
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortEee.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @enum EEE_FIELD_ENT
 *
 * @brief Enumeration of EEE fields in registers.
*/
typedef enum{

    /** field in control 0 */
    Li_limit,

    /** field in control 0 */
    Ts_limit,

    /** field in control 1 */
    LPI_request_enable,

    /** field in control 1 */
    LPI_request_force,

    /** field in control 1 */
    LPI_manual_mode,

    /** field in control 1 */
    Tw_limit,

    /** all bits of LPI_Status_Register */
    LPI_Status_Register_full,

    EEE_FIELD__MUST_BE_LAST_E

} EEE_FIELD_ENT;

/**
* @enum EEE_REG_ENT
 *
 * @brief Enumeration of EEE registers.
*/
typedef enum{

    /** register control 0 */
    LPI_control_0_Register,

    /** register control 1 */
    LPI_control_1_Register,

    /** register LPI_Status_Register */
    LPI_Status_Register

} EEE_REG_ENT;

/*
 * eee_port_fields_info
 *
 * Description: DB of positions of fields of EEE
 *
 * Fields:
 *  regId    - the register-Id
 *  startBit - start bit of the field
 *  numOfBits- num of bits of the field
 *
*/
static struct{
    EEE_REG_ENT  regId;
    GT_U32  startBit;
    GT_U32  numOfBits;
}eee_port_fields_info[EEE_FIELD__MUST_BE_LAST_E] =
{
     {LPI_control_0_Register,0,8}/*Li_limit,             control 0 */
    ,{LPI_control_0_Register,8,8}/*Ts_limit,             control 0 */

    ,{LPI_control_1_Register,0,1}/*LPI_request_enable,   control 1 */
    ,{LPI_control_1_Register,1,1}/*LPI_request_force,    control 1 */
    ,{LPI_control_1_Register,2,1}/*LPI_manual_mode,      control 1 */
    ,{LPI_control_1_Register,4,12}/*Tw_limit,            control 1 */

    ,{LPI_Status_Register,0,32}/*LPI_Status_Register_full,  full register */

};

/**
* @internal eee_fieldInfoGet function
* @endinternal
*
* @brief   EEE : get field info
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portMacNum               - the MAC number !!!
* @param[in] fieldId                  - the field info to get
*
* @param[out] regAddrPtr               - (pointer to) the register address to access
* @param[out] startBitPtr              - (pointer to) startBit of the field to access
* @param[out] numBitsPtr               - (pointer to) number of bit of the field to access
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fieldId , regId
*/
static GT_STATUS   eee_fieldInfoGet(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portMacNum,
    IN EEE_FIELD_ENT fieldId,
    OUT GT_U32  *regAddrPtr,
    OUT GT_U32  *startBitPtr,
    OUT GT_U32  *numBitsPtr
)
{
    GT_U32  regAddr;
    struct dxch_perPortRegs *portRegsPtr;

    if(fieldId >= EEE_FIELD__MUST_BE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portRegsPtr = &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum];

    switch(eee_port_fields_info[fieldId].regId)
    {
        case LPI_control_0_Register:
            regAddr = portRegsPtr->eeeLpiControl[0];
            break;
        case LPI_control_1_Register:
            regAddr = portRegsPtr->eeeLpiControl[1];
            break;
        case LPI_Status_Register:
            regAddr = portRegsPtr->eeeLpiStatus;
            break;
        default :
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regAddr;
    *startBitPtr = eee_port_fields_info[fieldId].startBit;
    *numBitsPtr  = eee_port_fields_info[fieldId].numOfBits;

    return GT_OK;
}


/**
* @internal eee_setPerPort function
* @endinternal
*
* @brief   EEE : set 'per port' field
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have MAC)
* @param[in] fieldId                  - the field to set
* @param[in] value                    -  to set to the field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_OUT_OF_RANGE          - on value out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   eee_setPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN EEE_FIELD_ENT fieldId,
    IN GT_U32       value
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;       /* register address */
    GT_U32 portMacNum; /* Mac number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32 startBit,numBits;

    /* check portNum and get converted MAC port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'global mac number' to portGroupId,local port -- supporting multi port group device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = eee_fieldInfoGet(devNum,portMacNum,fieldId,&regAddr,&startBit,&numBits);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(value >= (GT_U32)(1 << numBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                               regAddr, startBit, numBits, value);
}

/**
* @internal eee_getPerPort function
* @endinternal
*
* @brief   EEE : get 'per port' field
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have MAC)
* @param[in] fieldId                  - the field to get
*
* @param[out] valuePtr                 - (pointer to) the value read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_OUT_OF_RANGE          - on value out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   eee_getPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN EEE_FIELD_ENT fieldId,
    OUT GT_U32       *valuePtr
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;       /* register address */
    GT_U32 portMacNum; /* Mac number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32 startBit,numBits;

    /* check portNum and get converted MAC port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'global mac number' to portGroupId,local port -- supporting multi port group device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = eee_fieldInfoGet(devNum,portMacNum,fieldId,&regAddr,&startBit,&numBits);
    if (rc != GT_OK)
    {
        return rc;
    }

    return  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
                               regAddr, startBit, numBits, valuePtr);
}

/**
* @internal internal_cpssDxChPortEeeLpiRequestEnableSet function
* @endinternal
*
* @brief   Set port enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEeeLpiRequestEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL &&
        portMacObjPtr->macDrvMacEeeConfigGetFunc &&
        portMacObjPtr->macDrvMacEeeConfigSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        CPSS_MACDRV_EEE_CONFIG_STC  eeeConfig;

        rc = portMacObjPtr->macDrvMacEeeConfigGetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        eeeConfig.enable = enable;
        rc = portMacObjPtr->macDrvMacEeeConfigSetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    return eee_setPerPort(devNum,portNum,LPI_request_enable,BOOL2BIT_MAC(enable));
}
/**
* @internal cpssDxChPortEeeLpiRequestEnableSet function
* @endinternal
*
* @brief   Set port enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiRequestEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEeeLpiRequestEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortEeeLpiRequestEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEeeLpiRequestEnableGet function
* @endinternal
*
* @brief   Get port's enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEeeLpiRequestEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL &&
        portMacObjPtr->macDrvMacEeeConfigGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        CPSS_MACDRV_EEE_CONFIG_STC  eeeConfig;

        rc = portMacObjPtr->macDrvMacEeeConfigGetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        *enablePtr = eeeConfig.enable;
        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    rc  = eee_getPerPort(devNum,portNum,LPI_request_enable,&value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}
/**
* @internal cpssDxChPortEeeLpiRequestEnableGet function
* @endinternal
*
* @brief   Get port's enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiRequestEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEeeLpiRequestEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortEeeLpiRequestEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEeeLpiManualModeSet function
* @endinternal
*
* @brief   Set port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - the manual mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEeeLpiManualModeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  mode
)
{
    GT_STATUS   rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */
    GT_U32      value1,value2;/* values to set to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL &&
        portMacObjPtr->macDrvMacEeeConfigGetFunc &&
        portMacObjPtr->macDrvMacEeeConfigSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        CPSS_MACDRV_EEE_CONFIG_STC  eeeConfig;

        rc = portMacObjPtr->macDrvMacEeeConfigGetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        eeeConfig.mode = mode;
        rc = portMacObjPtr->macDrvMacEeeConfigSetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    switch(mode)
    {
        case CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E:
            value1 = 0;
            value2 = 0xFFFFFFFF;/*skip the register setting*/
            break;
        case CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENABLED_E:
            value1 = 1;
            value2 = 0;
            break;
        case CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_FORCED_E:
            value1 = 1;
            value2 = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    if (value2 != 0xFFFFFFFF)
    {
        /* start with field 'force' to make operation 'atomic' */
        rc = eee_setPerPort(devNum,portNum,LPI_request_force,value2);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return eee_setPerPort(devNum,portNum,LPI_manual_mode,value1);
}
/**
* @internal cpssDxChPortEeeLpiManualModeSet function
* @endinternal
*
* @brief   Set port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - the manual mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiManualModeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEeeLpiManualModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChPortEeeLpiManualModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEeeLpiManualModeGet function
* @endinternal
*
* @brief   Get port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - (pointer to) the manual mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEeeLpiManualModeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  *modePtr
)
{
    GT_STATUS   rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */
    GT_U32      value; /* value to read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL &&
        portMacObjPtr->macDrvMacEeeConfigGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        CPSS_MACDRV_EEE_CONFIG_STC  eeeConfig;

        rc = portMacObjPtr->macDrvMacEeeConfigGetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        *modePtr = eeeConfig.mode;
        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    rc = eee_getPerPort(devNum,portNum,LPI_manual_mode,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (value == 0)
    {
        *modePtr = CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E;
        return GT_OK;
    }

    rc = eee_getPerPort(devNum,portNum,LPI_request_force,&value);

    *modePtr = (value == 0) ?
        CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENABLED_E :
        CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_FORCED_E;

    return rc;
}
/**
* @internal cpssDxChPortEeeLpiManualModeGet function
* @endinternal
*
* @brief   Get port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - (pointer to) the manual mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiManualModeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEeeLpiManualModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChPortEeeLpiManualModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEeeLpiTimeLimitsSet function
* @endinternal
*
* @brief   Set port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function set value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] twLimit                  - The minimum time from LPI de-assertion until valid data can be sent .
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] liLimit                  - The minimum time since going into LPI and allowing LPI de-assert.
*                                      (APPLICABLE RANGES: 0..254)
* @param[in] tsLimit                  - The minimum time from emptying of Tx FIFO and LPI assert.
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_OUT_OF_RANGE          - on out of range : twLimit , liLimit , tsLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEeeLpiTimeLimitsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  twLimit,
    IN  GT_U32                  liLimit,
    IN  GT_U32                  tsLimit
)
{
    GT_STATUS   rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL &&
        portMacObjPtr->macDrvMacEeeConfigGetFunc &&
        portMacObjPtr->macDrvMacEeeConfigSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        CPSS_MACDRV_EEE_CONFIG_STC  eeeConfig;

        rc = portMacObjPtr->macDrvMacEeeConfigGetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        eeeConfig.twLimit = twLimit;
        eeeConfig.liLimit = liLimit;
        eeeConfig.tsLimit = tsLimit;
        rc = portMacObjPtr->macDrvMacEeeConfigSetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* all other fields are checked inside 'eee_setPerPort' */
    /* but this field not supports the full 8 bits */
    if (liLimit >= 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = eee_setPerPort(devNum,portNum,Tw_limit,twLimit);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = eee_setPerPort(devNum,portNum,Li_limit,liLimit);
    if(rc != GT_OK)
    {
        return rc;
    }
    return eee_setPerPort(devNum,portNum,Ts_limit,tsLimit);
}
/**
* @internal cpssDxChPortEeeLpiTimeLimitsSet function
* @endinternal
*
* @brief   Set port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function set value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] twLimit                  - The minimum time from LPI de-assertion until valid data can be sent .
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] liLimit                  - The minimum time since going into LPI and allowing LPI de-assert.
*                                      (APPLICABLE RANGES: 0..254)
* @param[in] tsLimit                  - The minimum time from emptying of Tx FIFO and LPI assert.
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_OUT_OF_RANGE          - on out of range : twLimit , liLimit , tsLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiTimeLimitsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  twLimit,
    IN  GT_U32                  liLimit,
    IN  GT_U32                  tsLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEeeLpiTimeLimitsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, twLimit, liLimit, tsLimit));

    rc = internal_cpssDxChPortEeeLpiTimeLimitsSet(devNum, portNum, twLimit, liLimit, tsLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, twLimit, liLimit, tsLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEeeLpiTimeLimitsGet function
* @endinternal
*
* @brief   Get port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function get value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] twLimitPtr               - (pointer to) The minimum time from LPI de-assertion until valid data can be sent .
* @param[out] liLimitPtr               - (pointer to) The minimum time since going into LPI and allowing LPI de-assert.
* @param[out] tsLimitPtr               - (pointer to) The minimum time from emptying of Tx FIFO and LPI assert.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEeeLpiTimeLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *twLimitPtr,
    OUT GT_U32                  *liLimitPtr,
    OUT GT_U32                  *tsLimitPtr
)
{
    GT_STATUS   rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(twLimitPtr);
    CPSS_NULL_PTR_CHECK_MAC(liLimitPtr);
    CPSS_NULL_PTR_CHECK_MAC(tsLimitPtr);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL &&
        portMacObjPtr->macDrvMacEeeConfigGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;
        CPSS_MACDRV_EEE_CONFIG_STC  eeeConfig;

        rc = portMacObjPtr->macDrvMacEeeConfigGetFunc(devNum,portNum,
                                  &eeeConfig,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        *twLimitPtr = eeeConfig.twLimit;
        *liLimitPtr = eeeConfig.liLimit;
        *tsLimitPtr = eeeConfig.tsLimit;
        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    rc = eee_getPerPort(devNum,portNum,Tw_limit,twLimitPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = eee_getPerPort(devNum,portNum,Li_limit,liLimitPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return eee_getPerPort(devNum,portNum,Ts_limit,tsLimitPtr);
}

/**
* @internal cpssDxChPortEeeLpiTimeLimitsGet function
* @endinternal
*
* @brief   Get port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function get value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] twLimitPtr               - (pointer to) The minimum time from LPI de-assertion until valid data can be sent .
* @param[out] liLimitPtr               - (pointer to) The minimum time since going into LPI and allowing LPI de-assert.
* @param[out] tsLimitPtr               - (pointer to) The minimum time from emptying of Tx FIFO and LPI assert.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiTimeLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *twLimitPtr,
    OUT GT_U32                  *liLimitPtr,
    OUT GT_U32                  *tsLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEeeLpiTimeLimitsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, twLimitPtr, liLimitPtr, tsLimitPtr));

    rc = internal_cpssDxChPortEeeLpiTimeLimitsGet(devNum, portNum, twLimitPtr, liLimitPtr, tsLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, twLimitPtr, liLimitPtr, tsLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




/**
* @internal internal_cpssDxChPortEeeLpiStatusGet function
* @endinternal
*
* @brief   Get port LPI status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statusPtr                - (pointer to) the status info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortEeeLpiStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_STATUS_STC *statusPtr
)
{
    GT_STATUS   rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */
    GT_U32      value; /* value to read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL &&
        portMacObjPtr->macDrvMacEeeStatusGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacEeeStatusGetFunc(devNum,portNum,
                                  &statusPtr->macRxPathLpi,
                                  &statusPtr->macTxPathLpi,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        statusPtr->macTxPathLpWait  = statusPtr->macTxPathLpi;
        statusPtr->pcsTxPathLpi     = statusPtr->macTxPathLpi;
        statusPtr->pcsRxPathLpi     = statusPtr->macRxPathLpi;
        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    rc = eee_getPerPort(devNum,portNum,LPI_Status_Register_full,&value);

    statusPtr->macTxPathLpi     = BIT2BOOL_MAC(((value >> 4) & 1));
    statusPtr->macTxPathLpWait  = BIT2BOOL_MAC(((value >> 3) & 1));
    statusPtr->macRxPathLpi     = BIT2BOOL_MAC(((value >> 2) & 1));
    statusPtr->pcsTxPathLpi     = BIT2BOOL_MAC(((value >> 1) & 1));
    statusPtr->pcsRxPathLpi     = BIT2BOOL_MAC(((value >> 0) & 1));

    return rc;
}

/**
* @internal cpssDxChPortEeeLpiStatusGet function
* @endinternal
*
* @brief   Get port LPI status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statusPtr                - (pointer to) the status info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_STATUS_STC *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEeeLpiStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statusPtr));

    rc = internal_cpssDxChPortEeeLpiStatusGet(devNum, portNum, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

