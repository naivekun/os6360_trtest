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
* @file cpssDxChBrgGen.c
*
* @brief CPSS DxCh implementation for Bridge Generic APIs.
*
*
* @version   76
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

/**
* @internal internal_cpssDxChBrgGenIgmpSnoopEnable function
* @endinternal
*
* @brief   Enable/disable trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] status                   - GT_TRUE for enable and GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIgmpSnoopEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      status
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;     /* register address */
    GT_U32      value;       /* value to write into register */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(status);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <IGMP_TRAP_EN>*/
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_IGMP_TRAP_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum, portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <IGMPTrapEn> */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 12, 1, value);
    }

    return rc;
}

/**
* @internal cpssDxChBrgGenIgmpSnoopEnable function
* @endinternal
*
* @brief   Enable/disable trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] status                   - GT_TRUE for enable and GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      status
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIgmpSnoopEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, status));

    rc = internal_cpssDxChBrgGenIgmpSnoopEnable(devNum, portNum, status);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, status));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIgmpSnoopEnableGet function
* @endinternal
*
* @brief   Get status of trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] statusPtr                - (pointer to ) GT_TRUE for enable and GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIgmpSnoopEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *statusPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(statusPtr);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <IGMP_TRAP_EN>*/
         rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_IGMP_TRAP_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <IGMPTrapEn> */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 12, 1, &value);
    }

    if (rc != GT_OK)
        return rc;

    *statusPtr = BIT2BOOL_MAC(value);
    return rc;
}

/**
* @internal cpssDxChBrgGenIgmpSnoopEnableGet function
* @endinternal
*
* @brief   Get status of trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] statusPtr                - (pointer to ) GT_TRUE for enable and GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIgmpSnoopEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statusPtr));

    rc = internal_cpssDxChBrgGenIgmpSnoopEnableGet(devNum, portNum, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenDropIpMcEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  state
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;
        value = state ? CPSS_PACKET_CMD_DROP_HARD_E : CPSS_PACKET_CMD_FORWARD_E;

        return prvCpssHwPpSetRegField(devNum, regAddr, 6, 3, value);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[0];

    value = BOOL2BIT_MAC(state);

    /* Bridge Global Configuration Register0, field <DropIPMcEn> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 25, 1, value);
}

/**
* @internal cpssDxChBrgGenDropIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenDropIpMcEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropIpMcEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, state));

    rc = internal_cpssDxChBrgGenDropIpMcEnable(devNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] statePtr                 - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenDropIpMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *statePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 3, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        *statePtr = (value == CPSS_PACKET_CMD_FORWARD_E) ? GT_FALSE : GT_TRUE;
        return GT_OK;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[0];

    /* Bridge Global Configuration Register0, field <DropIPMcEn> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 25, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *statePtr = (value == 0) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenDropIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] statePtr                 - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenDropIpMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropIpMcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, statePtr));

    rc = internal_cpssDxChBrgGenDropIpMcEnableGet(devNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropNonIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenDropNonIpMcEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  state
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;
        value = state ? CPSS_PACKET_CMD_DROP_HARD_E : CPSS_PACKET_CMD_FORWARD_E;

        return prvCpssHwPpSetRegField(devNum, regAddr, 9, 3, value);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                bridgeRegs.bridgeGlobalConfigRegArray[0];
    value = BOOL2BIT_MAC(state);

    /* Bridge Global Configuration Register0, field <DropNonIPMcEn> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 26, 1, value);
}

/**
* @internal cpssDxChBrgGenDropNonIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropNonIpMcEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, state));

    rc = internal_cpssDxChBrgGenDropNonIpMcEnable(devNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropNonIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] statePtr                 - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenDropNonIpMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *statePtr
)
{
    GT_U32 rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 9, 3, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        *statePtr = (value == CPSS_PACKET_CMD_FORWARD_E) ? GT_FALSE : GT_TRUE;
        return GT_OK;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                bridgeRegs.bridgeGlobalConfigRegArray[0];

    /* Bridge Global Configuration Register0, field <DropNonIPMcEn> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 26, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    *statePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgGenDropNonIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] statePtr                 - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropNonIpMcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, statePtr));

    rc = internal_cpssDxChBrgGenDropNonIpMcEnableGet(devNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropInvalidSaEnable function
* @endinternal
*
* @brief   Drop all Ethernet packets with MAC SA that are Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE,  filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenDropInvalidSaEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        /* the device not support 2 actions:
            1. enable/disable drop - cpssDxChBrgGenDropInvalidSaEnable
            2. hard/sof drop - cpssDxChBrgSecurBreachEventDropModeSet(eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E)

           the devices support the frw/mirro/trap/h.drop/s.drop in single field */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig0;
        value = (enable == GT_TRUE) ? CPSS_PACKET_CMD_DROP_HARD_E : CPSS_PACKET_CMD_FORWARD_E;

        return prvCpssHwPpSetRegField(devNum, regAddr, 0, 3, value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                bridgeRegs.bridgeGlobalConfigRegArray[0];

        /* Bridge Global Configuration Register0, field <DropInvalidSA> */
        return prvCpssHwPpSetRegField(devNum, regAddr, 27, 1, BOOL2BIT_MAC(enable));
    }
}

/**
* @internal cpssDxChBrgGenDropInvalidSaEnable function
* @endinternal
*
* @brief   Drop all Ethernet packets with MAC SA that are Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE,  filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenDropInvalidSaEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropInvalidSaEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgGenDropInvalidSaEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropInvalidSaEnableGet function
* @endinternal
*
* @brief   Get status of droping all Ethernet packets with MAC SA that are
*         Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenDropInvalidSaEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        /* the device not support 2 actions:
            1. enable/disable drop - cpssDxChBrgGenDropInvalidSaEnable
            2. hard/sof drop - cpssDxChBrgSecurBreachEventDropModeSet(eventType = CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E)

           the devices support the frw/mirro/trap/h.drop/s.drop in single field */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig0;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 3, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        *enablePtr = (CPSS_PACKET_CMD_FORWARD_E == value) ? GT_FALSE : GT_TRUE;

        return GT_OK;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                bridgeRegs.bridgeGlobalConfigRegArray[0];

    /* Bridge Global Configuration Register0, field <DropInvalidSA> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 27, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 0) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenDropInvalidSaEnableGet function
* @endinternal
*
* @brief   Get status of droping all Ethernet packets with MAC SA that are
*         Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenDropInvalidSaEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropInvalidSaEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgGenDropInvalidSaEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenUcLocalSwitchingEnable function
* @endinternal
*
* @brief   Enable/disable local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. If the packet is received from a cascade port, the packets source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*       2. For xCat and above devices to enable local switching of known
*       Unicast traffic, both ingress port configuration and VLAN entry (by
*       function cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
static GT_STATUS internal_cpssDxChBrgGenUcLocalSwitchingEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;     /* register address */
    GT_U32      value;       /* value to write into register */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <UC_LOCAL_CMD>*/
         rc = prvCpssDxChWriteTableEntryField(devNum,
                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                       portNum,
                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                       SIP5_L2I_EPORT_TABLE_FIELDS_UC_LOCAL_CMD_E, /* field name */
                       PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                       value);

    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum, portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <UcLocalEn> */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 1, 1, value);
    }

    return rc;
}

/**
* @internal cpssDxChBrgGenUcLocalSwitchingEnable function
* @endinternal
*
* @brief   Enable/disable local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. If the packet is received from a cascade port, the packets source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*       2. For xCat and above devices to enable local switching of known
*       Unicast traffic, both ingress port configuration and VLAN entry (by
*       function cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
GT_STATUS cpssDxChBrgGenUcLocalSwitchingEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenUcLocalSwitchingEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenUcLocalSwitchingEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenUcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get status of local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note 1. If the packet is received from a cascade port, the packets source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*       2. For xCat and above devices to enable local switching of known
*       Unicast traffic, both ingress port configuration and VLAN entry (by
*       function cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
static GT_STATUS internal_cpssDxChBrgGenUcLocalSwitchingEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;     /* register address */
    GT_U32      value;       /* value to write into register */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set Bridge-Ingress-ePort table */
         rc = prvCpssDxChReadTableEntryField(devNum,
                      CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                      portNum,
                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                      SIP5_L2I_EPORT_TABLE_FIELDS_UC_LOCAL_CMD_E, /* field name */
                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                      &value);

    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum, portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <UcLocalEn> */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 1, 1, &value);
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgGenUcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get status of local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note 1. If the packet is received from a cascade port, the packets source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*       2. For xCat and above devices to enable local switching of known
*       Unicast traffic, both ingress port configuration and VLAN entry (by
*       function cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
GT_STATUS cpssDxChBrgGenUcLocalSwitchingEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenUcLocalSwitchingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenUcLocalSwitchingEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIgmpSnoopModeSet function
* @endinternal
*
* @brief   Set global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Trap mode: All IGMP packets are trapped to the CPU,
*                                      regardless of their type.
*                                      IGMP Snoop mode: Query messages are mirrored to the CPU.
*                                      Non-Query messages are Trapped to the CPU.
*                                      IGMP Router mode: All IGMP packets are mirrored to the CPU,
*                                      regardless of their type.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIgmpSnoopModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_IGMP_SNOOP_MODE_ENT     mode
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 offset;      /* offset in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        offset = 6;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        bridgeRegs.bridgeGlobalConfigRegArray[0];
        offset = 12;
    }

    switch (mode)
    {
        case CPSS_IGMP_ALL_TRAP_MODE_E:
            value = 0;
            break;
        case CPSS_IGMP_SNOOP_TRAP_MODE_E:
            value = 1;
            break;
        case CPSS_IGMP_ROUTER_MIRROR_MODE_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
   }

    /* Bridge Global Configuration Register0, <IGMPMode> */
    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 2, value);
}

/**
* @internal cpssDxChBrgGenIgmpSnoopModeSet function
* @endinternal
*
* @brief   Set global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Trap mode: All IGMP packets are trapped to the CPU,
*                                      regardless of their type.
*                                      IGMP Snoop mode: Query messages are mirrored to the CPU.
*                                      Non-Query messages are Trapped to the CPU.
*                                      IGMP Router mode: All IGMP packets are mirrored to the CPU,
*                                      regardless of their type.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_IGMP_SNOOP_MODE_ENT     mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIgmpSnoopModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgGenIgmpSnoopModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIgmpSnoopModeGet function
* @endinternal
*
* @brief   Get status of global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to)Trap/Mirror mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
static GT_STATUS internal_cpssDxChBrgGenIgmpSnoopModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_IGMP_SNOOP_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 offset;      /* offset in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        offset = 6;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        bridgeRegs.bridgeGlobalConfigRegArray[0];
        offset = 12;
    }

    /* Bridge Global Configuration Register0, <IGMPMode> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 2, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
        case 0:
            *modePtr =  CPSS_IGMP_ALL_TRAP_MODE_E;
            break;
        case 1:
            *modePtr =  CPSS_IGMP_SNOOP_TRAP_MODE_E;
            break;
        case 2:
            *modePtr =  CPSS_IGMP_ROUTER_MIRROR_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
   }

    return rc;
}

/**
* @internal cpssDxChBrgGenIgmpSnoopModeGet function
* @endinternal
*
* @brief   Get status of global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to)Trap/Mirror mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_IGMP_SNOOP_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIgmpSnoopModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgGenIgmpSnoopModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenArpBcastToCpuCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs or ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmdMode                  - command interface mode
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, command mode or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For ARP broadcast received on VLAN - IPv4/IPv6 Control Traffic To CPU must be enabled by function
*       cpssDxChBrgVlanIpCntlToCpuSet
*       For ARP broadcast received on port - ARP Broadcast Trap To CPU must be enabled by function
*       cpssDxChBrgGenArpTrapEnable
*
*/
static GT_STATUS internal_cpssDxChBrgGenArpBcastToCpuCmdSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    IN CPSS_PACKET_CMD_ENT              cmd
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset;  /* The start bit number in the register */
    GT_U32 fieldLength;  /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value, cmd)

        switch (cmdMode)
        {
            case CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E:
                /*port ARP Broadcast Command*/
                fieldOffset = 24;
                fieldLength = 3;
                break;
            case CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E:
                /*VLAN ARP Broadcast Command*/
                fieldOffset = 21;
                fieldLength = 3;
                break;
            default:
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                bridgeEngineConfig.bridgeCommandConfig1;

    }
    else
    {
        switch (cmd)
        {
            case CPSS_PACKET_CMD_FORWARD_E:
                value = 0;
                break;
            case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
                value = 1;
                break;
            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                value = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /*VLAN ARP Broadcast Command*/
        fieldOffset = 7;
        fieldLength = 2;

        /* Bridge Global Configuration Register0, field <ARPBCCmd> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bridgeRegs.bridgeGlobalConfigRegArray[0];

    }

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, value);
}

/**
* @internal cpssDxChBrgGenArpBcastToCpuCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs or ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmdMode                  - command interface mode
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, command mode or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For ARP broadcast received on VLAN - IPv4/IPv6 Control Traffic To CPU must be enabled by function
*       cpssDxChBrgVlanIpCntlToCpuSet
*       For ARP broadcast received on port - ARP Broadcast Trap To CPU must be enabled by function
*       cpssDxChBrgGenArpTrapEnable
*
*/
GT_STATUS cpssDxChBrgGenArpBcastToCpuCmdSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    IN CPSS_PACKET_CMD_ENT              cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenArpBcastToCpuCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cmdMode, cmd));

    rc = internal_cpssDxChBrgGenArpBcastToCpuCmdSet(devNum, cmdMode, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cmdMode, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenArpBcastToCpuCmdGet function
* @endinternal
*
* @brief   Get status of CPU ARP Broadcast packets for all VLANs or ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmdMode                  - command interface mode
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, command mode or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
static GT_STATUS internal_cpssDxChBrgGenArpBcastToCpuCmdGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    OUT CPSS_PACKET_CMD_ENT             *cmdPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset;  /* The start bit number in the register */
    GT_U32 fieldLength;  /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        switch (cmdMode)
        {
            case CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E:
                /*port ARP Broadcast Command*/
                fieldOffset = 24;
                fieldLength = 3;
                break;
            case CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E:
                /*VLAN ARP Broadcast Command*/
                fieldOffset = 21;
                fieldLength = 3;
                break;
            default:
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;
    }
    else
    {
        /*VLAN ARP Broadcast Command*/
        fieldOffset = 7;
        fieldLength = 2;

        /* Bridge Global Configuration Register0, field <ARPBCCmd> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];
    }

    /* Bridge Global Configuration Register0, field <ARPBCCmd> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*cmdPtr, value);

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenArpBcastToCpuCmdGet function
* @endinternal
*
* @brief   Get status of CPU ARP Broadcast packets for all VLANs or ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmdMode                  - command interface mode
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, command mode or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenArpBcastToCpuCmdGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    OUT CPSS_PACKET_CMD_ENT             *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenArpBcastToCpuCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cmdMode, cmdPtr));

    rc = internal_cpssDxChBrgGenArpBcastToCpuCmdGet(devNum, cmdMode, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cmdMode, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRipV1MirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE = Forward normally
*                                      GT_TRUE  = Mirror-to-CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenRipV1MirrorToCpuEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;
        value = enable ? CPSS_PACKET_CMD_MIRROR_TO_CPU_E : CPSS_PACKET_CMD_FORWARD_E;

        return prvCpssHwPpSetRegField(devNum, regAddr, 15, 3, value);

    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[0];

    value = BOOL2BIT_MAC(enable);

    /* Bridge Global Configuration Register0, field <IPv4RIPv1MirrorEn> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 28, 1, value);
}

/**
* @internal cpssDxChBrgGenRipV1MirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE = Forward normally
*                                      GT_TRUE  = Mirror-to-CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenRipV1MirrorToCpuEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRipV1MirrorToCpuEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgGenRipV1MirrorToCpuEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRipV1MirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE = Forward normally
*                                      GT_TRUE  = Mirror-to-CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenRipV1MirrorToCpuEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        rc =  prvCpssHwPpGetRegField(devNum, regAddr, 15, 3, &value);
        if(GT_OK != rc)
        {
            return rc;
        }

        *enablePtr = (value == CPSS_PACKET_CMD_MIRROR_TO_CPU_E) ? GT_TRUE : GT_FALSE;
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[0];

    /* Bridge Global Configuration Register0, field <IPv4RIPv1MirrorEn> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 28, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgGenRipV1MirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE = Forward normally
*                                      GT_TRUE  = Mirror-to-CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenRipV1MirrorToCpuEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRipV1MirrorToCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgGenRipV1MirrorToCpuEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIeeeReservedMcastTrapEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIeeeReservedMcastTrapEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 offset;      /* offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        offset = 3;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[0];
        offset = 5;
    }

    value = BOOL2BIT_MAC(enable);

    /* Bridge Global Configuration Register0, field <IEEEReservedMulticastToCPUEn> */
    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 1, value);
}

