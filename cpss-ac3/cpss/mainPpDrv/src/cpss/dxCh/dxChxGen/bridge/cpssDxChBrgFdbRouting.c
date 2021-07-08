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
* @file cpssDxChBrgFdbRouting.c
*
* @brief FDB Host Routing CPSS DxCh implementation.
*
*
* @version   3
********************************************************************************
*/
/* get the device info and common info */
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>

/* #define CPSS_DXCH_FDB_DUMP */

#ifdef CPSS_DXCH_FDB_DUMP
    #define debugPrint(_x)      cpssOsPrintf _x
#else
    #define debugPrint(_x)
#endif

/**
* @internal internal_cpssDxChBrgFdbRoutingPortIpUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6 or both
* @param[in] enable                   - GT_TRUE  -  using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingPortIpUcEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS                               rc;           /* return code         */
    GT_U32                                  data;         /* Table subfield data */
    GT_U32                                  fieldAmount;  /* Table field amount  */
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT fieldName[2]; /* field names         */
    GT_U32                                  i;            /* Loop index          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            fieldAmount  = 1;
            fieldName[0] = SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV4_ROUTING_EN_E;
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            fieldAmount  = 1;
            fieldName[0] = SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV6_ROUTING_EN_E;
            break;
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            fieldAmount  = 2;
            fieldName[0] = SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV4_ROUTING_EN_E;
            fieldName[1] = SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV6_ROUTING_EN_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data = BOOL2BIT_MAC(enable);

    for (i = 0; (i < fieldAmount); i++)
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            fieldName[i], /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbRoutingPortIpUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6 or both
* @param[in] enable                   - GT_TRUE  -  using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingPortIpUcEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingPortIpUcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, protocol, enable));

    rc = internal_cpssDxChBrgFdbRoutingPortIpUcEnableSet(devNum, portNum, protocol, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, protocol, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingPortIpUcEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingPortIpUcEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS                               rc;           /* return code         */
    GT_U32                                  data;         /* Table subfield data */
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT fieldName   ; /* field name          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            fieldName = SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV4_ROUTING_EN_E;
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            fieldName = SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV6_ROUTING_EN_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        fieldName, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbRoutingPortIpUcEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingPortIpUcEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingPortIpUcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, protocol, enablePtr));

    rc = internal_cpssDxChBrgFdbRoutingPortIpUcEnableGet(devNum, portNum, protocol, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, protocol, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgFdbRoutingUcRefreshEnableSet function
* @endinternal
*
* @brief   Enable/Disable refresh for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcRefreshEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;           /* return code    */
    GT_U32    regAddr;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 20, 1, BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcRefreshEnableSet function
* @endinternal
*
* @brief   Enable/Disable refresh for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcRefreshEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcRefreshEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgFdbRoutingUcRefreshEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcRefreshEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable refresh status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcRefreshEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS   rc;      /* return value */
    GT_U32      hwData;  /* value from hw */
    GT_U32      regAddr; /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 20, 1, &hwData);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcRefreshEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable refresh status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcRefreshEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcRefreshEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgFdbRoutingUcRefreshEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable aging for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcAgingEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;           /* return code    */
    GT_U32    regAddr;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 21, 1, BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable aging for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcAgingEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcAgingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgFdbRoutingUcAgingEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcAgingEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable aging status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcAgingEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS   rc;      /* return value */
    GT_U32      hwData;  /* value from hw */
    GT_U32      regAddr; /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 21, 1, &hwData);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcAgingEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable aging status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcAgingEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcAgingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgFdbRoutingUcAgingEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcTransplantEnableSet function
* @endinternal
*
* @brief   Enable/Disable transplanting for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcTransplantEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;           /* return code    */
    GT_U32    regAddr;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 22, 1, BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcTransplantEnableSet function
* @endinternal
*
* @brief   Enable/Disable transplanting for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcTransplantEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcTransplantEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgFdbRoutingUcTransplantEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcTransplantEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable transplanting status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcTransplantEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS   rc;      /* return value */
    GT_U32      hwData;  /* value from hw */
    GT_U32      regAddr; /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 22, 1, &hwData);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcTransplantEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable transplanting status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcTransplantEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcTransplantEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgFdbRoutingUcTransplantEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcDeleteEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcDeleteEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;           /* return code    */
    GT_U32    regAddr;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 23, 1, BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcDeleteEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcDeleteEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcDeleteEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgFdbRoutingUcDeleteEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcDeleteEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable delete status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcDeleteEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS   rc;      /* return value */
    GT_U32      hwData;  /* value from hw */
    GT_U32      regAddr; /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 23, 1, &hwData);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcDeleteEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable delete status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcDeleteEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcDeleteEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgFdbRoutingUcDeleteEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcAAandTAToCpuSet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting
*         mechanism. When an entry is aged out or is transplanted, a message can
*         be forwarded to the CPU.
*         This field enables/disables forwarding address aged out messages and
*         transplanted address messages to the Host CPU for UC route FDB entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcAAandTAToCpuSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;           /* return code    */
    GT_U32    regAddr;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 20, 1, BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcAAandTAToCpuSet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting
*         mechanism. When an entry is aged out or is transplanted, a message can
*         be forwarded to the CPU.
*         This field enables/disables forwarding address aged out messages and
*         transplanted address messages to the Host CPU for UC route FDB entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcAAandTAToCpuSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcAAandTAToCpuSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingUcAAandTAToCpuGet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting mechanism
*         When an entry is aged out or is transplanted, a message can be forwarded
*         to the CPU.
*         This field get the enables/disable status of forwarding address aged out
*         messages and transplanted address messages to the Host CPU for UC route FDB entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingUcAAandTAToCpuGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS   rc;      /* return value */
    GT_U32      hwData;  /* value from hw */
    GT_U32      regAddr; /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 20, 1, &hwData);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingUcAAandTAToCpuGet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting mechanism
*         When an entry is aged out or is transplanted, a message can be forwarded
*         to the CPU.
*         This field get the enables/disable status of forwarding address aged out
*         messages and transplanted address messages to the Host CPU for UC route FDB entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingUcAAandTAToCpuGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingUcAAandTAToCpuGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingNextHopPacketCmdSet function
* @endinternal
*
* @brief   Set Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] nhPacketCmd              - Route entry command. supported commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E ,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingNextHopPacketCmdSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PACKET_CMD_ENT         nhPacketCmd
)
{
    GT_STATUS rc;           /* return code    */
    GT_U32    regAddr;      /* value to write */
    GT_U32    nhPacketCmdVal;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.routerGlobalCtrl.routerGlobalCtrl1;

    switch(nhPacketCmd)
    {
        case CPSS_PACKET_CMD_ROUTE_E:
            nhPacketCmdVal=0;
            break;
        case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
            nhPacketCmdVal=1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            nhPacketCmdVal=2;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            nhPacketCmdVal=3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            nhPacketCmdVal=4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 28, 3, nhPacketCmdVal);

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingNextHopPacketCmdSet function
* @endinternal
*
* @brief   Set Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] nhPacketCmd              - Route entry command. supported commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E ,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingNextHopPacketCmdSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PACKET_CMD_ENT         nhPacketCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingNextHopPacketCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nhPacketCmd));

    rc = internal_cpssDxChBrgFdbRoutingNextHopPacketCmdSet(devNum, nhPacketCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nhPacketCmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbRoutingNextHopPacketCmdGet function
* @endinternal
*
* @brief   Get Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] nhPacketCmdPtr           - (pointer to) Route entry command. supported commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E ,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_VALUE             - wrong hw value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbRoutingNextHopPacketCmdGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_PACKET_CMD_ENT         *nhPacketCmdPtr
)
{
    GT_STATUS   rc;      /* return value */
    GT_U32      hwData;  /* value from hw */
    GT_U32      regAddr; /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(nhPacketCmdPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.routerGlobalCtrl.routerGlobalCtrl1;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 28, 3, &hwData);

    if(rc != GT_OK)
        return rc;

    switch(hwData)
    {
        case 0:
            *nhPacketCmdPtr=CPSS_PACKET_CMD_ROUTE_E;
            break;
        case 1:
            *nhPacketCmdPtr=CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
            break;
        case 2:
            *nhPacketCmdPtr=CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            *nhPacketCmdPtr=CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            *nhPacketCmdPtr=CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgFdbRoutingNextHopPacketCmdGet function
* @endinternal
*
* @brief   Get Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] nhPacketCmdPtr           - (pointer to) Route entry command. supported commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E ,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_VALUE             - wrong hw value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutingNextHopPacketCmdGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_PACKET_CMD_ENT         *nhPacketCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbRoutingNextHopPacketCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nhPacketCmdPtr));

    rc = internal_cpssDxChBrgFdbRoutingNextHopPacketCmdGet(devNum, nhPacketCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nhPacketCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