/**
* @internal cpssDxChBrgGenIeeeReservedMcastTrapEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastTrapEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIeeeReservedMcastTrapEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgGenIeeeReservedMcastTrapEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIeeeReservedMcastTrapEnableGet function
* @endinternal
*
* @brief   Get status of trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenIeeeReservedMcastTrapEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 offset;      /* offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        offset = 3;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[0];
        offset = 5;
    }

    /* Bridge Global Configuration Register0, field <IEEEReservedMulticastToCPUEn> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgGenIeeeReservedMcastTrapEnableGet function
* @endinternal
*
* @brief   Get status of trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastTrapEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIeeeReservedMcastTrapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
* @endinternal
*
* @brief   Enables forwarding, trapping, or mirroring to the CPU any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..3).
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
*                                      The parameter defines profile (table number)
*                                      for the 256 protocols. Profile bound per port by
*                                      <cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh1, DxCh2, DxCh3 devices:
*       BPDU packets with MAC DA = 01-80-C2-00-00-00 are not affect
*       by this mechanism. BPDUs are implicitly trapped to the CPU
*       if the ingress port span tree is not disabled.
*
*/
static GT_STATUS internal_cpssDxChBrgGenIeeeReservedMcastProtCmdSet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 field;       /* register field offset */
    GT_U32 value;       /* value to write into register */
    GT_U32 regData;     /* register data */
    GT_U32 regMask;     /* register data mask*/
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch (cmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            value = 0;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            value = 1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            value = 2;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            value = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* ignore profile index input value but always use first register in the DB */
        profileIndex = 0;
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* Bobcat2, Caelum, Bobcat3 devices supports only 8 profiles */
            if (profileIndex >= 8)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            /* xCat and above devices supports only 4 profiles */
            if (profileIndex >= 4)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IEEEReservedMcConfig0[protocol / 16] +
                    (profileIndex * (
                        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                ctrlTrafficToCPUConfig.IEEEReservedMcConfig1[0] -
                        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                ctrlTrafficToCPUConfig.IEEEReservedMcConfig0[0]));
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            bridgeRegs.macRangeFltrBase[profileIndex] + 0x1000 * (protocol / 16);
    }

    field = 2 * (protocol % 16);

    /* IEEE Reserved Multicast Configuration Register<n> */

    /* IEEE Reserved Multicast Configuration registers read errata */
    if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
          PRV_CPSS_DXCH_XCAT_IEEE_RESERVED_MC_CONFG_REG_READ_WA_E) == GT_TRUE)
    {
        if(profileIndex > 1)
        {
            rc = prvCpssHwPpReadRegister(devNum, (regAddr + 0x10), &regData);
            if(rc != GT_OK)
            {
                return rc;
            }

            regMask = ~(0x3 << field);
            regData &= regMask;
            regData |= value << field;
            return prvCpssHwPpWriteRegister(devNum, regAddr, regData);
        }
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, field, 2, value);
}

/**
* @internal cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
* @endinternal
*
* @brief   Enables forwarding, trapping, or mirroring to the CPU any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..3).
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
*                                      The parameter defines profile (table number)
*                                      for the 256 protocols. Profile bound per port by
*                                      <cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh1, DxCh2, DxCh3 devices:
*       BPDU packets with MAC DA = 01-80-C2-00-00-00 are not affect
*       by this mechanism. BPDUs are implicitly trapped to the CPU
*       if the ingress port span tree is not disabled.
*
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastProtCmdSet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIeeeReservedMcastProtCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, protocol, cmd));

    rc = internal_cpssDxChBrgGenIeeeReservedMcastProtCmdSet(devNum, profileIndex, protocol, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, protocol, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIeeeReservedMcastProtCmdGet function
* @endinternal
*
* @brief   Gets command (forwarding, trapping, or mirroring to the CPU) any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..3).
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
*                                      The parameter defines profile (table number)
*                                      for the 256 protocols. Profile bound per port by
*                                      <cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIeeeReservedMcastProtCmdGet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    OUT CPSS_PACKET_CMD_ENT *cmdPtr
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 field;       /* register field offset */
    GT_U32 value;       /* value to read from register */
    GT_STATUS rc;       /* returned status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* ignore profile index input value but always use first register in the DB */
        profileIndex = 0;
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* Bobcat2, Caelum, Bobcat3 devices supports only 8 profiles */
            if (profileIndex >= 8)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            /* xCat and above devices supports only 4 profiles */
            if (profileIndex >= 4)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IEEEReservedMcConfig0[protocol / 16] +
                    (profileIndex * (
                        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                ctrlTrafficToCPUConfig.IEEEReservedMcConfig1[0] -
                        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                ctrlTrafficToCPUConfig.IEEEReservedMcConfig0[0]));
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        bridgeRegs.macRangeFltrBase[profileIndex] + 0x1000 * (protocol / 16);
    }

    /* IEEE Reserved Multicast Configuration registers read errata */
    if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
          PRV_CPSS_DXCH_XCAT_IEEE_RESERVED_MC_CONFG_REG_READ_WA_E) == GT_TRUE)
    {
        if(profileIndex > 1)
        {
            regAddr += 0x10;
        }
    }

    field = 2 * (protocol % 16);

    rc = prvCpssHwPpGetRegField(devNum, regAddr, field, 2, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
        case 0:
            *cmdPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            *cmdPtr = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            *cmdPtr = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        default:
            *cmdPtr = CPSS_PACKET_CMD_DROP_SOFT_E;
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenIeeeReservedMcastProtCmdGet function
* @endinternal
*
* @brief   Gets command (forwarding, trapping, or mirroring to the CPU) any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..3).
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
*                                      The parameter defines profile (table number)
*                                      for the 256 protocols. Profile bound per port by
*                                      <cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastProtCmdGet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    OUT CPSS_PACKET_CMD_ENT *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIeeeReservedMcastProtCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, protocol, cmdPtr));

    rc = internal_cpssDxChBrgGenIeeeReservedMcastProtCmdGet(devNum, profileIndex, protocol, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, protocol, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenCiscoL2ProtCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenCiscoL2ProtCmdSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset; /* The start bit number in the register */
    GT_U32 fieldLength;  /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        fieldOffset = 18;
        fieldLength = 3;

    }
    else
    {
        /* Bridge Global Configuration Register0, field <CiscoCommand> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];

        fieldOffset = 9;
        fieldLength = 2;
    }

    switch (cmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            value = 0;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            value = 1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, value);
}

/**
* @internal cpssDxChBrgGenCiscoL2ProtCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenCiscoL2ProtCmdSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenCiscoL2ProtCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cmd));

    rc = internal_cpssDxChBrgGenCiscoL2ProtCmdSet(devNum, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenCiscoL2ProtCmdGet function
* @endinternal
*
* @brief   Get status (trapping or mirroring) of the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
static GT_STATUS internal_cpssDxChBrgGenCiscoL2ProtCmdGet
(
    IN  GT_U8                devNum,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset; /* The start bit number in the register */
    GT_U32 fieldLength;  /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        fieldOffset = 18;
        fieldLength = 3;

    }
    else
    {
        /* Bridge Global Configuration Register0, field <CiscoCommand> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];

        fieldOffset = 9;
        fieldLength = 2;
    }

    /* Bridge Global Configuration Register0, field <CiscoCommand> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
        case 0:
            *cmdPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1 :
            *cmdPtr = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            *cmdPtr = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenCiscoL2ProtCmdGet function
* @endinternal
*
* @brief   Get status (trapping or mirroring) of the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenCiscoL2ProtCmdGet
(
    IN  GT_U8                devNum,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenCiscoL2ProtCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cmdPtr));

    rc = internal_cpssDxChBrgGenCiscoL2ProtCmdGet(devNum, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpV6IcmpTrapEnable function
* @endinternal
*
* @brief   Enable/disable IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE -
*                                      IPv6 ICMP packets received on this port are not trapped
*                                      to the CPU. These packets may be trapped or mirrored
*                                      to the CPU based on the VID assigned to the packet
*                                      see cpssDxChBrgVlanIpV6IcmpToCpuEnable function
*                                      GT_TRUE -
*                                      IPv6 ICMP packets with a ICMP type that matches one of the
*                                      ICMP types configured in the IPv6 ICMP Message Type
*                                      Configuration Register<n> by function
*                                      cpssDxChBrgGenIcmpv6MsgTypeSet are trapped to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpV6IcmpTrapEnable
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;     /* register address */
    GT_U32      value;       /* value to write into register */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    value = BOOL2BIT_MAC(enable);

        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <ICMPTrapEn> */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 14, 1, value);

    return rc;
}

/**
* @internal cpssDxChBrgGenIpV6IcmpTrapEnable function
* @endinternal
*
* @brief   Enable/disable IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE -
*                                      IPv6 ICMP packets received on this port are not trapped
*                                      to the CPU. These packets may be trapped or mirrored
*                                      to the CPU based on the VID assigned to the packet
*                                      see cpssDxChBrgVlanIpV6IcmpToCpuEnable function
*                                      GT_TRUE -
*                                      IPv6 ICMP packets with a ICMP type that matches one of the
*                                      ICMP types configured in the IPv6 ICMP Message Type
*                                      Configuration Register<n> by function
*                                      cpssDxChBrgGenIcmpv6MsgTypeSet are trapped to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6IcmpTrapEnable
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpV6IcmpTrapEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenIpV6IcmpTrapEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpV6IcmpTrapEnableGet function
* @endinternal
*
* @brief   Gets status of IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE -
*                                      IPv6 ICMP packets received on this port are not trapped
*                                      to the CPU. These packets may be trapped or mirrored
*                                      to the CPU based on the VID assigned to the packet
*                                      see cpssDxChBrgVlanIpV6IcmpToCpuEnable function
*                                      GT_TRUE -
*                                      IPv6 ICMP packets with a ICMP type that matches one of the
*                                      ICMP types configured in the IPv6 ICMP Message Type
*                                      Configuration Register<n> by function
*                                      cpssDxChBrgGenIcmpv6MsgTypeSet are trapped to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenIpV6IcmpTrapEnableGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;          /* return status */
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <ICMPTrapEn> */
        rc  = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 14, 1, &value);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;

}

/**
* @internal cpssDxChBrgGenIpV6IcmpTrapEnableGet function
* @endinternal
*
* @brief   Gets status of IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE -
*                                      IPv6 ICMP packets received on this port are not trapped
*                                      to the CPU. These packets may be trapped or mirrored
*                                      to the CPU based on the VID assigned to the packet
*                                      see cpssDxChBrgVlanIpV6IcmpToCpuEnable function
*                                      GT_TRUE -
*                                      IPv6 ICMP packets with a ICMP type that matches one of the
*                                      ICMP types configured in the IPv6 ICMP Message Type
*                                      Configuration Register<n> by function
*                                      cpssDxChBrgGenIcmpv6MsgTypeSet are trapped to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChBrgGenIpV6IcmpTrapEnableGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpV6IcmpTrapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenIpV6IcmpTrapEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIcmpv6MsgTypeSet function
* @endinternal
*
* @brief   Set trap/mirror/forward command for specified ICMP message type.
*         To enable ICMPv6 trapping/mirroring on a VLAN interface,
*         set the <IPv6ICMP ToCPUEn> bit in the VLAN<n> Entry by function
*         cpssDxChBrgVlanIpV6IcmpToCpuEnable
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the  of the message type(up to eight IPv6 ICMP types)
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] msgType                  - ICMP message type
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
*                                       or index > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIcmpv6MsgTypeSet
(
    IN GT_U8                devNum,
    IN GT_U32               index,
    IN GT_U8                msgType,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32  regAddr;                /* register address */
    GT_U32  value;                  /* value to write into register */
    GT_U32  field;                  /* register field offset */
    GT_U32  rc;                     /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (index >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (cmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            value = 0;
            break;

        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            value = 1;
            break;

        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            value = 2;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv6ICMPCommand;
        field = 3 * index;

        /* IPv6 ICMP Command Register <ICMPCmd> */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, field, 3, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv6ICMPMsgTypeConfig[index / 4];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.ipv6IcmpMsgCmd;
        field = 2 * index;

        /* IPv6 ICMP Command Register <ICMPCmd> */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, field, 2, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            bridgeRegs.ipv6IcmpMsgTypeBase + 0x1000 * (index / 4);
    }

    field = 8 * (index % 4);
    value = msgType;

    /* IPv6 ICMP Message Type Configuration Register<n> <ICMPMsgType<> */
    return prvCpssHwPpSetRegField(devNum, regAddr, field, 8, value);
}

/**
* @internal cpssDxChBrgGenIcmpv6MsgTypeSet function
* @endinternal
*
* @brief   Set trap/mirror/forward command for specified ICMP message type.
*         To enable ICMPv6 trapping/mirroring on a VLAN interface,
*         set the <IPv6ICMP ToCPUEn> bit in the VLAN<n> Entry by function
*         cpssDxChBrgVlanIpV6IcmpToCpuEnable
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the  of the message type(up to eight IPv6 ICMP types)
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] msgType                  - ICMP message type
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
*                                       or index > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIcmpv6MsgTypeSet
(
    IN GT_U8                devNum,
    IN GT_U32               index,
    IN GT_U8                msgType,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIcmpv6MsgTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, msgType, cmd));

    rc = internal_cpssDxChBrgGenIcmpv6MsgTypeSet(devNum, index, msgType, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, msgType, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIcmpv6MsgTypeGet function
* @endinternal
*
* @brief   Get status of trap/mirror/forward command for specified ICMP message type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the  of the message type(up to eight IPv6 ICMP types)
*                                      (APPLICABLE RANGES: 0..7)
*
* @param[out] msgTypePtr               - ICMP message type
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
*                                       or index > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
static GT_STATUS internal_cpssDxChBrgGenIcmpv6MsgTypeGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               index,
    OUT GT_U8                *msgTypePtr,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{
    GT_U32  regAddr;                /* register address */
    GT_U32  value;                  /* register value  */
    GT_U32  field;                  /* register field offset */
    GT_U32  rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(msgTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    if (index >= BIT_3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv6ICMPCommand;
        field = 3 * index;

        /* IPv6 ICMP Command Register <ICMPCmd> */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, field, 3, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.ipv6IcmpMsgCmd;
        field = 2 * index;

        /* IPv6 ICMP Command Register <ICMPCmd> */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, field, 2, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    switch (value)
    {
        case 0:
            *cmdPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;

        case 1:
            *cmdPtr = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;

        case 2:
            *cmdPtr = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv6ICMPMsgTypeConfig[index / 4];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        bridgeRegs.ipv6IcmpMsgTypeBase + 0x1000 * (index / 4);
    }
    field = 8 * (index % 4);

    /* IPv6 ICMP Message Type Configuration Register<n> <ICMPMsgType<> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, field, 8, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *msgTypePtr = (GT_U8)value;

    return rc;
}

/**
* @internal cpssDxChBrgGenIcmpv6MsgTypeGet function
* @endinternal
*
* @brief   Get status of trap/mirror/forward command for specified ICMP message type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the  of the message type(up to eight IPv6 ICMP types)
*                                      (APPLICABLE RANGES: 0..7)
*
* @param[out] msgTypePtr               - ICMP message type
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
*                                       or index > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenIcmpv6MsgTypeGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               index,
    OUT GT_U8                *msgTypePtr,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIcmpv6MsgTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, msgTypePtr, cmdPtr));

    rc = internal_cpssDxChBrgGenIcmpv6MsgTypeGet(devNum, index, msgTypePtr, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, msgTypePtr, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
* @param[in] enable                   - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or IP protocol version
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      enable
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into register */
    GT_U32 field;           /* register field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;

        switch (protocolStack)
        {
        case CPSS_IP_PROTOCOL_IPV4_E:
            field = 12;
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            field = 5;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        /* Bridge Global Configuration Register0, field <IPv4/6LinkLocalMirrorEn> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];

        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                field = 23;
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                field = 11;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }

    value = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, field, 1, value);
}

/**
* @internal cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
* @param[in] enable                   - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or IP protocol version
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enable));

    rc = internal_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(devNum, protocolStack, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of mirroring to CPU
*         for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
*
* @param[out] enablePtr                - pointer to enable/disable status of mirroring to CPU.
*                                      GT_TRUE  - mirroring to CPU for IP Link Local
*                                      Control protocol is enabled.
*                                      GT_FALSE - mirroring to CPU for IP Link Local
*                                      Control protocol is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or IP protocol version
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into register */
    GT_U32 field;           /* register field offset */
    GT_STATUS rc;           /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;

        switch (protocolStack)
        {
        case CPSS_IP_PROTOCOL_IPV4_E:
            field = 12;
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            field = 5;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        /* Bridge Global Configuration Register0, field <IPv4/6LinkLocalMirrorEn> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];

        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                field = 23;
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                field = 11;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, field, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of mirroring to CPU
*         for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
*
* @param[out] enablePtr                - pointer to enable/disable status of mirroring to CPU.
*                                      GT_TRUE  - mirroring to CPU for IP Link Local
*                                      Control protocol is enabled.
*                                      GT_FALSE - mirroring to CPU for IP Link Local
*                                      Control protocol is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or IP protocol version
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enablePtr));

    rc = internal_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(devNum, protocolStack, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocol                 - IPv4 or IPv6 protocol version
* @param[in] protocol                 - link local IP protocol
* @param[in] enable                   - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, IP protocol version or
*                                       Link Local IP protocol
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U8                        protocol,
    IN GT_BOOL                      enable
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into register */
    GT_U32 field;           /* register field offset */
    GT_U32 bits;            /* register field width */
    GT_U32 index;           /* array index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    field = protocol % 32;
    bits = 1;
    index = protocol / 32;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            bits = 3;
            field = 3 * (protocol % 8);
            index = protocol / 8;

            value = (enable == GT_TRUE) ? CPSS_PACKET_CMD_MIRROR_TO_CPU_E : CPSS_PACKET_CMD_FORWARD_E;
        }
        else
        {
            value = BOOL2BIT_MAC(enable);
        }
    }
    else
    {
        value = BOOL2BIT_MAC(enable);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv4McLinkLocalConfig[index];
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv6McLinkLocalConfig[index];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.ipv4McastLinkLocalBase + 0x1000 * (index);
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.ipv6McastLinkLocalBase + 0x1000 * (index);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    /* IPv4/6 Multicast Link-Local Configuration Register<n> <IPv4MulLinkLocalMirrorEn> */
    return prvCpssHwPpSetRegField(devNum, regAddr, field, bits, value);
}

/**
* @internal cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocol                 - IPv4 or IPv6 protocol version
* @param[in] protocol                 - link local IP protocol
* @param[in] enable                   - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, IP protocol version or
*                                       Link Local IP protocol
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U8                        protocol,
    IN GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, protocol, enable));

    rc = internal_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(devNum, protocolStack, protocol, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, protocol, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocol                 - IPv4 or IPv6 protocol version
* @param[in] protocol                 - link local IP protocol
*
* @param[out] enablePtr                - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, IP protocol version or
*                                       Link Local IP protocol
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN  GT_U8                        protocol,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into register */
    GT_U32 field;           /* register field offset */
    GT_U32 bits;            /* register field width */
    GT_U32 index;           /* array index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    field = protocol % 32;
    bits = 1;
    index = protocol / 32;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            bits = 3;
            field = 3 * (protocol % 8);
            index = protocol / 8;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv4McLinkLocalConfig[index];
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    ctrlTrafficToCPUConfig.IPv6McLinkLocalConfig[index];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.ipv4McastLinkLocalBase + 0x1000 * (index);
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.ipv6McastLinkLocalBase + 0x1000 * (index);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* IPv4/6 Multicast Link-Local Configuration Register<n> <IPv4MulLinkLocalMirrorEn> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, field, bits, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            *enablePtr = (value == CPSS_PACKET_CMD_FORWARD_E) ? GT_FALSE : GT_TRUE;
        }
        else
        {
            *enablePtr = BIT2BOOL_MAC(value);
        }
    }
    else
    {
        *enablePtr = BIT2BOOL_MAC(value);
    }


    return rc;
}

/**
* @internal cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocol                 - IPv4 or IPv6 protocol version
* @param[in] protocol                 - link local IP protocol
*
* @param[out] enablePtr                - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, IP protocol version or
*                                       Link Local IP protocol
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN  GT_U8                        protocol,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, protocol, enablePtr));

    rc = internal_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(devNum, protocolStack, protocol, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, protocol, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpV6SolicitedCmdSet function
* @endinternal
*
* @brief   Set trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmd                      - supported commands:   CPSS_PACKET_CMD_FORWARD_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or packet control command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpV6SolicitedCmdSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset; /* The start bit number in the register */
    GT_U32 fieldLength; /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        fieldOffset = 12;
        fieldLength = 3;

    }
    else
    {
        /* Bridge Global Configuration Register0, field <IPv6NeighborSolicitedNodeCmd> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];

        fieldOffset = 1;
        fieldLength = 2;
    }

    switch (cmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            value = 0;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            value = 1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, value);
}

/**
* @internal cpssDxChBrgGenIpV6SolicitedCmdSet function
* @endinternal
*
* @brief   Set trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmd                      - supported commands:   CPSS_PACKET_CMD_FORWARD_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or packet control command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedCmdSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpV6SolicitedCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cmd));

    rc = internal_cpssDxChBrgGenIpV6SolicitedCmdSet(devNum, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpV6SolicitedCmdGet function
* @endinternal
*
* @brief   Get status of trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or packet control command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
static GT_STATUS internal_cpssDxChBrgGenIpV6SolicitedCmdGet
(
    IN  GT_U8                devNum,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset; /* The start bit number in the register */
    GT_U32 fieldLength; /* The number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_EARCH
        /* need to add support for 5 commands */

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        fieldOffset = 12;
        fieldLength = 3;

    }
    else
    {
        /* Bridge Global Configuration Register0, field <IPv6NeighborSolicitedNodeCmd> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];

        fieldOffset = 1;
        fieldLength = 2;
    }

    /* Bridge Global Configuration Register0, field <IPv6NeighborSolicitedNodeCmd> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch (value)
    {
        case 0:
            *cmdPtr = CPSS_PACKET_CMD_FORWARD_E;
            value = 0;
            break;
        case 1:
            *cmdPtr = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            *cmdPtr = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgGenIpV6SolicitedCmdGet function
* @endinternal
*
* @brief   Get status of trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or packet control command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedCmdGet
(
    IN  GT_U8                devNum,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpV6SolicitedCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cmdPtr));

    rc = internal_cpssDxChBrgGenIpV6SolicitedCmdGet(devNum, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet function
* @endinternal
*
* @brief   Set IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] addressPtr               - (pointer to)IP address
* @param[in] maskPtr                  - (pointer to) mask of the address
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet
(
    IN GT_U8        devNum,
    IN GT_IPV6ADDR  *addressPtr,
    IN GT_IPV6ADDR  *maskPtr
)
{

    GT_U32 regAddr1[2], regAddr2[2];/* register address                      */
    GT_U32 numPipes;                /* number of pipes to set 1 or 2         */
    GT_U32 pipeId;                  /* pipe ID (0 - ingress , 1 - egress)    */
    GT_U32 regNum;                  /* address and mask bits register number */
    GT_U32 regData;                 /* register's data                       */
    GT_U32 rc;                      /* function return code                  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(addressPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* registers for the ingress PIPE to recognize those 'Ipv6_ND' (set in TTI , used by bridge) */
        regAddr1[0] = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg0;
        regAddr2[0] = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg0;

        /* registers for the egress PIPE to recognize those 'Ipv6_ND' (set in HA , used by EPCL) */
        regAddr1[1] = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).egrPolicyDIPSolicitationData0;
        regAddr2[1] = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).egrPolicyDIPSolicitationMask0;

        numPipes = 2;
    }
    else
    {
        /* IPv6 Solicited-Node Multicast Address Configuration Register0  */
        regAddr1[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.ipv6McastSolicitNodeAddrBase;

        /*IPv6 Solicited-Node Multicast Address Mask Register0*/
        regAddr2[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.ipv6McastSolicitNodeMaskBase;
        numPipes = 1;
    }

    for(pipeId = 0 ; pipeId < numPipes; pipeId++)
    {
        for(regNum = 0; regNum < 4; regNum++)
        {
            /* first register comprize LSB of IP address- bits 31:0
               second register comprize bits 63:32 and so on  */
            regData =  (addressPtr->arIP[((3 - regNum) * 4)]     << 24) |
                       (addressPtr->arIP[((3 - regNum) * 4) + 1] << 16) |
                       (addressPtr->arIP[((3 - regNum) * 4) + 2] << 8)  |
                       (addressPtr->arIP[((3 - regNum) * 4) + 3]);

            /* IPv6 Solicited-Node Multicast Address Configuration Register
               field  <IPv6SNMCAddr> */
            rc = prvCpssHwPpWriteRegister(devNum, regAddr1[pipeId], regData);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* first register comprize LSB of IP mask - bits 31:0
               second register comprize bits 63:32 and so on  */
            regData =  (maskPtr->arIP[((3 - regNum) * 4)]     << 24) |
                       (maskPtr->arIP[((3 - regNum) * 4) + 1] << 16) |
                       (maskPtr->arIP[((3 - regNum) * 4) + 2] << 8)  |
                       (maskPtr->arIP[((3 - regNum) * 4) + 3]);

            /* IPv6 Solicited-Node Multicast Address Mask Register,
               field <IPv6SNMCAddrMask> */
            rc = prvCpssHwPpWriteRegister(devNum, regAddr2[pipeId], regData);
            if (rc != GT_OK)
            {
                return rc;
            }

            regAddr1[pipeId] += 0x4;
            regAddr2[pipeId] += 0x4;
        }
    }




    return GT_OK;
}

/**
* @internal cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet function
* @endinternal
*
* @brief   Set IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] addressPtr               - (pointer to)IP address
* @param[in] maskPtr                  - (pointer to) mask of the address
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet
(
    IN GT_U8        devNum,
    IN GT_IPV6ADDR  *addressPtr,
    IN GT_IPV6ADDR  *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addressPtr, maskPtr));

    rc = internal_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(devNum, addressPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, addressPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet function
* @endinternal
*
* @brief   Get IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] addressPtr               - (pointer to)IP address
* @param[out] maskPtr                  - (pointer to) mask of the address
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet
(
    IN  GT_U8        devNum,
    OUT GT_IPV6ADDR  *addressPtr,
    OUT GT_IPV6ADDR  *maskPtr
)
{

    GT_U32 regAddr1, regAddr2;      /* register address                      */
    GT_U32 regNum;                  /* address and mask bits register number */
    GT_U32 regData;                 /* register's data                       */
    GT_U32 rc;                      /* function return code                  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(addressPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* registers for the ingress PIPE to recognize those 'Ipv6_ND' (set in TTI , used by bridge) */
        regAddr1 = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg0;
        regAddr2 = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg0;
    }
    else
    {
        /* IPv6 Solicited-Node Multicast Address Configuration Register0  */
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.ipv6McastSolicitNodeAddrBase;

        /*IPv6 Solicited-Node Multicast Address Mask Register0*/
        regAddr2 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.ipv6McastSolicitNodeMaskBase;
    }

    for(regNum = 0; regNum < 4; regNum++)
    {

        /* IPv6 Solicited-Node Multicast Address Configuration Register
           field  <IPv6SNMCAddr> */
        rc = prvCpssHwPpReadRegister(devNum, regAddr1, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }

        addressPtr->arIP[((3 - regNum) * 4)]     = (GT_U8)((regData >> 24) & 0xFF);
        addressPtr->arIP[((3 - regNum) * 4) + 1] = (GT_U8)((regData >> 16) & 0xFF);
        addressPtr->arIP[((3 - regNum) * 4) + 2] = (GT_U8)((regData >> 8) & 0xFF);
        addressPtr->arIP[((3 - regNum) * 4) + 3] = (GT_U8)(regData & 0xFF);


        /* IPv6 Solicited-Node Multicast Address Mask Register,
           field <IPv6SNMCAddrMask> */
        rc = prvCpssHwPpReadRegister(devNum, regAddr2, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }

        maskPtr->arIP[((3 - regNum) * 4)]     = (GT_U8)((regData >> 24) & 0xFF);
        maskPtr->arIP[((3 - regNum) * 4) + 1] = (GT_U8)((regData >> 16) & 0xFF);
        maskPtr->arIP[((3 - regNum) * 4) + 2] = (GT_U8)((regData >> 8) & 0xFF);
        maskPtr->arIP[((3 - regNum) * 4) + 3] = (GT_U8)(regData & 0xFF);

        regAddr1 += 0x4;
        regAddr2 += 0x4;
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet function
* @endinternal
*
* @brief   Get IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] addressPtr               - (pointer to)IP address
* @param[out] maskPtr                  - (pointer to) mask of the address
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet
(
    IN  GT_U8        devNum,
    OUT GT_IPV6ADDR  *addressPtr,
    OUT GT_IPV6ADDR  *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addressPtr, maskPtr));

    rc = internal_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(devNum, addressPtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, addressPtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRateLimitGlobalCfgSet function
* @endinternal
*
* @brief   Configures global ingress rate limit parameters - rate limiting mode,
*         Four configurable global time window periods are assigned to ports as
*         a function of their operating speed:
*         10 Gbps, 1 Gbps, 100 Mbps, and 10 Mbps.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported windows for DxCh devices:
*       1000 Mbps: range - 256-16384 uSec  granularity - 256 uSec
*       100 Mbps: range - 256-131072 uSec  granularity - 256 uSec
*       10  Mbps: range - 256-1048570 uSec granularity - 256 uSec
*       10  Gbps: range - 25.6-104857 uSec granularity - 25.6 uSec
*       For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
static GT_STATUS internal_cpssDxChBrgGenRateLimitGlobalCfgSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC     *brgRateLimitPtr
)
{
    GT_U32  regAddr;            /* register address */
    GT_STATUS rc;               /* function return code */
    GT_U32  value;              /* value to write into register */
    GT_U32  granularity;        /* rate limit window granularity */
    GT_U32  win1000Mbps = 0;    /* window time for 1000Gbps ports */
    GT_U32  win100Mbps = 0;     /* window time for 100Gbps ports */
    GT_U32  win10Mbps = 0;      /* window time for 10Mbps ports */
    GT_U32  win10Gbps = 0;      /* window time for 10Gbps ports */
    GT_U32  dropMode;           /* packet drop mode */
    GT_U32  countType;          /* rate limit drop counter type */
    GT_U32 numOfBits;/* number of bit to set to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E );

    CPSS_NULL_PTR_CHECK_MAC(brgRateLimitPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK
        /* the granularity is configurable in register !
           for now use legacy 256 value */
        granularity = 256;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        /*  The granularity doesn't depend from PP core clock for DxCh3 and above */
        granularity = 256;
    }
    else
    {
        granularity = (256 * PRV_CPSS_PP_MAC(devNum)->baseCoreClock) / PRV_CPSS_PP_MAC(devNum)->coreClock;

    }

    if (brgRateLimitPtr->win1000Mbps >= (2 * granularity))
    {
        win1000Mbps = (brgRateLimitPtr->win1000Mbps / granularity) - 1;
    }

    /* In Cheetah the maximum value of the 1000MWindow field, in the Ingress Rate
       limit Configuration Register0 is 31. (FEr#140) */
    if((PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_RATE_LIMIT_GIG_COUNTER_BITS_NUM_WA_E)
        == GT_TRUE) && (win1000Mbps > 0x1f))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    else
    {
        if (win1000Mbps > 0x3f)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }


    if (brgRateLimitPtr->win100Mbps >= (2 * granularity))
    {
        win100Mbps = (brgRateLimitPtr->win100Mbps / granularity) - 1;
    }
    if (win100Mbps > 0x1ff)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(brgRateLimitPtr->win10Mbps >= (2 * granularity))
    {
        win10Mbps = (brgRateLimitPtr->win10Mbps / granularity) - 1;
    }
    if (win10Mbps > 0xfff)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if((brgRateLimitPtr->win10Gbps) >= (2 * granularity/10))
    {
        win10Gbps = (brgRateLimitPtr->win10Gbps * 10 / granularity) - 1;
    }

    if (win10Gbps > 0xfff)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
            ingrPortsRateLimitConfig.ingrRateLimitConfig0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeRateLimitConfigRegArray[0];
    }

    value = (win100Mbps << 12) | (win1000Mbps << 21);

    value |= win10Mbps ;

    /* Ingress Rate Limit Configuration Register0,
       fields <10MWindow>, <100MWindow>, <1000MWindow> */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 27, value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
            ingrPortsRateLimitConfig.ingrRateLimitConfig1;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeRateLimitConfigRegArray[1];
    }

    /* need to add support for 5 commands */
    CPSS_TBD_BOOKMARK_EARCH
    rc = prvCpssDxChHwDropModeToHwValueAndPacketCommandConvert(devNum,
            brgRateLimitPtr->dropMode,/*drop mode - SW value */
            NULL,       /*packet command - SW value  */
            &dropMode);/* hw value */
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(brgRateLimitPtr->rMode)
    {
    case CPSS_RATE_LIMIT_PCKT_BASED_E:
        countType = 1;
        break;
    case CPSS_RATE_LIMIT_BYTE_BASED_E:
        countType = 0;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            numOfBits = 28;
            value = win10Gbps | (countType << 24) | (dropMode << 25);
        }
        else
        {
            numOfBits = 16;
            value = win10Gbps | (countType << 12) | (dropMode << 13);
        }
    }
    else
    {
        numOfBits = 14;
        value = win10Gbps | (countType << 12) | (dropMode << 13);
    }


    /* Ingress Rate Limit Configuration Register1,
       fields <10GWindow>, <IngressRateLimitMode>, <IngressRateLimitDropMode> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, numOfBits, value);
}

/**
* @internal cpssDxChBrgGenRateLimitGlobalCfgSet function
* @endinternal
*
* @brief   Configures global ingress rate limit parameters - rate limiting mode,
*         Four configurable global time window periods are assigned to ports as
*         a function of their operating speed:
*         10 Gbps, 1 Gbps, 100 Mbps, and 10 Mbps.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported windows for DxCh devices:
*       1000 Mbps: range - 256-16384 uSec  granularity - 256 uSec
*       100 Mbps: range - 256-131072 uSec  granularity - 256 uSec
*       10  Mbps: range - 256-1048570 uSec granularity - 256 uSec
*       10  Gbps: range - 25.6-104857 uSec granularity - 25.6 uSec
*       For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitGlobalCfgSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC     *brgRateLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRateLimitGlobalCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, brgRateLimitPtr));

    rc = internal_cpssDxChBrgGenRateLimitGlobalCfgSet(devNum, brgRateLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, brgRateLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRateLimitGlobalCfgGet function
* @endinternal
*
* @brief   Get global ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
static GT_STATUS internal_cpssDxChBrgGenRateLimitGlobalCfgGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    *brgRateLimitPtr
)
{
    GT_U32  regAddr;            /* register address */
    GT_STATUS rc;               /* function return code */
    GT_U32 value;               /* value to write into register */
    GT_U32  granularity;        /* rate limit window granularity */
    GT_U32  win1000Mbps;        /* window time for 1000Gbps ports */
    GT_U32  win100Mbps;         /* window time for 100Gbps ports */
    GT_U32  win10Mbps;          /* window time for 10Mbps ports */
    GT_U32  win10Gbps;          /* window time for 10Gbps ports */
    GT_U32 numOfBits;/* number of bit to set to register */
    GT_U32 hwDropMode;          /* hw Drop Mode */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(brgRateLimitPtr);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK
        /* the granularity is configurable in register !
           for now use legacy 256 value */
        granularity = 256;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
    {
        /*  The granularity doesn't depend from PP core clock for DxCh3 and above */
        granularity = 256;
    }
    else
    {
        granularity = (256 * PRV_CPSS_PP_MAC(devNum)->baseCoreClock) / PRV_CPSS_PP_MAC(devNum)->coreClock;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
            ingrPortsRateLimitConfig.ingrRateLimitConfig0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeRateLimitConfigRegArray[0];
    }

    /* Ingress Rate Limit Configuration Register0,
       fields <10MWindow>, <100MWindow>, <1000MWindow> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 27, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    win10Mbps   = (value) & 0xfff;
    win100Mbps  = (value >> 12) & 0x1ff;
    win1000Mbps = (value >> 21) & 0x3f;

    brgRateLimitPtr->win10Mbps =   granularity + (win10Mbps * granularity);
    brgRateLimitPtr->win100Mbps =  granularity + (win100Mbps * granularity);
    brgRateLimitPtr->win1000Mbps = granularity + (win1000Mbps * granularity);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
            ingrPortsRateLimitConfig.ingrRateLimitConfig1;
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            numOfBits = 28;
        }
        else
        {
            numOfBits = 16;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeRateLimitConfigRegArray[1];
        numOfBits = 14;
    }

    /* Ingress Rate Limit Configuration Register1,
       fields <10GWindow>, <IngressRateLimitMode>, <IngressRateLimitDropMode> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, numOfBits, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    win10Gbps = (value) & 0xfff;

    brgRateLimitPtr->win10Gbps = (granularity/10) +
        (win10Gbps  * (granularity / 10));

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            brgRateLimitPtr->rMode = ((value >> 24) & 0x1) ?
                CPSS_RATE_LIMIT_PCKT_BASED_E : CPSS_RATE_LIMIT_BYTE_BASED_E;
            hwDropMode = (value >> 25) & 0x7;
        }
        else
        {
            brgRateLimitPtr->rMode = ((value >> 12) & 0x1) ?
                CPSS_RATE_LIMIT_PCKT_BASED_E : CPSS_RATE_LIMIT_BYTE_BASED_E;
            /* convert the 3 bits of command to packetCommand */
            hwDropMode = (value >> 13) & 0x7;
        }
    }
    else
    {
        brgRateLimitPtr->rMode = ((value >> 12) & 0x1) ?
            CPSS_RATE_LIMIT_PCKT_BASED_E : CPSS_RATE_LIMIT_BYTE_BASED_E;
        hwDropMode = (value >> 13) & 0x1;
    }

    /* need to add support for 5 commands */
    CPSS_TBD_BOOKMARK_EARCH
    /* convert HW value to drop mode */
    rc = prvCpssDxChHwHwValueToDropModeAndPacketCommandConvert(devNum,
            hwDropMode,/* hw value */
            NULL,   /*packet command - SW value  */
            &brgRateLimitPtr->dropMode);/*drop mode - SW value */
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenRateLimitGlobalCfgGet function
* @endinternal
*
* @brief   Get global ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitGlobalCfgGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    *brgRateLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRateLimitGlobalCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, brgRateLimitPtr));

    rc = internal_cpssDxChBrgGenRateLimitGlobalCfgGet(devNum, brgRateLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, brgRateLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortRateLimitSet function
* @endinternal
*
* @brief   Configures port ingress rate limit parameters
*         Each port maintains rate limits for unknown unicast packets,
*         known unicast packets, multicast packets and broadcast packets,
*         single configurable limit threshold value, and a single internal counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_OUT_OF_RANGE          - on out of range rate limit values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitSet
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     port,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC    *portGfgPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_STATUS rc;           /* function return code */
    GT_U32 value;           /* value to write into register */
    GT_U32 mask;            /* mask for register write */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  maxRateLimit;   /* maximal rate limit allowed value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, port);
    CPSS_NULL_PTR_CHECK_MAC(portGfgPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        maxRateLimit = BIT_22 - 1;
    }
    else
    {
        maxRateLimit = BIT_16 - 1;
    }

    if (portGfgPtr->rateLimit > maxRateLimit)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(port >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                port);
        }

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_ALDRIN2_BRIDGE_RATE_LIMIT_UC_WA_E) &&
            ((portGfgPtr->enableUcKnown == GT_TRUE) |
             (portGfgPtr->enableUcUnk == GT_TRUE)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                "This Aldrin2 revision does not support "
                "enableUcKnown/enableUcUnk features\n");
        }

        /* update all fields except <RateLimitDropCountEn>, <SYNRateLimEn> (bits #4, #5) */
        mask = (GT_U32)-1 ^ (BIT_4 | BIT_5);
        value = 0;
        value |= /*BcRateLimEn*/
            ((portGfgPtr->enableBc == GT_TRUE) ? 1 : 0);
        value |= /*UcRateLimEn*/
            ((portGfgPtr->enableUcKnown == GT_TRUE) ? 1 : 0) << 2;
        value |= /*UnkUcRateLimEn*/
            ((portGfgPtr->enableUcUnk == GT_TRUE) ? 1 : 0) << 3;
        value |= /*RegisteredMcRateLimEn*/
            ((portGfgPtr->enableMcReg == GT_TRUE) ? 1 : 0) << 1;
        value |= /*UnregisteredMcRateLimEn*/
            ((portGfgPtr->enableMc == GT_TRUE) ? 1 : 0) << 6;
        value |= portGfgPtr->rateLimit << 7;

        /* update the entry */
        rc = prvCpssDxChWriteTableEntryMasked(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
                                        port,/* global physical port */
                                        &value,
                                        &mask);
        return rc;
    }
    else
    {
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, port);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.portControl[port];

    value =
        ((portGfgPtr->enableBc == GT_TRUE) ? 1 : 0);
    value |=
        ((portGfgPtr->enableUcKnown == GT_TRUE) ? 1 : 0) << 2;
    value |=
        ((portGfgPtr->enableUcUnk == GT_TRUE) ? 1 : 0) << 3;

    if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat2 */
        value |=
            ((portGfgPtr->enableMcReg == GT_TRUE) ? 1 : 0) << 1;
    }
    else
    {
        value |=
            ((portGfgPtr->enableMc == GT_TRUE) ? 1 : 0) << 1;
    }

    /* Ingress Port<n> Bridge Configuration Register0,
       fields <BcRateLimEn>, <McRateLimEn>, <UcRateLimEn>, <UnkUcRateLimEn> */
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 2, 4, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr += 0x10;
    value = portGfgPtr->rateLimit << 5;
    mask = 0x001FFFE0;

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* for xCat when rate limit is enabled on MC setting enable for unreg MC */
        /* is made explicitly */
        value |= ((portGfgPtr->enableMc == GT_TRUE) ? 1 : 0) << 24;
        mask |= ( 1 << 24 );
    }


    /* Ingress Port<n> Bridge Configuration Register1,
       fields <IngressLimit> and <UnregisteredMcRateLimEn> */
    return prvCpssHwPpPortGroupWriteRegBitMask(devNum, portGroupId,regAddr,
                                                  mask, value);
}
}

/**
* @internal cpssDxChBrgGenPortRateLimitSet function
* @endinternal
*
* @brief   Configures port ingress rate limit parameters
*         Each port maintains rate limits for unknown unicast packets,
*         known unicast packets, multicast packets and broadcast packets,
*         single configurable limit threshold value, and a single internal counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_OUT_OF_RANGE          - on out of range rate limit values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitSet
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     port,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC    *portGfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port, portGfgPtr));

    rc = internal_cpssDxChBrgGenPortRateLimitSet(devNum, port, portGfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port, portGfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortRateLimitGet function
* @endinternal
*
* @brief   Get port ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitGet
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     port,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC   *portGfgPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32 value;           /* value to read from register */
    GT_STATUS rc;           /* function return code */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, port);
    CPSS_NULL_PTR_CHECK_MAC(portGfgPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(port >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                port);
        }
        /* get the entry */
        rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
                                        port,/* global physical port */
                                        &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        portGfgPtr->enableBc = /*BcRateLimEn*/
            (value & 0x1) ? GT_TRUE : GT_FALSE;
        portGfgPtr->enableUcKnown = /*UcRateLimEn*/
            ((value >> 2) & 0x1) ? GT_TRUE : GT_FALSE;
        portGfgPtr->enableUcUnk = /*UnkUcRateLimEn*/
            ((value >> 3) & 0x1) ? GT_TRUE : GT_FALSE;
        portGfgPtr->enableMcReg = /*RegisteredMcRateLimEn*/
            ((value >> 1) & 0x1) ? GT_TRUE : GT_FALSE;
        portGfgPtr->enableMc = /*UnregisteredMcRateLimEn*/
            ((value >> 6) & 0x1) ? GT_TRUE : GT_FALSE;
        portGfgPtr->rateLimit = (value >> 7) & (BIT_22 - 1);

    }
    else
    {
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, port);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.portControl[port];

    /* Ingress Port<n> Bridge Configuration Register0,
       fields <BcRateLimEn>, <McRateLimEn>, <UcRateLimEn>, <UnkUcRateLimEn> */
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 2, 4, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    portGfgPtr->enableBc =
        (value & 0x1) ? GT_TRUE : GT_FALSE;
    portGfgPtr->enableUcKnown =
        ((value >> 2) & 0x1) ? GT_TRUE : GT_FALSE;
    portGfgPtr->enableUcUnk =
        ((value >> 3) & 0x1) ? GT_TRUE : GT_FALSE;

    if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        portGfgPtr->enableMcReg =
            ((value >> 1) & 0x1) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        portGfgPtr->enableMc =
            ((value >> 1) & 0x1) ? GT_TRUE : GT_FALSE;
    }

    regAddr += 0x10;
    /* Ingress Port<n> Bridge Configuration Register1,
       field <IngressLimit> */
    rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    portGfgPtr->rateLimit = (value >> 5) & 0xFFFF;

    if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        portGfgPtr->enableMc = BIT2BOOL_MAC((value >> 24) & 0x1);
    }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenPortRateLimitGet function
* @endinternal
*
* @brief   Get port ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitGet
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     port,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC   *portGfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port, portGfgPtr));

    rc = internal_cpssDxChBrgGenPortRateLimitGet(devNum, port, portGfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port, portGfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortRateLimitTcpSynSet function
* @endinternal
*
* @brief   Enable or disable TCP SYN packets' rate limit for a port.
*         Value of rate limit is configured by cpssDxChBrgGenPortRateLimitSet and
*         cpssDxChBrgGenRateLimitGlobalCfgSet
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - enable/disable TCP SYN packets rate limit.
*                                      GT_TRUE - enable rate limit, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitTcpSynSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     port,
    IN GT_BOOL                  enable
)
{
    GT_U32  regAddr;            /* register address */
    GT_U32  value;              /* value to write from register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_LION_E | CPSS_LION2_E );

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, port);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(port >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                port);
        }
        /* set the field <SYNRateLimEn> */
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
                                        port,/* global physical port */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        5,/*start bit*/
                                        1,/*num of bits*/
                                        BOOL2BIT_MAC(enable));
    }
    else
    {
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, port);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.portControl1[port];

    value = BOOL2BIT_MAC(enable);

    /* Ingress Port<n> Bridge Configuration Register1, field <SYNRateLimEn> */
    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 23, 1, value);
}
}

/**
* @internal cpssDxChBrgGenPortRateLimitTcpSynSet function
* @endinternal
*
* @brief   Enable or disable TCP SYN packets' rate limit for a port.
*         Value of rate limit is configured by cpssDxChBrgGenPortRateLimitSet and
*         cpssDxChBrgGenRateLimitGlobalCfgSet
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - enable/disable TCP SYN packets rate limit.
*                                      GT_TRUE - enable rate limit, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitTcpSynSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     port,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitTcpSynSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port, enable));

    rc = internal_cpssDxChBrgGenPortRateLimitTcpSynSet(devNum, port, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgGenPortRateLimitTcpSynGet function
* @endinternal
*
* @brief   Get the status of TCP SYN packets' rate limit for a port (enable/disable).
*         Value of rate limit can be read by cpssDxChBrgGenPortRateLimitGet and
*         cpssDxChBrgGenRateLimitGlobalCfgGet.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] enablePtr                - pointer to TCP SYN packets rate limit status.
*                                      GT_TRUE - enable rate limit, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitTcpSynGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    port,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32  regAddr;            /* register address */
    GT_U32  value;              /* value to write from register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_STATUS rc;           /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, port);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(port >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                port);
        }
        /* get the field <SYNRateLimEn> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
                                        port,/* global physical port */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        5,/*start bit*/
                                        1,/*num of bits*/
                                        &value);
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, port);

        /* Ingress Port<n> Bridge Configuration Register1, field <SYNRateLimEn> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.portControl1[port];

        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 23, 1, &value);
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgGenPortRateLimitTcpSynGet function
* @endinternal
*
* @brief   Get the status of TCP SYN packets' rate limit for a port (enable/disable).
*         Value of rate limit can be read by cpssDxChBrgGenPortRateLimitGet and
*         cpssDxChBrgGenRateLimitGlobalCfgGet.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] enablePtr                - pointer to TCP SYN packets rate limit status.
*                                      GT_TRUE - enable rate limit, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitTcpSynGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    port,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitTcpSynGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, port, enablePtr));

    rc = internal_cpssDxChBrgGenPortRateLimitTcpSynGet(devNum, port, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, port, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenUdpBcDestPortCfgSet function
* @endinternal
*
* @brief   Configure UDP Broadcast Destination Port configuration table.
*         It's possible to configure several UDP destination ports with their Trap
*         or Mirror to CPU command and CPU code. See datasheet of device for maximal
*         number of UDP destination ports, which may be configured. The feature may
*         be enabled or disabled per VLAN by cpssDxChBrgVlanUdpBcPktsToCpuEnable
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*                                      (APPLICABLE RANGES: 0..11)
* @param[in] udpPortNum               - UDP destination port number
* @param[in] cpuCode                  - CPU code for packets trapped or mirrored by the feature.
*                                      Acceptable CPU codes are: CPSS_NET_UDP_BC_MIRROR_TRAP0_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP1_E, CPSS_NET_UDP_BC_MIRROR_TRAP2_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP3_E
* @param[in] cmd                      - packet command for UDP Broadcast packets
*                                      either CPSS_DXCH_BRG_GEN_CNTRL_TRAP_E
*                                      or CPSS_DXCH_BRG_GEN_CNTRL_MIRROR_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or CPU code or cmd or
*                                       entryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenUdpBcDestPortCfgSet
(
   IN GT_U8                     devNum,
   IN GT_U32                    entryIndex,
   IN GT_U16                    udpPortNum,
   IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode,
   IN CPSS_PACKET_CMD_ENT       cmd
)
{
    GT_U32  value;                  /* value to write from register */
    GT_U32  regAddr;                /* register address */
    GT_U32  cpuCodeIndex;           /* cpu code index */
    GT_U32  hwPacketCmd;            /* packet command to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);


    if(entryIndex >= 12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(cpuCode)
    {
        case CPSS_NET_UDP_BC_MIRROR_TRAP0_E:
             cpuCodeIndex = 0;
             break;

        case CPSS_NET_UDP_BC_MIRROR_TRAP1_E:
             cpuCodeIndex = 1;
             break;

        case CPSS_NET_UDP_BC_MIRROR_TRAP2_E:
             cpuCodeIndex = 2;
             break;

        case CPSS_NET_UDP_BC_MIRROR_TRAP3_E:
             cpuCodeIndex = 3;
             break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(cmd)
    {

        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
             hwPacketCmd = 0;
             break;

        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
             hwPacketCmd = 1;
             break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*          Port        Valid        Packet Cmd            CPU Code Index */
    value = udpPortNum | (1 << 16) | (hwPacketCmd << 17) | (cpuCodeIndex << 18);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    UDPBcMirrorTrapUDPRelayConfig.UDPBcDestinationPortConfigTable[entryIndex];
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.udpBcDestPortConfigReg[entryIndex];
    }

    /* UDP Broadcast Destination Port<n> Configuration Table,
       fields <UDPDestPort>, <UDPDestPortValid>, UDPDestPortCmd,
       <UDPDestPortCPUCodeIndex>*/
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 20, value);
}

/**
* @internal cpssDxChBrgGenUdpBcDestPortCfgSet function
* @endinternal
*
* @brief   Configure UDP Broadcast Destination Port configuration table.
*         It's possible to configure several UDP destination ports with their Trap
*         or Mirror to CPU command and CPU code. See datasheet of device for maximal
*         number of UDP destination ports, which may be configured. The feature may
*         be enabled or disabled per VLAN by cpssDxChBrgVlanUdpBcPktsToCpuEnable
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*                                      (APPLICABLE RANGES: 0..11)
* @param[in] udpPortNum               - UDP destination port number
* @param[in] cpuCode                  - CPU code for packets trapped or mirrored by the feature.
*                                      Acceptable CPU codes are: CPSS_NET_UDP_BC_MIRROR_TRAP0_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP1_E, CPSS_NET_UDP_BC_MIRROR_TRAP2_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP3_E
* @param[in] cmd                      - packet command for UDP Broadcast packets
*                                      either CPSS_DXCH_BRG_GEN_CNTRL_TRAP_E
*                                      or CPSS_DXCH_BRG_GEN_CNTRL_MIRROR_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or CPU code or cmd or
*                                       entryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgSet
(
   IN GT_U8                     devNum,
   IN GT_U32                    entryIndex,
   IN GT_U16                    udpPortNum,
   IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode,
   IN CPSS_PACKET_CMD_ENT       cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenUdpBcDestPortCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, udpPortNum, cpuCode, cmd));

    rc = internal_cpssDxChBrgGenUdpBcDestPortCfgSet(devNum, entryIndex, udpPortNum, cpuCode, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, udpPortNum, cpuCode, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenUdpBcDestPortCfgGet function
* @endinternal
*
* @brief   Get UDP Broadcast Destination Port configuration table.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*                                      (APPLICABLE RANGES: 0..11)
*
* @param[out] validPtr                 - (pointer to) entry validity:
*                                      GT_TRUE  - valid entry,
*                                      GT_FALSE - invalid entry.
* @param[out] udpPortNumPtr            - (pointer to) UDP destination port number
* @param[out] cpuCodePtr               - (pointer to) CPU code for packets trapped or mirrored
*                                      by the feature. Acceptable CPU codes are:
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP0_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP1_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP2_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP3_E
* @param[out] cmdPtr                   - (pointer to) packet command for UDP Broadcast packets
*                                      either CPSS_DXCH_BRG_GEN_CNTRL_TRAP_E
*                                      or CPSS_DXCH_BRG_GEN_CNTRL_MIRROR_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or entryIndex > 11
* @retval GT_HW_ERROR              - failed to read from hw.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenUdpBcDestPortCfgGet
(
   IN GT_U8                     devNum,
   IN GT_U32                    entryIndex,
   OUT GT_BOOL                  *validPtr,
   OUT GT_U16                   *udpPortNumPtr,
   OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr,
   OUT CPSS_PACKET_CMD_ENT      *cmdPtr
)
{
    GT_STATUS rc;                   /* return code */
    GT_U32  regAddr;                /* register address */
    GT_U32  regData;                /* register data    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(udpPortNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);
    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    if(entryIndex >= 12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    UDPBcMirrorTrapUDPRelayConfig.UDPBcDestinationPortConfigTable[entryIndex];
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.udpBcDestPortConfigReg[entryIndex];
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &regData);
    if ( GT_OK != rc)
    {
        return rc;
    }

    *validPtr = BIT2BOOL_MAC( U32_GET_FIELD_MAC(regData, 16, 1) );

    *udpPortNumPtr = (GT_U16)U32_GET_FIELD_MAC(regData, 0, 16);

    switch (U32_GET_FIELD_MAC(regData, 18, 2))
    {
        case 0: *cpuCodePtr = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;
                break;

        case 1: *cpuCodePtr = CPSS_NET_UDP_BC_MIRROR_TRAP1_E;
                break;

        case 2: *cpuCodePtr = CPSS_NET_UDP_BC_MIRROR_TRAP2_E;
                break;

        case 3: *cpuCodePtr = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;
                break;

        default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *cmdPtr = ( U32_GET_FIELD_MAC(regData, 17, 1) == 0 ) ?
                CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
                CPSS_PACKET_CMD_TRAP_TO_CPU_E;

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenUdpBcDestPortCfgGet function
* @endinternal
*
* @brief   Get UDP Broadcast Destination Port configuration table.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*                                      (APPLICABLE RANGES: 0..11)
*
* @param[out] validPtr                 - (pointer to) entry validity:
*                                      GT_TRUE  - valid entry,
*                                      GT_FALSE - invalid entry.
* @param[out] udpPortNumPtr            - (pointer to) UDP destination port number
* @param[out] cpuCodePtr               - (pointer to) CPU code for packets trapped or mirrored
*                                      by the feature. Acceptable CPU codes are:
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP0_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP1_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP2_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP3_E
* @param[out] cmdPtr                   - (pointer to) packet command for UDP Broadcast packets
*                                      either CPSS_DXCH_BRG_GEN_CNTRL_TRAP_E
*                                      or CPSS_DXCH_BRG_GEN_CNTRL_MIRROR_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or entryIndex > 11
* @retval GT_HW_ERROR              - failed to read from hw.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgGet
(
   IN GT_U8                     devNum,
   IN GT_U32                    entryIndex,
   OUT GT_BOOL                  *validPtr,
   OUT GT_U16                   *udpPortNumPtr,
   OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr,
   OUT CPSS_PACKET_CMD_ENT      *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenUdpBcDestPortCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, validPtr, udpPortNumPtr, cpuCodePtr, cmdPtr));

    rc = internal_cpssDxChBrgGenUdpBcDestPortCfgGet(devNum, entryIndex, validPtr, udpPortNumPtr, cpuCodePtr, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, validPtr, udpPortNumPtr, cpuCodePtr, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenUdpBcDestPortCfgInvalidate function
* @endinternal
*
* @brief   Invalidate UDP Broadcast Destination Port configuration table.
*         All fields in entry will be reset.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or intryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenUdpBcDestPortCfgInvalidate
(
   IN GT_U8                                     devNum,
   IN GT_U32                                    entryIndex
)
{
    GT_U32  value;                  /* value to write from register */
    GT_U32  regAddr;                /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    if(entryIndex > 11)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    UDPBcMirrorTrapUDPRelayConfig.UDPBcDestinationPortConfigTable[entryIndex];
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.udpBcDestPortConfigReg[entryIndex];
    }

    value = 0;

    /* UDP Broadcast Destination Port<n> Configuration Table */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 20, value);
}

/**
* @internal cpssDxChBrgGenUdpBcDestPortCfgInvalidate function
* @endinternal
*
* @brief   Invalidate UDP Broadcast Destination Port configuration table.
*         All fields in entry will be reset.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or intryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgInvalidate
(
   IN GT_U8                                     devNum,
   IN GT_U32                                    entryIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenUdpBcDestPortCfgInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex));

    rc = internal_cpssDxChBrgGenUdpBcDestPortCfgInvalidate(devNum, entryIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenBpduTrapEnableSet function
* @endinternal
*
* @brief   Enable or disable trapping of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - trap packets to CPU with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 on a ingress port whose span state
*                                      is not disabled.
*                                      GT_FALSE - packets with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 are not affected by ingress port's span state
*                                      and forwarded as standard Multicast frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenBpduTrapEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32  regAddr;                    /* register address */
    GT_U32  value;                      /* value to write from register */
    GT_U32  bitIndex;                   /* bit index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        bitIndex = 10;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        bridgeRegs.bridgeGlobalConfigRegArray[0];
        bitIndex = 21;
    }

    value = BOOL2BIT_MAC(enable);

    /* Bridge Global Configuration Register0, field <CustomerBPDUTrapEn> */
    return prvCpssHwPpSetRegField(devNum, regAddr, bitIndex, 1, value);
}

/**
* @internal cpssDxChBrgGenBpduTrapEnableSet function
* @endinternal
*
* @brief   Enable or disable trapping of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - trap packets to CPU with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 on a ingress port whose span state
*                                      is not disabled.
*                                      GT_FALSE - packets with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 are not affected by ingress port's span state
*                                      and forwarded as standard Multicast frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenBpduTrapEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenBpduTrapEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgGenBpduTrapEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenBpduTrapEnableGet function
* @endinternal
*
* @brief   Get trapping status of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - trap packets to CPU with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 on a ingress port whose span state
*                                      is not disabled.
*                                      GT_FALSE - packets with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 are not affected by ingress port's span state
*                                      and forwarded as standard Multicast frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenBpduTrapEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;                    /* register address */
    GT_U32  value;                      /* value to write from register */
    GT_U32  bitIndex;                   /* bit index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        bitIndex = 10;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        bridgeRegs.bridgeGlobalConfigRegArray[0];
        bitIndex = 21;
    }

    /* Bridge Global Configuration Register0, field <CustomerBPDUTrapEn> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitIndex, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenBpduTrapEnableGet function
* @endinternal
*
* @brief   Get trapping status of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - trap packets to CPU with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 on a ingress port whose span state
*                                      is not disabled.
*                                      GT_FALSE - packets with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 are not affected by ingress port's span state
*                                      and forwarded as standard Multicast frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenBpduTrapEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenBpduTrapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgGenBpduTrapEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenArpTrapEnable function
* @endinternal
*
* @brief   Enable/disable trapping ARP Broadcast packets to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] enable                   - GT_TRUE - ARP Broadcast packets received on this port
*                                      are trapped to the CPU.
*                                      GT_FALSE - ARP Broadcast packets received on this port
*                                      are not trapped to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenArpTrapEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc=GT_OK;
    GT_U32      regAddr;     /* register address */
    GT_U32      value;       /* value to write into register */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <ARP_BC_TRAP_EN>*/
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_ARP_BC_TRAP_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <ARPBCTrapEn> */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 13, 1, value);
    }

    return rc;
}

/**
* @internal cpssDxChBrgGenArpTrapEnable function
* @endinternal
*
* @brief   Enable/disable trapping ARP Broadcast packets to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] enable                   - GT_TRUE - ARP Broadcast packets received on this port
*                                      are trapped to the CPU.
*                                      GT_FALSE - ARP Broadcast packets received on this port
*                                      are not trapped to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenArpTrapEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenArpTrapEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenArpTrapEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenArpTrapEnableGet function
* @endinternal
*
* @brief   Get trapping ARP Broadcast packets to the CPU status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                -  pointer to trapping ARP Broadcast packets status
*                                      GT_TRUE - ARP Broadcast packets received on this port
*                                      are trapped to the CPU.
*                                      GT_FALSE - ARP Broadcast packets received on this port
*                                      are not trapped to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenArpTrapEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to read from register */
    GT_U32 portGroupId; /* the port group Id - support multi-port-groups device */
    GT_STATUS rc;       /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <ARP_BC_TRAP_EN>*/
         rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_ARP_BC_TRAP_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum);

        /* Ingress Port<n> Bridge Configuration Register0, field <ARPBCTrapEn> */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 13, 1, &value);
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;

}

/**
* @internal cpssDxChBrgGenArpTrapEnableGet function
* @endinternal
*
* @brief   Get trapping ARP Broadcast packets to the CPU status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                -  pointer to trapping ARP Broadcast packets status
*                                      GT_TRUE - ARP Broadcast packets received on this port
*                                      are trapped to the CPU.
*                                      GT_FALSE - ARP Broadcast packets received on this port
*                                      are not trapped to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenArpTrapEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenArpTrapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenArpTrapEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
static GT_STATUS internal_cpssDxChBrgGenDropIpMcModeSet
(
    IN GT_U8                    devNum,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* need to add support for 5 commands */
    CPSS_TBD_BOOKMARK_EARCH
    /* convert drop mode to HW value */
    rc = prvCpssDxChHwDropModeToHwValueAndPacketCommandConvert(devNum,
            mode,/*drop mode - SW value */
            NULL,   /*packet command - SW value  */
            &value);/* hw value */
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        return prvCpssHwPpSetRegField(devNum, regAddr, 6, 3, value);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        bridgeRegs.bridgeGlobalConfigRegArray[1];

        /* Bridge Global Configuration Register1, field <IP MC Drop Mode> */
        return prvCpssHwPpSetRegField(devNum, regAddr, 28, 1, value);
    }


}

/**
* @internal cpssDxChBrgGenDropIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropIpMcModeSet
(
    IN GT_U8                    devNum,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropIpMcModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgGenDropIpMcModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
static GT_STATUS internal_cpssDxChBrgGenDropIpMcModeGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    value;       /* value to read from register */
    GT_STATUS rc;          /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 3, &value);
    }
    else
    {
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[1];

    /* Bridge Global Configuration Register1, field <IP MC Drop Mode> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 28, 1, &value);
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    /* need to add support for 5 commands */
    CPSS_TBD_BOOKMARK_EARCH
    /* convert HW value to drop mode */
    rc = prvCpssDxChHwHwValueToDropModeAndPacketCommandConvert(devNum,
            value,/* hw value */
            NULL,   /*packet command - SW value  */
            modePtr);/*drop mode - SW value */
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal cpssDxChBrgGenDropIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropIpMcModeGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropIpMcModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgGenDropIpMcModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgGenDropNonIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
static GT_STATUS internal_cpssDxChBrgGenDropNonIpMcModeSet
(
    IN GT_U8                    devNum,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* need to add support for 5 commands */
    CPSS_TBD_BOOKMARK_EARCH
    /* convert drop mode to HW value */
    rc = prvCpssDxChHwDropModeToHwValueAndPacketCommandConvert(devNum,
            mode,/*drop mode - SW value */
            NULL,   /*packet command - SW value  */
            &value);/* hw value */
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        return prvCpssHwPpSetRegField(devNum, regAddr, 9, 3, value);
    }
    else
    {
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[1];

    /* Bridge Global Configuration Register1, field <Non-IP MC Drop Mode> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 29, 1, value);
    }
}

/**
* @internal cpssDxChBrgGenDropNonIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcModeSet
(
    IN GT_U8                    devNum,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropNonIpMcModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgGenDropNonIpMcModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenDropNonIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
static GT_STATUS internal_cpssDxChBrgGenDropNonIpMcModeGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    value;       /* value to read from register */
    GT_STATUS rc;          /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeCommandConfig1;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 9, 3, &value);
    }
    else
    {
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeGlobalConfigRegArray[1];

    /* Bridge Global Configuration Register1, field <Non-IP MC Drop Mode> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 29, 1, &value);
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    /* need to add support for 5 commands */
    CPSS_TBD_BOOKMARK_EARCH
    /* convert HW value to drop mode */
    rc = prvCpssDxChHwHwValueToDropModeAndPacketCommandConvert(devNum,
            value,/* hw value */
            NULL,   /*packet command - SW value  */
            modePtr);/*drop mode - SW value */
    if (rc != GT_OK)
    {
        return rc;
    }


    return GT_OK;

}

/**
* @internal cpssDxChBrgGenDropNonIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcModeGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenDropNonIpMcModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgGenDropNonIpMcModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenCfiRelayEnableSet function
* @endinternal
*
* @brief   Enable / Disable CFI bit relay.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenCfiRelayEnableSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 offset;      /* field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
        offset = 28;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;
        offset = 28;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
        offset = 22;
    }

    value = BOOL2BIT_MAC(enable);

    /* Enable / Disable CFI bit relay */
    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 1, value);

}

/**
* @internal cpssDxChBrgGenCfiRelayEnableSet function
* @endinternal
*
* @brief   Enable / Disable CFI bit relay.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenCfiRelayEnableSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenCfiRelayEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgGenCfiRelayEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenCfiRelayEnableGet function
* @endinternal
*
* @brief   Get CFI bit relay status.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                -  pointer to CFI bit relay status.
*                                      - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenCfiRelayEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    value;       /* value to read from register */
    GT_STATUS rc;          /* return status */
    GT_U32    offset;      /* field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;
        offset = 28;
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.ttiUnitConfig;
        offset = 28;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
        offset = 22;
    }

    /* Get CFI bit relay status */
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, offset, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;

}

/**
* @internal cpssDxChBrgGenCfiRelayEnableGet function
* @endinternal
*
* @brief   Get CFI bit relay status.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                -  pointer to CFI bit relay status.
*                                      - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenCfiRelayEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenCfiRelayEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgGenCfiRelayEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet function
* @endinternal
*
* @brief   Select the IEEE Reserved Multicast profile (table) associated with port
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
* @param[in] profileIndex             - profile index. The parameter defines profile (table
*                                      number) for the 256 protocols (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_OUT_OF_RANGE          - for profileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               profileIndex
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;     /* register address */
    GT_U32      field;       /* register field offset */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32       localPort;   /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (profileIndex > 7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* set the <IEEE_RSVD_MC_TABLE_SEL> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_IEEE_RSVD_MC_TABLE_SEL_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        profileIndex);
    }
    else
    {
    if (profileIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        if (localPort < 16)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            bridgeRegs.ieeeReservedMcastProfileSelect0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            bridgeRegs.ieeeReservedMcastProfileSelect1;
        }

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            field = 30;
        }
        else
        {
            field = 2 * (localPort % 16);
        }

        /* set IEEE table select0/1 Register */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, field, 2, profileIndex);
    }

    return rc;
}

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet function
* @endinternal
*
* @brief   Select the IEEE Reserved Multicast profile (table) associated with port
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
* @param[in] profileIndex             - profile index. The parameter defines profile (table
*                                      number) for the 256 protocols (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_OUT_OF_RANGE          - for profileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               profileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndex));

    rc = internal_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(devNum, portNum, profileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet function
* @endinternal
*
* @brief   Get the IEEE Reserved Multicast profile (table) associated with port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*
* @param[out] profileIndexPtr          - (pointer to) profile index. The parameter
*                                      defines profile (table number) for the 256 protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or profileId
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    OUT GT_U32              *profileIndexPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32   regAddr;    /* register address */
    GT_U32   field;      /* register field offset */
    GT_U32   portGroupId;/*the port group Id - support multi-port-groups device*/
    GT_U32    localPort;  /* local port - support multi-port-groups device */
    GT_U32   value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* read from Bridge-Ingress-ePort table */
        /* get the <IEEE_RSVD_MC_TABLE_SEL> */
         rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_IEEE_RSVD_MC_TABLE_SEL_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &value);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        if (localPort < 16)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            bridgeRegs.ieeeReservedMcastProfileSelect0;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            bridgeRegs.ieeeReservedMcastProfileSelect1;
        }

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            field = 30;
        }
        else
        {
            field = 2 * (localPort % 16);
        }

        /* get IEEE table select0/1 Register */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, field, 2, &value);
    }

    if(rc != GT_OK)
        return rc;

    *profileIndexPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet function
* @endinternal
*
* @brief   Get the IEEE Reserved Multicast profile (table) associated with port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*
* @param[out] profileIndexPtr          - (pointer to) profile index. The parameter
*                                      defines profile (table number) for the 256 protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or profileId
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    OUT GT_U32              *profileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndexPtr));

    rc = internal_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(devNum, portNum, profileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet function
* @endinternal
*
* @brief   Enable/disable dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_FALSE -
*                                      untagged packets with size more then 1518 bytes not dropped
*                                      GT_TRUE -
*                                      untagged packets with size more then 1518 bytes dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      bitNum; /* number of "OverSizeUntaggedPacketsFilterEn" bit */
                        /* in VLAN QoS entry                               */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to post-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            ((enable == GT_FALSE) ? 0 : 1));
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat and above devices */
            bitNum = 9;
        }
        else
        {
            /* DxCh3 devices */
            bitNum = 14;
        }

        rc = prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                           portNum,
                                           0 /*fieldWordNum*/,
                                           bitNum /*fieldOffset*/,
                                           1 /*fieldLength*/,
                                           ((enable == GT_FALSE) ? 0 : 1) /*fieldValue*/);
    }
    return rc;

}

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet function
* @endinternal
*
* @brief   Enable/disable dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_FALSE -
*                                      untagged packets with size more then 1518 bytes not dropped
*                                      GT_TRUE -
*                                      untagged packets with size more then 1518 bytes dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet function
* @endinternal
*
* @brief   Get Enable/disable status of dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE -
*                                      untagged packets with size more then 1518 bytes not dropped
*                                      GT_TRUE -
*                                      untagged packets with size more then 1518 bytes dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32    hwValue; /* harware value                                   */
    GT_STATUS rc;      /* return code                                     */
    GT_U32    bitNum;  /* number of "OverSizeUntaggedPacketsFilterEn" bit */
                       /* in VLAN QoS entry                               */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to post-tti-lookup-ingress-eport table */
         rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwValue);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat and above devices */
            bitNum = 9;
        }
        else
        {
            /* DxCh3 devices */
            bitNum = 14;
        }

        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,
                                            0 /*fieldWordNum*/,
                                            bitNum /*fieldOffset*/,
                                            1 /*fieldLength*/,
                                            &hwValue /*fieldValue*/);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (hwValue == 0) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet function
* @endinternal
*
* @brief   Get Enable/disable status of dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE -
*                                      untagged packets with size more then 1518 bytes not dropped
*                                      GT_TRUE -
*                                      untagged packets with size more then 1518 bytes dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet function
* @endinternal
*
* @brief   Set the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable/disable filtering of untagged packets with length bigger than
*       1518 bytes on ePort, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
*
*/
static GT_STATUS internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_U32      regAddr;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,command);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt;
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 3, value);
}

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet function
* @endinternal
*
* @brief   Set the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable/disable filtering of untagged packets with length bigger than
*       1518 bytes on ePort, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
*
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, command));

    rc = internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(devNum, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet function
* @endinternal
*
* @brief   Get the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the filtering status on ePort of untagged packets with length bigger
*       than 1518 bytes, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
*
*/
static GT_STATUS internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 3, &value);
    if (rc == GT_OK)
    {
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr,value);
    }
    return rc;
}

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet function
* @endinternal
*
* @brief   Get the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the filtering status on ePort of untagged packets with length bigger
*       than 1518 bytes, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
*
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, commandPtr));

    rc = internal_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(devNum, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRateLimitSpeedCfgEnableSet function
* @endinternal
*
* @brief   Enable/Disable the port speed configuration that will be used by the
*         Bridge ingress rate limiter, regardless of the actual speed in which
*         a port actually works.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE -  Port speed for bridge rate limit
*                                      calculations is according to the actual
*                                      port speed.
*                                      GT_TRUE - Port speed for bridge rate limit
*                                      calculations is taken from a per-port configuration
*                                      cpssDxChBrgGenPortRateLimitSpeedGranularitySet()
*                                      and NOT from the port actual speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
static GT_STATUS internal_cpssDxChBrgGenRateLimitSpeedCfgEnableSet
(
    IN GT_U8      devNum,
    IN GT_BOOL    enable
)
{
    GT_U32  regAddr;  /* register address */
    GT_U32  regValue; /* register value */
    GT_U32  bitNum;   /* bit number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_TBD_BOOKMARK/* the Bobcat2, Caelum, Bobcat3 support 'per port' configuration and not 'global' configuration */
    /* so need API 'per port' */

    if(PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat2 and above devices */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.stackSpeedGranularity;
        bitNum  = 31;
    }
    else
    {
        /* xCat & xCat3 devices */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.dft1Reg;
        bitNum  = 25;
    }

    regValue = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, bitNum, 1, regValue);
}

/**
* @internal cpssDxChBrgGenRateLimitSpeedCfgEnableSet function
* @endinternal
*
* @brief   Enable/Disable the port speed configuration that will be used by the
*         Bridge ingress rate limiter, regardless of the actual speed in which
*         a port actually works.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE -  Port speed for bridge rate limit
*                                      calculations is according to the actual
*                                      port speed.
*                                      GT_TRUE - Port speed for bridge rate limit
*                                      calculations is taken from a per-port configuration
*                                      cpssDxChBrgGenPortRateLimitSpeedGranularitySet()
*                                      and NOT from the port actual speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitSpeedCfgEnableSet
(
    IN GT_U8      devNum,
    IN GT_BOOL    enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRateLimitSpeedCfgEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgGenRateLimitSpeedCfgEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRateLimitSpeedCfgEnableGet function
* @endinternal
*
* @brief   Get the status of the port speed configuration (Enabled/Disabled) that
*         will be used by the Bridge ingress rate limiter, regardless of the actual
*         speed in which a port actually works.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE -  Port speed for bridge rate limit
*                                      calculations is according to the actual
*                                      port speed.
*                                      GT_TRUE -   Port speed for bridge rate limit
*                                      calculations is taken from a per-port configuration
*                                      cpssDxChBrgGenPortRateLimitSpeedGranularitySet()
*                                      and NOT from the port actual speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
static GT_STATUS internal_cpssDxChBrgGenRateLimitSpeedCfgEnableGet
(
    IN  GT_U8      devNum,
    OUT GT_BOOL    *enablePtr
)
{

    GT_U32  regAddr;  /* register address */
    GT_U32  regValue; /* register value */
    GT_U32  bitNum;   /* bit number */
    GT_STATUS rc;     /* returned status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    CPSS_TBD_BOOKMARK/* the Bobcat2, Caelum, Bobcat3 support 'per port' configuration and not 'global' configuration */
    /* so need API 'per port' */

    if(PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat2 and above devices */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.stackSpeedGranularity;
        bitNum  = 31;
    }
    else
    {
        /* xCat & xCat3 devices */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.dft1Reg;
        bitNum  = 25;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitNum, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenRateLimitSpeedCfgEnableGet function
* @endinternal
*
* @brief   Get the status of the port speed configuration (Enabled/Disabled) that
*         will be used by the Bridge ingress rate limiter, regardless of the actual
*         speed in which a port actually works.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE -  Port speed for bridge rate limit
*                                      calculations is according to the actual
*                                      port speed.
*                                      GT_TRUE -   Port speed for bridge rate limit
*                                      calculations is taken from a per-port configuration
*                                      cpssDxChBrgGenPortRateLimitSpeedGranularitySet()
*                                      and NOT from the port actual speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitSpeedCfgEnableGet
(
    IN  GT_U8      devNum,
    OUT GT_BOOL    *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRateLimitSpeedCfgEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgGenRateLimitSpeedCfgEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortRateLimitSpeedGranularitySet function
* @endinternal
*
* @brief   Configure 100Mbps/10Mbps/1Gbps/10Gbps port speed for the use of the bridge
*         ingress rate limit calculations.
*         If port speed for Bridge ingress rate limit calculations is enabled,
*         cpssDxChBrgGenRateLimitSpeedCfgEnableSet() API, then this configuration
*         is used.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      (APPLICABLE RANGES: xCat, xCat3 0..23)
* @param[in] speedGranularity         - Port speed granularity
*                                      (10Mbps/100Mbps/1Gbps/10Gbps) that
*                                      will be used for bridge ingress rate limit
*                                      calculations.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum or speedGranularity
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitSpeedGranularitySet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_SPEED_ENT      speedGranularity
)
{
    GT_U32  regAddrMii;    /* MII speed register address */
    GT_U32  regAddrGmii;   /* GMII speed register address */
    GT_U32  regAddrXg;  /* Stack port speed register address */
    GT_U32  stackValue;    /* stack port register value */
    GT_U32  bitOffset;     /* bit offset inside register */
    GT_U32  miiSpeed;      /* MII speed (10/100) selection value */
    GT_U32  gmiiSpeed;     /* GMII speed (1000) selection value */
    GT_U32  xgSpeed;       /* XG speed (10 000) selection value */
    GT_STATUS rc;          /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(portNum >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                portNum);
        }

        /* NOTE: USE GLOBAL PORT -- no local port convert */
        regAddrMii = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.MIISpeedGranularity[OFFSET_TO_WORD_MAC(portNum)];
        regAddrGmii = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.GMIISpeedGranularity[OFFSET_TO_WORD_MAC(portNum)];
        regAddrXg = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.XGSpeedGranularity[OFFSET_TO_WORD_MAC(portNum)];

        bitOffset = OFFSET_TO_BIT_MAC(portNum);
        miiSpeed  = 0;
        gmiiSpeed = 0;
        xgSpeed   = 0;
        switch (speedGranularity)
        {
            /* 10 Mbps */
            case CPSS_PORT_SPEED_10_E:
                break;

            /* 100 Mbps*/
            case CPSS_PORT_SPEED_100_E:
                miiSpeed = 1;
                break;

            /* 1 Gbps*/
            case CPSS_PORT_SPEED_1000_E:
                gmiiSpeed = 1;
                break;

            /* 10 Gbps*/
            case CPSS_PORT_SPEED_10000_E:
                xgSpeed = 1;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Configure MII speed select register */
        rc = prvCpssHwPpSetRegField(devNum, regAddrMii, bitOffset, 1, miiSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Configure GMII speed select register */
        rc = prvCpssHwPpSetRegField(devNum, regAddrGmii, bitOffset, 1, gmiiSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Configure XG speed select register */
        rc = prvCpssHwPpSetRegField(devNum, regAddrXg, bitOffset, 1, xgSpeed);
        return rc;
    }

    if(PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat2 devices */
        regAddrMii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.miiSpeedGranularity;
        regAddrGmii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.gmiiSpeedGranularity;
        regAddrXg = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.stackSpeedGranularity;

        /* MII bit offset */
        bitOffset = portNum;
    }
    else
    {
        /* xCat & xCat3 supports only first 24 ports */
        if (portNum >= PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* xCat & xCat3 devices */
        regAddrMii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.dft1Reg;
        regAddrGmii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.mllMetalFix;
        regAddrXg = 0;/* not used */

        /* MII bit offset */
        bitOffset = (portNum + 1);
    }


    switch (speedGranularity)
    {
        /* 10 Mbps */
        case CPSS_PORT_SPEED_10_E:
            miiSpeed  = 0;
            gmiiSpeed = 0;
            xgSpeed   = 1;
       break;

        /* 100 Mbps*/
        case CPSS_PORT_SPEED_100_E:
            miiSpeed  = 1;
            gmiiSpeed = 0;
            xgSpeed   = 1;
            break;

        /* 1 Gbps*/
        case CPSS_PORT_SPEED_1000_E:
            miiSpeed  = 0; /* don't care in 1000 mode */
            gmiiSpeed = 1;
            xgSpeed   = 1;
            break;

        /* 10 Gbps*/
        case CPSS_PORT_SPEED_10000_E:
            /* 10G window supported only for stack ports 24..27 */
            if (portNum < PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            miiSpeed  = 0; /* don't care in 10000 mode */
            gmiiSpeed = 0; /* don't care in 10000 mode */
            xgSpeed   = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* network and stacking ports have different registers for configuration */
    if (portNum < PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS)
    {
        /* skip MII speed configuration for 1000.
           10000 speed is ignored because it's not supported
           for network ports. */
        if (gmiiSpeed == 0)
        {
            /* Configure MII speed select register */
            rc = prvCpssHwPpSetRegField(devNum, regAddrMii, bitOffset, 1, miiSpeed);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* Configure GMII speed select register */
        rc = prvCpssHwPpSetRegField(devNum, regAddrGmii, portNum, 1, gmiiSpeed);
    }
    else
    {  /* stack port configuration */
        /* the stack port has 3 bits for speed selection:
            - bit#0 is 1000 speed selection.
            - bit#1 is 10/100 speed selection.
            - bit#2 is 10 000 speed selection. */
        stackValue = gmiiSpeed | (miiSpeed << 1) | (xgSpeed << 2);
        bitOffset = (portNum - PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS) * 3;

        /* Configure Stack port speed select register */
        rc = prvCpssHwPpSetRegField(devNum, regAddrXg, bitOffset, 3, stackValue);
    }

    return rc;
}

/**
* @internal cpssDxChBrgGenPortRateLimitSpeedGranularitySet function
* @endinternal
*
* @brief   Configure 100Mbps/10Mbps/1Gbps/10Gbps port speed for the use of the bridge
*         ingress rate limit calculations.
*         If port speed for Bridge ingress rate limit calculations is enabled,
*         cpssDxChBrgGenRateLimitSpeedCfgEnableSet() API, then this configuration
*         is used.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      (APPLICABLE RANGES: xCat, xCat3 0..23)
* @param[in] speedGranularity         - Port speed granularity
*                                      (10Mbps/100Mbps/1Gbps/10Gbps) that
*                                      will be used for bridge ingress rate limit
*                                      calculations.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum or speedGranularity
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenPortRateLimitSpeedGranularitySet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_SPEED_ENT      speedGranularity
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitSpeedGranularitySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speedGranularity));

    rc = internal_cpssDxChBrgGenPortRateLimitSpeedGranularitySet(devNum, portNum, speedGranularity);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speedGranularity));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortRateLimitSpeedGranularityGet function
* @endinternal
*
* @brief   Get port speed configuration. This configuration is used for bridge
*         ingress rate limit calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      (APPLICABLE RANGES: xCat, xCat3 0..23)
*
* @param[out] speedGranularityPtr      - (pointer to) port speed granularity
*                                      (10Mbps/100Mbps/1Gbps/10Gbps) that will be
*                                      used for bridge ingress rate limit calculations.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitSpeedGranularityGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_SPEED_ENT         *speedGranularityPtr
)
{
    GT_U32  regValue;     /* register value */
    GT_U32  bitOffset;    /* bit offset inside MII register */
    GT_U32  regAddrMii;   /* MII speed register address */
    GT_U32  regAddrGmii;  /* GMII speed register address */
    GT_U32  regAddrXg; /* Stack port speed register address */
    GT_STATUS rc;         /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(speedGranularityPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(portNum >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                portNum);
        }
        /* NOTE: USE GLOBAL PORT -- no local port convert */
        regAddrMii = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.MIISpeedGranularity[OFFSET_TO_WORD_MAC(portNum)];
        regAddrGmii = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.GMIISpeedGranularity[OFFSET_TO_WORD_MAC(portNum)];
        regAddrXg = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.XGSpeedGranularity[OFFSET_TO_WORD_MAC(portNum)];

        bitOffset = OFFSET_TO_BIT_MAC(portNum);

        /* get XG speed select register */
        rc = prvCpssHwPpGetRegField(devNum, regAddrXg, bitOffset, 1, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(regValue)
        {
            *speedGranularityPtr = CPSS_PORT_SPEED_10000_E;
            return GT_OK;
        }

        /* get GMII speed select register */
        rc = prvCpssHwPpGetRegField(devNum, regAddrGmii, bitOffset, 1, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(regValue)
        {
            *speedGranularityPtr = CPSS_PORT_SPEED_1000_E;
            return GT_OK;
        }

        /* get MII speed select register */
        rc = prvCpssHwPpGetRegField(devNum, regAddrMii, bitOffset, 1, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        *speedGranularityPtr = regValue ?
                        CPSS_PORT_SPEED_100_E :
                        CPSS_PORT_SPEED_10_E;

        return GT_OK;
    }

    if(PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat2 devices */
        regAddrMii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.miiSpeedGranularity;
        regAddrGmii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.gmiiSpeedGranularity;
        regAddrXg = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.stackSpeedGranularity;

        /* MII bit offset */
        bitOffset = portNum;
    }
    else
    {
        /* xCat & xCat3 supports only first 24 ports */
        if (portNum >= PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* xCat & xCat3 devices */
        regAddrMii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.dft1Reg;
        regAddrGmii = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.mllMetalFix;
        regAddrXg = 0;/* not used */

        /* MII bit offset */
        bitOffset = (portNum + 1);
    }

    /* network and stacking ports have different registers for configuration */
    if (portNum < PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS)
    {
        /* read GMII register first. */
        rc = prvCpssHwPpGetRegField(devNum, regAddrGmii, portNum, 1, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (regValue == 1)
        {
            *speedGranularityPtr = CPSS_PORT_SPEED_1000_E;
            return GT_OK;
        }

        /* read MII speed register */
        rc = prvCpssHwPpGetRegField(devNum, regAddrMii, bitOffset, 1, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        *speedGranularityPtr = (regValue == 1) ? CPSS_PORT_SPEED_100_E:
                                         CPSS_PORT_SPEED_10_E;
    }
    else
    {  /* stack port  */
        regAddrXg = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.stackSpeedGranularity;
        bitOffset = (portNum - PRV_DXCH_FIRST_XG_CAPABLE_PORT_CNS) * 3;

        /* read Stack port speed select register */
        rc = prvCpssHwPpGetRegField(devNum, regAddrXg, bitOffset, 3, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* the stack port has 3 bits for speed selection:
            - bit#0 is 1000 speed selection.
            - bit#1 is 10/100 speed selection.
            - bit#2 is 10 000 speed selection.*/

        /* check XG bit */
        if ((regValue & 0x4) == 0)
        {
            *speedGranularityPtr = CPSS_PORT_SPEED_10000_E;
        }
        /* check GMII bit */
        else if ((regValue & 0x1) == 1)
        {
            *speedGranularityPtr = CPSS_PORT_SPEED_1000_E;
        }
        else
        {
            /* the bit#2 is 1 and bit#0 is 0.
              the regValue may be 6 for 100 speed and 4 for 10 speed */
            *speedGranularityPtr = (regValue != 4) ? CPSS_PORT_SPEED_100_E:
                                             CPSS_PORT_SPEED_10_E;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenPortRateLimitSpeedGranularityGet function
* @endinternal
*
* @brief   Get port speed configuration. This configuration is used for bridge
*         ingress rate limit calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      (APPLICABLE RANGES: xCat, xCat3 0..23)
*
* @param[out] speedGranularityPtr      - (pointer to) port speed granularity
*                                      (10Mbps/100Mbps/1Gbps/10Gbps) that will be
*                                      used for bridge ingress rate limit calculations.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenPortRateLimitSpeedGranularityGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_SPEED_ENT         *speedGranularityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitSpeedGranularityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speedGranularityPtr));

    rc = internal_cpssDxChBrgGenPortRateLimitSpeedGranularityGet(devNum, portNum, speedGranularityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speedGranularityPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet function
* @endinternal
*
* @brief   Enable / Disable excluding unregistered IPv4 / IPv6 Link Local Multicast
*         packets from the per VLAN Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
* @param[in] enable                   - GT_TRUE -
*                                      exclude unregistered IPv4 / IPv6
*                                      Link Local Multicast packets from the per VLAN
*                                      Unregistered IPv4 / IPv6 Multicast command
*                                      - GT_FALSE -
*                                      Unregistered link Local Multicast packets
*                                      are handled according to the
*                                      Unregistered IPv4 / IPv6 Multicast command.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  bitOffset;  /* bit offset inside register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;

        switch (protocolStack)
        {
        case CPSS_IP_PROTOCOL_IPV4_E:
            bitOffset = 0;
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            bitOffset = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* Bridge Global Configuration Register2,
            field <ExcludeLLMCFromUnregIPv4MCFilter / ExcludeLLMCFromUnregIPv6MCFilter> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[2];

        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                bitOffset = 6;
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                bitOffset = 7;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    regValue = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, 1, regValue);

}

/**
* @internal cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet function
* @endinternal
*
* @brief   Enable / Disable excluding unregistered IPv4 / IPv6 Link Local Multicast
*         packets from the per VLAN Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
* @param[in] enable                   - GT_TRUE -
*                                      exclude unregistered IPv4 / IPv6
*                                      Link Local Multicast packets from the per VLAN
*                                      Unregistered IPv4 / IPv6 Multicast command
*                                      - GT_FALSE -
*                                      Unregistered link Local Multicast packets
*                                      are handled according to the
*                                      Unregistered IPv4 / IPv6 Multicast command.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enable));

    rc = internal_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(devNum, protocolStack, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet function
* @endinternal
*
* @brief   Get enable / disable status for excluding unregistered IPv4 / IPv6
*         Link Local Multicast packets from the per VLAN
*         Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
*
* @param[out] enablePtr                - pointer to status for excluding unregistered
*                                      IPv4 / IPv6 Link Local Multicast packets from the
*                                      per VLAN Unregistered IPv4 / IPv6 Multicast command.
*                                      - GT_TRUE -
*                                      exclude unregistered IPv4 / IPv6
*                                      Link Local Multicast packets from the per VLAN
*                                      Unregistered IPv4 / IPv6 Multicast command
*                                      - GT_FALSE -
*                                      Unregistered link Local Multicast packets
*                                      are handled according to the
*                                      Unregistered IPv4 / IPv6 Multicast command.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  bitOffset;  /* bit offset inside register */
    GT_STATUS rc;       /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;

        switch (protocolStack)
        {
        case CPSS_IP_PROTOCOL_IPV4_E:
            bitOffset = 0;
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            bitOffset = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* Bridge Global Configuration Register2,
            field <ExcludeLLMCFromUnregIPv4MCFilter / ExcludeLLMCFromUnregIPv6MCFilter> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[2];

        switch (protocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                bitOffset = 6;
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                bitOffset = 7;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, 1, &regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (regValue == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet function
* @endinternal
*
* @brief   Get enable / disable status for excluding unregistered IPv4 / IPv6
*         Link Local Multicast packets from the per VLAN
*         Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
*
* @param[out] enablePtr                - pointer to status for excluding unregistered
*                                      IPv4 / IPv6 Link Local Multicast packets from the
*                                      per VLAN Unregistered IPv4 / IPv6 Multicast command.
*                                      - GT_TRUE -
*                                      exclude unregistered IPv4 / IPv6
*                                      Link Local Multicast packets from the per VLAN
*                                      Unregistered IPv4 / IPv6 Multicast command
*                                      - GT_FALSE -
*                                      Unregistered link Local Multicast packets
*                                      are handled according to the
*                                      Unregistered IPv4 / IPv6 Multicast command.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enablePtr));

    rc = internal_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(devNum, protocolStack, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32  regAddr;            /* register address */
    GT_U32  value;              /* value to write from register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                          CPSS_LION_E );

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

        value = BOOL2BIT_MAC(enable);

        /* set the <EN_LEARN_ON_TRAP_IEEE_RSRV_MC>*/
        return prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ON_TRAP_IEEE_RSRV_MC_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        value);
    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_1_REGADDR_PTR(devNum,portNum);

        value = BOOL2BIT_MAC(enable);

        /* Ingress Port<n> Bridge Configuration Register1,
           field <EnLearnOnTrapIEEEReservedMC> */
        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr,
                                                  25, 1, value);
    }
}

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet function
* @endinternal
*
* @brief   Get status of MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                pointer to status of MAC SA learning on reserved IEEE
*                                      MC packets trapped packets:
*                                      - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32  regAddr;            /* register address */
    GT_U32  value;              /* value to write from register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_STATUS rc;               /* return code      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                          CPSS_LION_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

        /* set the <EN_LEARN_ON_TRAP_IEEE_RSRV_MC>*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ON_TRAP_IEEE_RSRV_MC_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    }
    else
    {
        PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_1_REGADDR_PTR(devNum,portNum);

        /* Ingress Port<n> Bridge Configuration Register1,
           field <EnLearnOnTrapIEEEReservedMC> */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 25, 1, &value);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet function
* @endinternal
*
* @brief   Get status of MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                pointer to status of MAC SA learning on reserved IEEE
*                                      MC packets trapped packets:
*                                      - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortRateLimitDropCntrEnableSet function
* @endinternal
*
* @brief   Enable/Disable port for counting packets dropped due to ingress rate
*         limiting by the Ingress Rate Limit Drop Counter
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  counting of packets dropped due to
*                                      ingress rate.
*                                      GT_FALSE: disable counting of packets dropped due to
*                                      ingress rate.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitDropCntrEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  value;          /* value to write into register */
    GT_U32  portGroupId;    /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(portNum >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                portNum);
        }
        /* set the field <RateLimitDropCountEn> */
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
                                        portNum,/* global physical port */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        4,/*start bit*/
                                        1,/*num of bits*/
                                        BOOL2BIT_MAC(enable));
    }
    else
    {
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.portControl[portNum];

    value = BOOL2BIT_MAC(enable);

    /* Ingress Port<n> Bridge Configuration Register0, <RateLimitDropCount> */
    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 11, 1, value);
}
}

/**
* @internal cpssDxChBrgGenPortRateLimitDropCntrEnableSet function
* @endinternal
*
* @brief   Enable/Disable port for counting packets dropped due to ingress rate
*         limiting by the Ingress Rate Limit Drop Counter
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  counting of packets dropped due to
*                                      ingress rate.
*                                      GT_FALSE: disable counting of packets dropped due to
*                                      ingress rate.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitDropCntrEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitDropCntrEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenPortRateLimitDropCntrEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortRateLimitDropCntrEnableGet function
* @endinternal
*
* @brief   Get port status (enabled/disabled) for counting packets dropped due
*         to ingress rate limiting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE: enable counting of packets dropped due to
*                                      ingress rate.
*                                      GT_FALSE: disable counting of packets dropped due to
*                                      ingress rate.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortRateLimitDropCntrEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_STATUS rc;           /* function return code */
    GT_U32 value;           /* value to write into register */
    GT_U32  portGroupId;    /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(portNum >= 256)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rate limit supports ports[0..255] and not [%d] \n",
                portNum);
        }
        /* get the field <RateLimitDropCountEn> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
                                        portNum,/* global physical port */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        4,/*start bit*/
                                        1,/*num of bits*/
                                        &value);
    }
    else
    {
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.portControl[portNum];

    /* Ingress Port<n> Bridge Configuration Register0, <RateLimitDropCount> */
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 11, 1, &value);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenPortRateLimitDropCntrEnableGet function
* @endinternal
*
* @brief   Get port status (enabled/disabled) for counting packets dropped due
*         to ingress rate limiting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE: enable counting of packets dropped due to
*                                      ingress rate.
*                                      GT_FALSE: disable counting of packets dropped due to
*                                      ingress rate.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitDropCntrEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortRateLimitDropCntrEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenPortRateLimitDropCntrEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRateLimitDropCntrSet function
* @endinternal
*
* @brief   Set rate limit drop counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - the  to set the rate limit drop counter.
*                                      the value size is 40 bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range drop counter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenRateLimitDropCntrSet
(
    IN GT_U8        devNum,
    IN GT_U64       value
)
{
    GT_U32              regAddr0;       /* first register address               */
    GT_U32              regAddr1;       /* second register address              */
    GT_U32              portGroupId;    /*  the port group Id - support multi-port-groups device */
    GT_U16              portGroupsNum;  /* number of port groups                */
    GT_U16              portGroupIndex; /* index of port group in BMP           */
    GT_U64              avrValue;       /* average value for one port group     */
    GT_STATUS           rc;             /* return code                          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr0 = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.ingrRateLimitDropCntr310;
        regAddr1 = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.ingrRateLimitDropCntr3932;
    }
    else
    {
        regAddr0 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeRateLimitDropCounter[0];
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeRateLimitDropCounter[1];
    }

    /* calculate number of port groups */
    rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,PRV_CPSS_DXCH_UNIT_L2I_E,&portGroupsNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* there are 40 bits for the counter so only 8 msb .

        but we 'split' the value between all port groups , so the counter can be
        multiple of portGroupsNum from 40 bits .

        so for 8 port groups we can support 43 bits counter !
    */
    if (value.l[1] >= (GT_U32)(BIT_8 * portGroupsNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* loop on all port groups to set the counter value */
    portGroupIndex = 0;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(
        devNum, portGroupId)
    {
        prvCpssPortGroupCounterAverageValueGet(portGroupsNum, portGroupIndex,
                                                   value, &avrValue);

        /* Ingress Rate Limit Drop Counter Register[0] - 32 lsb */
        rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupIndex,
                                                  regAddr0, avrValue.l[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Ingress Rate Limit Drop Counter Register[1] - 8 msb */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupIndex,
                                                regAddr1, 0, 8, avrValue.l[1]);
        if (rc != GT_OK)
        {
            return rc;
        }

        portGroupIndex++;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenRateLimitDropCntrSet function
* @endinternal
*
* @brief   Set rate limit drop counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - the  to set the rate limit drop counter.
*                                      the value size is 40 bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range drop counter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenRateLimitDropCntrSet
(
    IN GT_U8        devNum,
    IN GT_U64       value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRateLimitDropCntrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, value));

    rc = internal_cpssDxChBrgGenRateLimitDropCntrSet(devNum, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenRateLimitDropCntrGet function
* @endinternal
*
* @brief   Get rate limit drop counter.
*         The counter counts packets or bytes according to ingress rate limit mode
*         set by cpssDxChBrgGenRateLimitGlobalCfgSet.
*         The counter counts packets/bytes that are dropped due to ingress rate
*         limitting on all ports, where the drop rate limit counting is enabled
*         by cpssDxChBrgGenPortRateLimitDropCntrEnableSet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) the value of rate limit drop counter
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenRateLimitDropCntrGet
(
    IN  GT_U8        devNum,
    OUT GT_U64       *valuePtr
)
{
    GT_U32              regAddr0;       /* first register address               */
    GT_U32              regAddr1;       /* second register address              */
    GT_U32              tmpValue;       /* temporary value                      */
    GT_STATUS           rc;             /* return code                  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr0 = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.ingrRateLimitDropCntr310;
        regAddr1 = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ingrPortsRateLimitConfig.ingrRateLimitDropCntr3932;
    }
    else
    {
        regAddr0 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeRateLimitDropCounter[0];
        regAddr1 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeRateLimitDropCounter[1];
    }

    /* summary 32 lsb bits */
    rc = prvCpssPortGroupsBmpCounterSummary(devNum,
                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                regAddr0, 0, 32,
                                                NULL,
                                                valuePtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* summary 8 msb bits */
    rc = prvCpssPortGroupsBmpCounterSummary(devNum,
                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                regAddr1, 0, 8,
                                                &tmpValue,
                                                NULL);
    if (GT_OK != rc)
    {
        return rc;
    }

    valuePtr->l[1] += tmpValue;

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenRateLimitDropCntrGet function
* @endinternal
*
* @brief   Get rate limit drop counter.
*         The counter counts packets or bytes according to ingress rate limit mode
*         set by cpssDxChBrgGenRateLimitGlobalCfgSet.
*         The counter counts packets/bytes that are dropped due to ingress rate
*         limitting on all ports, where the drop rate limit counting is enabled
*         by cpssDxChBrgGenPortRateLimitDropCntrEnableSet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) the value of rate limit drop counter
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenRateLimitDropCntrGet
(
    IN  GT_U8        devNum,
    OUT GT_U64       *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenRateLimitDropCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, valuePtr));

    rc = internal_cpssDxChBrgGenRateLimitDropCntrGet(devNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgGenPortArpMacSaMismatchDropEnable function
* @endinternal
*
* @brief   Enable/disable configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is dropped.
*                                      GT_FALSE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is not dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The global ARP MAC SA configuration must remain. When both global
*       configuration is enabled and per port configuration is enabled the
*       ARP MAC SA mismatch check is performed and if fails packet is dropped.
*
*/
static GT_STATUS internal_cpssDxChBrgGenPortArpMacSaMismatchDropEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_U32      value;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

     /* prepare value to write */
    value = BOOL2BIT_MAC(enable);

    /* set the <ARP_MAC_SA_MIS_DROP_EN>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                SIP5_L2I_EPORT_TABLE_FIELDS_ARP_MAC_SA_MIS_DROP_EN_E, /* field name */
                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            value);
}

/**
* @internal cpssDxChBrgGenPortArpMacSaMismatchDropEnable function
* @endinternal
*
* @brief   Enable/disable configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is dropped.
*                                      GT_FALSE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is not dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The global ARP MAC SA configuration must remain. When both global
*       configuration is enabled and per port configuration is enabled the
*       ARP MAC SA mismatch check is performed and if fails packet is dropped.
*
*/
GT_STATUS cpssDxChBrgGenPortArpMacSaMismatchDropEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortArpMacSaMismatchDropEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenPortArpMacSaMismatchDropEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet function
* @endinternal
*
* @brief   Get configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is dropped.
*                                      GT_FALSE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is not dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc = GT_OK; /* function return code */
    GT_U32      value;      /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* get the <ARP_MAC_SA_MIS_DROP_EN>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_ARP_MAC_SA_MIS_DROP_EN_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    if(rc!=GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet function
* @endinternal
*
* @brief   Get configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is dropped.
*                                      GT_FALSE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is not dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] cmd                      - Unknown DA command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32      hwValue;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValue, cmd)

    /* set the <INGRESS_PORT_UNKNOWN_UC_FILTER_CMD>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNKNOWN_UC_FILTER_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwValue);
}

/**
* @internal cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] cmd                      - Unknown DA command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cmd));

    rc = internal_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(devNum, portNum, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cmdPtr                   - (pointer to) Unknown DA command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
{
    GT_STATUS   rc = GT_OK;     /* function return code */
    GT_U32      hwValue,value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* get the <INGRESS_PORT_UNKNOWN_UC_FILTER_CMD>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNKNOWN_UC_FILTER_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if(rc!=GT_OK)
        return rc;

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(value,hwValue);

    *cmdPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cmdPtr                   - (pointer to) Unknown DA command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cmdPtr));

    rc = internal_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(devNum, portNum, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] cmd                      - Unregistered MC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32      hwValue;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValue, cmd)

    /* set the <INGRESS_PORT_UNREG_MC_FILTER_CMD>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNREG_MC_FILTER_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwValue);
}

/**
* @internal cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] cmd                      - Unregistered MC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cmd));

    rc = internal_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(devNum, portNum, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] cmdPtr                   - (pointer to) Unregistered MC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
{
    GT_STATUS   rc = GT_OK;     /* function return code */
    GT_U32      hwValue,value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* get the <INGRESS_PORT_UNREG_MC_FILTER_CMD>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNREG_MC_FILTER_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if(rc!=GT_OK)
        return rc;

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(value,hwValue);

    *cmdPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] cmdPtr                   - (pointer to) Unregistered MC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cmdPtr));

    rc = internal_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(devNum, portNum, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIngressPortBcFilterDaCommandSet function
* @endinternal
*
* @brief   Set BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] cmd                      - BC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIngressPortBcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32      hwValue;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValue, cmd)

    /* set the <INGRESS_PORT_BC_FILTER_CMD>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_BC_FILTER_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwValue);
}

/**
* @internal cpssDxChBrgGenIngressPortBcFilterDaCommandSet function
* @endinternal
*
* @brief   Set BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] cmd                      - BC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortBcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIngressPortBcFilterDaCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cmd));

    rc = internal_cpssDxChBrgGenIngressPortBcFilterDaCommandSet(devNum, portNum, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIngressPortBcFilterDaCommandGet function
* @endinternal
*
* @brief   Get BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cmdPtr                   - (pointer to) BC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIngressPortBcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
{
    GT_STATUS   rc = GT_OK;     /* function return code */
    GT_U32      hwValue,value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* get the <INGRESS_PORT_BC_FILTER_CMD>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_BC_FILTER_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if(rc!=GT_OK)
        return rc;

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(value,hwValue);

    *cmdPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgGenIngressPortBcFilterDaCommandGet function
* @endinternal
*
* @brief   Get BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cmdPtr                   - (pointer to) BC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortBcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIngressPortBcFilterDaCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cmdPtr));

    rc = internal_cpssDxChBrgGenIngressPortBcFilterDaCommandGet(devNum, portNum, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortIpControlTrapEnableSet function
* @endinternal
*
* @brief   Set IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ipCntrlType              - IP control type
*                                      CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortIpControlTrapEnableSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT       ipCntrlType
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ipv4Value,ipv6Value;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);


    switch(ipCntrlType)
    {
        case CPSS_DXCH_BRG_IP_CTRL_NONE_E:
            ipv4Value = 0;
            ipv6Value = 0;
            break;

        case CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E:
            ipv4Value = 1;
            ipv6Value = 1;
            break;

        case CPSS_DXCH_BRG_IP_CTRL_IPV4_E:
            ipv4Value = 1;
            ipv6Value = 0;
            break;

        case CPSS_DXCH_BRG_IP_CTRL_IPV6_E:
            ipv4Value = 0;
            ipv6Value = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* set the <IPV4_CONTROL_TRAP_EN>*/
    rc =  prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_IPV4_CONTROL_TRAP_EN_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            ipv4Value);
    if(rc != GT_OK)
        return rc;

    /* set the <IPV6_CONTROL_TRAP_EN>*/
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_IPV6_CONTROL_TRAP_EN_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            ipv6Value);
    return rc;

}

/**
* @internal cpssDxChBrgGenPortIpControlTrapEnableSet function
* @endinternal
*
* @brief   Set IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ipCntrlType              - IP control type
*                                      CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIpControlTrapEnableSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT       ipCntrlType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortIpControlTrapEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipCntrlType));

    rc = internal_cpssDxChBrgGenPortIpControlTrapEnableSet(devNum, portNum, ipCntrlType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipCntrlType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortIpControlTrapEnableGet function
* @endinternal
*
* @brief   Get IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ipCntrlTypePtr           - (pointer to) IP control type
*                                      CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortIpControlTrapEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ipv4Value,ipv6Value;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(ipCntrlTypePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* Get the <IPV4_CONTROL_TRAP_EN>*/
    rc =  prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_IPV4_CONTROL_TRAP_EN_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &ipv4Value);
    if(rc!=GT_OK)
        return rc;

    /* Get the <IPV6_CONTROL_TRAP_EN>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_IPV6_CONTROL_TRAP_EN_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &ipv6Value);

    if(rc!=GT_OK)
        return rc;

    if((ipv4Value == 0)&&(ipv6Value == 0))
    {
        *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    }
    else
    {
        if((ipv4Value == 1)&&(ipv6Value == 1))
        {
            *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;
        }
        else
        {
            if((ipv4Value == 1)&&(ipv6Value == 0))
            {
                *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_IPV4_E;
            }
            else/* ((ipv4Value == 0)&&(ipv6Value == 1)) */
            {
                *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;
            }
        }
    }

    return rc;
}

/**
* @internal cpssDxChBrgGenPortIpControlTrapEnableGet function
* @endinternal
*
* @brief   Get IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ipCntrlTypePtr           - (pointer to) IP control type
*                                      CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIpControlTrapEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortIpControlTrapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipCntrlTypePtr));

    rc = internal_cpssDxChBrgGenPortIpControlTrapEnableGet(devNum, portNum, ipCntrlTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipCntrlTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet function
* @endinternal
*
* @brief   Set BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/disable UDP Trap Mirror.
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32    hwData;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    hwData = BOOL2BIT_MAC(enable);

    /* set the <BC_UDP_TRAP_OR_MIRROR_EN>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                            portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_BC_UDP_TRAP_OR_MIRROR_EN_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
}

/**
* @internal cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet function
* @endinternal
*
* @brief   Set BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/disable UDP Trap Mirror.
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet function
* @endinternal
*
* @brief   Get BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                - UDP Trap Mirror status
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwData;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* get the <BC_UDP_TRAP_OR_MIRROR_EN>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_BC_UDP_TRAP_OR_MIRROR_EN_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);
    if(rc!=GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet function
* @endinternal
*
* @brief   Get BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                - UDP Trap Mirror status
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenBypassModeSet function
* @endinternal
*
* @brief   Set Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
static GT_STATUS internal_cpssDxChBrgGenBypassModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT    mode
)
{
    GT_U32      fieldValue;     /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch(mode)
    {
        case CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E:
            fieldValue = 0;
            break;

        case CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E:
            fieldValue = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                  bridgeEngineConfig.bridgeGlobalConfig1,
                    14, 1, fieldValue);
}

/**
* @internal cpssDxChBrgGenBypassModeSet function
* @endinternal
*
* @brief   Set Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS cpssDxChBrgGenBypassModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT    mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenBypassModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgGenBypassModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenBypassModeGet function
* @endinternal
*
* @brief   Get Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
static GT_STATUS internal_cpssDxChBrgGenBypassModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      fieldValue;     /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssHwPpGetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                  bridgeEngineConfig.bridgeGlobalConfig1,
                    14, 1, &fieldValue);

    if (rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (fieldValue == 0x0) ?
                    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E :
                    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E;

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenBypassModeGet function
* @endinternal
*
* @brief   Get Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS cpssDxChBrgGenBypassModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenBypassModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgGenBypassModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenMtuConfigSet function
* @endinternal
*
* @brief   Enable/disable Target ePort MTU check feature.
*         and configure the following parameters:
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mtuCheckEnable           - target ePort MTU check feature enabled/disabled
*                                      GT_TRUE  MTU check enabled, GT_FALSE - MTU check disabled
* @param[in] byteCountMode            - byte-count mode for MTU
* @param[in] exceptionCommand         command assigned to frames that exceed the
*                                      default ePort MTU size
* @param[in] exceptionCpuCode         CPU/drop code assigned to frame which fails MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, byteCountMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenMtuConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_BOOL                                      mtuCheckEnable,
    IN CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode,
    IN CPSS_PACKET_CMD_ENT                          exceptionCommand,
    IN CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode
)
{
    GT_STATUS   rc;
    GT_U32      byteCountModeValue;
    GT_U32      exceptionCommandValue;
    GT_U32      regAddr;                   /* register address */
    GT_U32      hwData = 0;                /* value to write into register */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(exceptionCommandValue,exceptionCommand);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        preEgrEngineGlobalConfig.MTUGlobalConfig;

    /* set the MTU global config - target ePort MTU check enabled / disabled */
    hwData |= BOOL2BIT_MAC(mtuCheckEnable);

    /* set the MTU global config - byte count mode */
    switch(byteCountMode)
    {
        case CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E:
            byteCountModeValue = 0;
            break;

        case CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E:
            byteCountModeValue = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    hwData |= byteCountModeValue << 1;

    /* set the MTU global config - exception command for packets that exceed the MTU check */
    hwData |= exceptionCommandValue << 2;

    /* set the MTU global config - CPU code for packets that exceed the MTU check and sent to CPU */
    rc = prvCpssDxChNetIfCpuToDsaCode(exceptionCpuCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }
    hwData |= dsaCpuCode << 5;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 13, hwData);
    return rc;
}

/**
* @internal cpssDxChBrgGenMtuConfigSet function
* @endinternal
*
* @brief   Enable/disable Target ePort MTU check feature.
*         and configure the following parameters:
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mtuCheckEnable           - target ePort MTU check feature enabled/disabled
*                                      GT_TRUE  MTU check enabled, GT_FALSE - MTU check disabled
* @param[in] byteCountMode            - byte-count mode for MTU
* @param[in] exceptionCommand         command assigned to frames that exceed the
*                                      default ePort MTU size
* @param[in] exceptionCpuCode         CPU/drop code assigned to frame which fails MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, byteCountMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenMtuConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_BOOL                                      mtuCheckEnable,
    IN CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode,
    IN CPSS_PACKET_CMD_ENT                          exceptionCommand,
    IN CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenMtuConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mtuCheckEnable, byteCountMode, exceptionCommand, exceptionCpuCode));

    rc = internal_cpssDxChBrgGenMtuConfigSet(devNum, mtuCheckEnable, byteCountMode, exceptionCommand, exceptionCpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mtuCheckEnable, byteCountMode, exceptionCommand, exceptionCpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenMtuConfigGet function
* @endinternal
*
* @brief   Get the target ePort MTU feature parameters:
*         - the enable/disable target ePort MTU check feature.
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] mtuCheckEnablePtr        - (pointer to) enable/disable target ePort MTU feature.
*                                      GT_TRUE  MTU check enabled, GT_FALSE - MTU check disabled.
* @param[out] byteCountModePtr         - (pointer to) MTU byte-count mode
* @param[out] exceptionCommandPtr      - (pointer to) exception cmd for frames which
*                                      fails MTU check
* @param[out] exceptionCpuCodePtr      - (pointer to) CPU/drop code assigned to frame
*                                      which fails the MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong HW value
*/
static GT_STATUS internal_cpssDxChBrgGenMtuConfigGet
(
    IN  GT_U8                                         devNum,
    OUT GT_BOOL                                      *mtuCheckEnablePtr,
    OUT CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    *byteCountModePtr,
    OUT CPSS_PACKET_CMD_ENT                          *exceptionCommandPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT                     *exceptionCpuCodePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;                   /* register address */
    GT_U32      regData;                   /* value read from register */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mtuCheckEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(byteCountModePtr);
    CPSS_NULL_PTR_CHECK_MAC(exceptionCommandPtr);
    CPSS_NULL_PTR_CHECK_MAC(exceptionCpuCodePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        preEgrEngineGlobalConfig.MTUGlobalConfig;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get the MTU global config - MTU check enable / disable */
    *mtuCheckEnablePtr = ((U32_GET_FIELD_MAC(regData, 0, 1)) == 0) ? GT_FALSE : GT_TRUE;

    /* get the MTU global config - byte count mode */
    *byteCountModePtr = ((U32_GET_FIELD_MAC(regData, 1, 1)) == 0) ? CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E : CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;

    /* set the MTU global config - exception command for packets that exceed the MTU check */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*exceptionCommandPtr, (U32_GET_FIELD_MAC(regData, 2, 3)));

    /* get the MTU global config - CPU code for packets that exceed the MTU check and sent to CPU */
    dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)(U32_GET_FIELD_MAC(regData, 5, 8));
    rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode, exceptionCpuCodePtr);
    return rc;
}

/**
* @internal cpssDxChBrgGenMtuConfigGet function
* @endinternal
*
* @brief   Get the target ePort MTU feature parameters:
*         - the enable/disable target ePort MTU check feature.
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] mtuCheckEnablePtr        - (pointer to) enable/disable target ePort MTU feature.
*                                      GT_TRUE  MTU check enabled, GT_FALSE - MTU check disabled.
* @param[out] byteCountModePtr         - (pointer to) MTU byte-count mode
* @param[out] exceptionCommandPtr      - (pointer to) exception cmd for frames which
*                                      fails MTU check
* @param[out] exceptionCpuCodePtr      - (pointer to) CPU/drop code assigned to frame
*                                      which fails the MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong HW value
*/
GT_STATUS cpssDxChBrgGenMtuConfigGet
(
    IN  GT_U8                                         devNum,
    OUT GT_BOOL                                      *mtuCheckEnablePtr,
    OUT CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    *byteCountModePtr,
    OUT CPSS_PACKET_CMD_ENT                          *exceptionCommandPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT                     *exceptionCpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenMtuConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mtuCheckEnablePtr, byteCountModePtr, exceptionCommandPtr, exceptionCpuCodePtr));

    rc = internal_cpssDxChBrgGenMtuConfigGet(devNum, mtuCheckEnablePtr, byteCountModePtr, exceptionCommandPtr, exceptionCpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mtuCheckEnablePtr, byteCountModePtr, exceptionCommandPtr, exceptionCpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgGenMtuPortProfileIdxSet function
* @endinternal
*
* @brief   Set MTU profile index for the given target ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mtuProfileIdx            - index to global MTU entry
*                                      (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_OUT_OF_RANGE      mtuProfileIdx out of range
*/
static GT_STATUS internal_cpssDxChBrgGenMtuPortProfileIdxSet
(
    IN GT_U8         devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_U32        mtuProfileIdx
)
{
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (mtuProfileIdx > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* set ePort-to-physical-port-target-mapping table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
                                     portNum,
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                     0,
                                     2,
                                     mtuProfileIdx);
    return rc;

}

/**
* @internal cpssDxChBrgGenMtuPortProfileIdxSet function
* @endinternal
*
* @brief   Set MTU profile index for the given target ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mtuProfileIdx            - index to global MTU entry
*                                      (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_OUT_OF_RANGE      mtuProfileIdx out of range
*/
GT_STATUS cpssDxChBrgGenMtuPortProfileIdxSet
(
    IN GT_U8         devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_U32        mtuProfileIdx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenMtuPortProfileIdxSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mtuProfileIdx));

    rc = internal_cpssDxChBrgGenMtuPortProfileIdxSet(devNum, portNum, mtuProfileIdx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mtuProfileIdx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenMtuPortProfileIdxGet function
* @endinternal
*
* @brief   Get MTU profile index for the given ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] mtuProfileIdxPtr         - (pointer to)index to the global MTU entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenMtuPortProfileIdxGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    portNum,
    OUT GT_U32         *mtuProfileIdxPtr
)
{
    GT_STATUS rc;               /* return code          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mtuProfileIdxPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);


    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* get ePort-to-physical-port-target-mapping table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
                                     portNum,
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                     0,
                                     2,
                                     mtuProfileIdxPtr);
    return rc;

}

/**
* @internal cpssDxChBrgGenMtuPortProfileIdxGet function
* @endinternal
*
* @brief   Get MTU profile index for the given ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] mtuProfileIdxPtr         - (pointer to)index to the global MTU entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenMtuPortProfileIdxGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    portNum,
    OUT GT_U32         *mtuProfileIdxPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenMtuPortProfileIdxGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mtuProfileIdxPtr));

    rc = internal_cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, mtuProfileIdxPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mtuProfileIdxPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenMtuExceedCntrGet function
* @endinternal
*
* @brief   Get target ePort MTU exceeded counter.
*         The number of packets that triggered the Target ePort MTU
*         exceeded exception check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) the value of target ePort MTU exceeded counter.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenMtuExceedCntrGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *valuePtr
)
{
    GT_U32              regAddr;        /* register address */
    GT_STATUS           rc;             /* return code      */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        preEgrEngineGlobalConfig.targetEPortMTUExceededCntr;

    /* summary 32 lsb bits */
    rc = prvCpssPortGroupsBmpCounterSummary(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            regAddr, 0, 32,
                                            valuePtr, NULL);
    return rc;
}

/**
* @internal cpssDxChBrgGenMtuExceedCntrGet function
* @endinternal
*
* @brief   Get target ePort MTU exceeded counter.
*         The number of packets that triggered the Target ePort MTU
*         exceeded exception check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) the value of target ePort MTU exceeded counter.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenMtuExceedCntrGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenMtuExceedCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, valuePtr));

    rc = internal_cpssDxChBrgGenMtuExceedCntrGet(devNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgGenMtuProfileSet function
* @endinternal
*
* @brief   Set an MTU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
* @param[in] mtuSize                  - max transmit packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range mtuSize
*/
static GT_STATUS internal_cpssDxChBrgGenMtuProfileSet
(
    IN GT_U8    devNum,
    IN GT_U32   profile,
    IN GT_U32   mtuSize
)
{
    GT_U32 regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (profile > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (mtuSize > 16383)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        preEgrEngineGlobalConfig.MTUTableEntry[profile];

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 14, mtuSize);
}

/**
* @internal cpssDxChBrgGenMtuProfileSet function
* @endinternal
*
* @brief   Set an MTU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
* @param[in] mtuSize                  - max transmit packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range mtuSize
*/
GT_STATUS cpssDxChBrgGenMtuProfileSet
(
    IN GT_U8    devNum,
    IN GT_U32   profile,
    IN GT_U32   mtuSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenMtuProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, mtuSize));

    rc = internal_cpssDxChBrgGenMtuProfileSet(devNum, profile, mtuSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, mtuSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenMtuProfileGet function
* @endinternal
*
* @brief   Get an MTU size for ePort profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
*
* @param[out] mtuSizePtr               - (pointer to) max transmit packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgGenMtuProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profile,
    OUT GT_U32  *mtuSizePtr
)
{
    GT_U32 regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(mtuSizePtr);

    if (profile > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        preEgrEngineGlobalConfig.MTUTableEntry[profile];

    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 14, mtuSizePtr);
}

/**
* @internal cpssDxChBrgGenMtuProfileGet function
* @endinternal
*
* @brief   Get an MTU size for ePort profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
*
* @param[out] mtuSizePtr               - (pointer to) max transmit packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenMtuProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profile,
    OUT GT_U32  *mtuSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenMtuProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, mtuSizePtr));

    rc = internal_cpssDxChBrgGenMtuProfileGet(devNum, profile, mtuSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, mtuSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpmClassificationModeSet function
* @endinternal
*
* @brief   Configue IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] mode                     - IPM classification mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpmClassificationModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   mode
)
{
    GT_U32      fieldValue;     /* register field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    switch(mode)
    {
        case CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E:
            fieldValue = 0;
            break;
        case CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_AND_ETHERTYPE_E:
            fieldValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum,
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                  bridgeRegs.bridgeGlobalConfigRegArray[2],
                    21, 1, fieldValue);
}

/**
* @internal cpssDxChBrgGenIpmClassificationModeSet function
* @endinternal
*
* @brief   Configue IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] mode                     - IPM classification mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpmClassificationModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpmClassificationModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgGenIpmClassificationModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgGenIpmClassificationModeGet function
* @endinternal
*
* @brief   Get IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modePtr                  - (pointer to)IPM classification mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgGenIpmClassificationModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      fieldValue;     /* register field value  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    rc = prvCpssHwPpGetRegField(devNum,
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                bridgeRegs.bridgeGlobalConfigRegArray[2],
                    21, 1, &fieldValue);

    if(GT_OK != rc)
    {
        return rc;
    }

    *modePtr = (fieldValue == 0x0) ?
                    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E :
                    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_AND_ETHERTYPE_E;

    return GT_OK;
}

/**
* @internal cpssDxChBrgGenIpmClassificationModeGet function
* @endinternal
*
* @brief   Get IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modePtr                  - (pointer to)IPM classification mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpmClassificationModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgGenIpmClassificationModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgGenIpmClassificationModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

