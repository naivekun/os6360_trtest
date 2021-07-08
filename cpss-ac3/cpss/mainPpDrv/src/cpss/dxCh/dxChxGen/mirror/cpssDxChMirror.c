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
* @file cpssDxChMirror.c
*
* @brief CPSS DxCh Mirror APIs implementation.
*
*
* @version   57
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirrorLog.h>

#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChStc.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirror.h>

/* Convert analyzer port and device to null port and device */
#define CORE_MIRR_CONVERT_ANALYZER_FOR_CPU_OR_NULL_PORT_MAC(                       \
                       _analyzDev, _analyzPort, _ownDevice, _devNullPort)      \
{                                                                              \
    if (_analyzPort == CPSS_CPU_PORT_NUM_CNS)                                  \
    {                                                                          \
        _analyzDev = _ownDevice;                                               \
    }                                                                          \
    else                                                                       \
    if (_analyzPort == CPSS_NULL_PORT_NUM_CNS)                                 \
    {                                                                          \
        _analyzDev = _ownDevice;                                               \
        _analyzPort = _devNullPort;                                            \
    }                                                                          \
}

/**
* @internal internal_cpssDxChMirrorAnalyzerVlanTagEnable function
* @endinternal
*
* @brief   Enable/Disable Analyzer port adding additional VLAN Tag to mirrored
*         packets. The content of this VLAN tag is configured
*         by cpssDxChMirrorTxAnalyzerVlanTagConfig.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ports number to be set
* @param[in] enable                   - GT_TRUE - additional VLAN tag inserted to mirrored packets
*                                      GT_FALSE - no additional VLAN tag inserted to mirrored packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerVlanTagEnable
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  enable
)
{
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* function return code */
    GT_U32      fieldValue;     /* register's field value */
    GT_U32      mirrPort;       /* mirror port*/
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    /* Assign field value */
    fieldValue = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                               portNum,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                               SIP5_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN_E, /* field name */
                                               PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               fieldValue);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        mirrPort = OFFSET_TO_BIT_MAC(localPort);

        /* TO ANALYZER VLAN Adding Configuration Registers */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
            toAnalyzerVlanAddConfig[OFFSET_TO_WORD_MAC(localPort)];

        /* Bit per port indicating if a Vlan Tag is to be added to TO_ANALYZER Packets */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, mirrPort, 1, fieldValue);
    }
    return rc;
}

/**
* @internal cpssDxChMirrorAnalyzerVlanTagEnable function
* @endinternal
*
* @brief   Enable/Disable Analyzer port adding additional VLAN Tag to mirrored
*         packets. The content of this VLAN tag is configured
*         by cpssDxChMirrorTxAnalyzerVlanTagConfig.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ports number to be set
* @param[in] enable                   - GT_TRUE - additional VLAN tag inserted to mirrored packets
*                                      GT_FALSE - no additional VLAN tag inserted to mirrored packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagEnable
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerVlanTagEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChMirrorAnalyzerVlanTagEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorAnalyzerVlanTagEnableGet function
* @endinternal
*
* @brief   Get Analyzer port VLAN Tag to mirrored packets mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ports number to be set
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - additional VLAN tag inserted to mirrored packets
*                                      GT_FALSE - no additional VLAN tag inserted to mirrored packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerVlanTagEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* function return code */
    GT_U32      fieldValue;     /* register's field value */
    GT_U32      mirrPort;       /* mirror port*/
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                               CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                               portNum,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                               SIP5_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN_E, /* field name */
                                               PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               &fieldValue);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        mirrPort = OFFSET_TO_BIT_MAC(localPort);

        /* TO ANALYZER VLAN Adding Configuration Registers */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
            toAnalyzerVlanAddConfig[OFFSET_TO_WORD_MAC(localPort)];

        /* Bit per port indicating if a Vlan Tag is to be added to TO_ANALYZER Packets */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, mirrPort, 1, &fieldValue);
    }
    if(rc != GT_OK)
        return rc;

    /* Assign field value */
    *enablePtr = ((fieldValue != 0) ? GT_TRUE : GT_FALSE);

    return rc;
}

/**
* @internal cpssDxChMirrorAnalyzerVlanTagEnableGet function
* @endinternal
*
* @brief   Get Analyzer port VLAN Tag to mirrored packets mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ports number to be set
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - additional VLAN tag inserted to mirrored packets
*                                      GT_FALSE - no additional VLAN tag inserted to mirrored packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerVlanTagEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChMirrorAnalyzerVlanTagEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxAnalyzerVlanTagConfig
(
    IN GT_U8                                       devNum,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data */
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    CPSS_NULL_PTR_CHECK_MAC(analyzerVlanTagConfigPtr);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(analyzerVlanTagConfigPtr->vpt);
    PRV_CPSS_VLAN_VALUE_CHECK_MAC(analyzerVlanTagConfigPtr->vid);
    if (analyzerVlanTagConfigPtr->cfi > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* VLAN tag data vid+cfi+vpt+ethertype */
    regData = (analyzerVlanTagConfigPtr->vid) |
              (analyzerVlanTagConfigPtr->cfi << 12) |
              (analyzerVlanTagConfigPtr->vpt << 13) |
              (analyzerVlanTagConfigPtr->etherType << 16);

    /* Egress Analyzer VLAN Tag Configuration Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).egrAnalyzerVLANTagConfig;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.egrAnalyzerVlanTagConfig;
    }

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal cpssDxChMirrorTxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerVlanTagConfig
(
    IN GT_U8                                       devNum,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxAnalyzerVlanTagConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerVlanTagConfigPtr));

    rc = internal_cpssDxChMirrorTxAnalyzerVlanTagConfig(devNum, analyzerVlanTagConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerVlanTagConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxAnalyzerVlanTagConfigGet
(
    IN GT_U8                                       devNum,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data */
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    CPSS_NULL_PTR_CHECK_MAC(analyzerVlanTagConfigPtr);


    /* Egress Analyzer VLAN Tag Configuration Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).egrAnalyzerVLANTagConfig;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.egrAnalyzerVlanTagConfig;
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if(rc != GT_OK)
        return rc;

    /* VLAN tag data vid+cfi+vpt+ethertype */
    analyzerVlanTagConfigPtr->vid = (GT_U16)(regData & 0xFFF);
    analyzerVlanTagConfigPtr->cfi = (GT_U8)((regData & 0x1000) >> 12);
    analyzerVlanTagConfigPtr->vpt = (GT_U8)((regData & 0xE000) >> 13);
    analyzerVlanTagConfigPtr->etherType = (GT_U16)((regData & 0xFFFF0000) >> 16);

    return rc;
}

/**
* @internal cpssDxChMirrorTxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerVlanTagConfigGet
(
    IN GT_U8                                       devNum,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxAnalyzerVlanTagConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerVlanTagConfigPtr));

    rc = internal_cpssDxChMirrorTxAnalyzerVlanTagConfigGet(devNum, analyzerVlanTagConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerVlanTagConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxAnalyzerVlanTagConfig
(
    IN GT_U8                                       devNum,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data */
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    CPSS_NULL_PTR_CHECK_MAC(analyzerVlanTagConfigPtr);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(analyzerVlanTagConfigPtr->vpt);
    PRV_CPSS_VLAN_VALUE_CHECK_MAC(analyzerVlanTagConfigPtr->vid);
    if (analyzerVlanTagConfigPtr->cfi > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* VLAN tag data vid+cfi+vpt+ethertype */
    regData = (analyzerVlanTagConfigPtr->vid) |
              (analyzerVlanTagConfigPtr->cfi << 12) |
              (analyzerVlanTagConfigPtr->vpt << 13) |
              (analyzerVlanTagConfigPtr->etherType << 16);

    /* Ingress Analyzer VLAN Tag Configuration Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).ingrAnalyzerVLANTagConfig;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.ingAnalyzerVlanTagConfig;
    }

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal cpssDxChMirrorRxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerVlanTagConfig
(
    IN GT_U8                                       devNum,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxAnalyzerVlanTagConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerVlanTagConfigPtr));

    rc = internal_cpssDxChMirrorRxAnalyzerVlanTagConfig(devNum, analyzerVlanTagConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerVlanTagConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxAnalyzerVlanTagConfigGet
(
    IN GT_U8                                       devNum,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data */
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E);

    CPSS_NULL_PTR_CHECK_MAC(analyzerVlanTagConfigPtr);

    /* Ingress Analyzer VLAN Tag Configuration Register */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).ingrAnalyzerVLANTagConfig;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.ingAnalyzerVlanTagConfig;
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if(rc != GT_OK)
        return rc;

    /* VLAN tag data vid+cfi+vpt+ethertype */
    analyzerVlanTagConfigPtr->vid = (GT_U16)(regData & 0xFFF);
    analyzerVlanTagConfigPtr->cfi = (GT_U8)((regData & 0x1000) >> 12);
    analyzerVlanTagConfigPtr->vpt = (GT_U8)((regData & 0xE000) >> 13);
    analyzerVlanTagConfigPtr->etherType = (GT_U16)((regData & 0xFFFF0000) >> 16);

    return rc;
}

/**
* @internal cpssDxChMirrorRxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerVlanTagConfigGet
(
    IN GT_U8                                       devNum,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxAnalyzerVlanTagConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerVlanTagConfigPtr));

    rc = internal_cpssDxChMirrorRxAnalyzerVlanTagConfigGet(devNum, analyzerVlanTagConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerVlanTagConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a port to be an analyzer port of tx mirrored
*         ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerHwDev            - pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxAnalyzerPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     analyzerPort,
    IN  GT_HW_DEV_NUM   analyzerHwDev
)
{

    GT_U32      regAddr;     /* hw pp memory address */
    GT_U32      data;        /* data for hw write in pp */
    GT_STATUS   rc;          /* function call return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffPortsCfg;

    CORE_MIRR_CONVERT_ANALYZER_FOR_CPU_OR_NULL_PORT_MAC(
            analyzerHwDev, analyzerPort, (PRV_CPSS_HW_DEV_NUM_MAC(devNum)),
            PRV_CPSS_DXCH_NULL_PORT_NUM_CNS);

    data = ((analyzerPort & 0x3f) << 5) |
            (analyzerHwDev & 0x1f);

    /* configure the egress analyzer device and port */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 11, data);

    return rc;
}

/**
* @internal cpssDxChMirrorTxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a port to be an analyzer port of tx mirrored
*         ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerHwDev            - pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     analyzerPort,
    IN  GT_HW_DEV_NUM   analyzerHwDev
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxAnalyzerPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerPort, analyzerHwDev));

    rc = internal_cpssDxChMirrorTxAnalyzerPortSet(devNum, analyzerPort, analyzerHwDev);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerPort, analyzerHwDev));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of tx mirrored ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to)port number of analyzer port
* @param[out] analyzerHwDevPtr         - (pointer to)pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxAnalyzerPortGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *analyzerPortPtr,
    OUT GT_HW_DEV_NUM   *analyzerHwDevPtr
)
{
    GT_U32      regAddr;     /* hw pp memory address */
    GT_U32      data;        /* data for hw write in pp */
    GT_STATUS   rc;          /* function call return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(analyzerPortPtr);
    CPSS_NULL_PTR_CHECK_MAC(analyzerHwDevPtr);

    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffPortsCfg;

    /* get configuration of the egress analyzer device and port */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 11, &data);

    if(rc != GT_OK)
        return rc;


    *analyzerPortPtr = ((data & 0x7E0) >> 5);
    *analyzerHwDevPtr = (data & 0x1f);

    return rc;

}

/**
* @internal cpssDxChMirrorTxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of tx mirrored ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to)port number of analyzer port
* @param[out] analyzerHwDevPtr         - (pointer to)pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerPortGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *analyzerPortPtr,
    OUT GT_HW_DEV_NUM   *analyzerHwDevPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxAnalyzerPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerPortPtr, analyzerHwDevPtr));

    rc = internal_cpssDxChMirrorTxAnalyzerPortGet(devNum, analyzerPortPtr, analyzerHwDevPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerPortPtr, analyzerHwDevPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerHwDev            - pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxAnalyzerPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     analyzerPort,
    IN  GT_HW_DEV_NUM   analyzerHwDev
)
{

    GT_U32      regAddr;     /* hw pp memory address */
    GT_U32      data;        /* data for hw write in pp */
    GT_STATUS   rc;          /* function call return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffPortsCfg;

    CORE_MIRR_CONVERT_ANALYZER_FOR_CPU_OR_NULL_PORT_MAC(
            analyzerHwDev, analyzerPort, (PRV_CPSS_HW_DEV_NUM_MAC(devNum)),
            PRV_CPSS_DXCH_NULL_PORT_NUM_CNS);

    data = ((analyzerPort & 0x3f) << 5) |
            (analyzerHwDev & 0x1f);


    /* configure the ingress analyzer device and port */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 11, data);

    return rc;
}

/**
* @internal cpssDxChMirrorRxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerHwDev            - pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     analyzerPort,
    IN  GT_HW_DEV_NUM   analyzerHwDev
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxAnalyzerPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerPort, analyzerHwDev));

    rc = internal_cpssDxChMirrorRxAnalyzerPortSet(devNum, analyzerPort, analyzerHwDev);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerPort, analyzerHwDev));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of Rx mirrored ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to)port number of analyzer port
* @param[out] analyzerHwDevPtr         - (pointer to)pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxAnalyzerPortGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *analyzerPortPtr,
    OUT GT_HW_DEV_NUM   *analyzerHwDevPtr
)
{
    GT_U32      regAddr;     /* hw pp memory address */
    GT_U32      data;        /* data for hw write in pp */
    GT_STATUS   rc;          /* function call return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(analyzerPortPtr);
    CPSS_NULL_PTR_CHECK_MAC(analyzerHwDevPtr);

    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffPortsCfg;

    /* get configuration of the ingress analyzer device and port */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 11, 11, &data);

    if(rc != GT_OK)
        return rc;


    *analyzerPortPtr = ((data & 0x7E0) >> 5);
    *analyzerHwDevPtr = (data & 0x1f);

    return rc;
}

/**
* @internal cpssDxChMirrorRxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of Rx mirrored ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to)port number of analyzer port
* @param[out] analyzerHwDevPtr         - (pointer to)pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerPortGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *analyzerPortPtr,
    OUT GT_HW_DEV_NUM   *analyzerHwDevPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxAnalyzerPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerPortPtr, analyzerHwDevPtr));

    rc = internal_cpssDxChMirrorRxAnalyzerPortGet(devNum, analyzerPortPtr, analyzerHwDevPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerPortPtr, analyzerHwDevPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxCascadeMonitorEnable function
* @endinternal
*
* @brief   One global bit that is set to 1 when performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] enable                   - enable/disable Egress Monitoring on cascading ports.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxCascadeMonitorEnable
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{

    GT_U32      regAddr, regData; /* pp memory address and data for hw access*/
    GT_STATUS   rc;               /* function call return value            */
    GT_U32      bitNum;           /* bit number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    regData = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.cscdEgressMonitoringEnableConfReg;
        bitNum = 0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.trSrcSniff;
        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            /* Cheetah 2 and above devices */
            bitNum = 29;
        }
        else
        {
            /* Cheetah and Cheetah+ devices */
            bitNum = 28;
        }

    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, bitNum, 1, regData);

    return rc;
}

/**
* @internal cpssDxChMirrorTxCascadeMonitorEnable function
* @endinternal
*
* @brief   One global bit that is set to 1 when performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] enable                   - enable/disable Egress Monitoring on cascading ports.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxCascadeMonitorEnable
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxCascadeMonitorEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChMirrorTxCascadeMonitorEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxCascadeMonitorEnableGet function
* @endinternal
*
* @brief   Get One global bit that indicate performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] enablePtr                - enable/disable Egress Monitoring on cascading ports.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxCascadeMonitorEnableGet
(
    IN   GT_U8   devNum,
    OUT  GT_BOOL *enablePtr
)
{

    GT_U32      regAddr, regData; /* pp memory address and data for hw access*/
    GT_STATUS   rc;               /* function call return value            */
    GT_U32      bitNum;           /* bit number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.cscdEgressMonitoringEnableConfReg;
        bitNum = 0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.trSrcSniff;
        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            /* Cheetah 2 and above devices */
            bitNum = 29;
        }
        else
        {
            /* Cheetah and Cheetah+ devices */
            bitNum = 28;
        }
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bitNum, 1, &regData);
    if(rc != GT_OK)
        return rc;

    *enablePtr = (regData == 0x1) ? GT_TRUE : GT_FALSE;


    return rc;
}

/**
* @internal cpssDxChMirrorTxCascadeMonitorEnableGet function
* @endinternal
*
* @brief   Get One global bit that indicate performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] enablePtr                - enable/disable Egress Monitoring on cascading ports.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxCascadeMonitorEnableGet
(
    IN   GT_U8   devNum,
    OUT  GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxCascadeMonitorEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChMirrorTxCascadeMonitorEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal prvCpssDxChMirrorPortIndexRegDataCalculate function
* @endinternal
*
* @brief   Calculate start bit and Port Index Register index
*         according to the port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number.
* @param[in] rxMirrorEnable           - GT_TRUE - Calculate  start bit and
*                                      Port Index Register index for ingress mirror port.
*                                      - GT_FALSE - Calculate  start bit and
*                                      Port Index Register index for egress mirror port.
*
* @param[out] startBitPtr              - pointer to start bit.
* @param[out] regIndexPtr              - pointer to register index.
*                                       None.
*/
static GT_VOID prvCpssDxChMirrorPortIndexRegDataCalculate
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 rxMirrorEnable,
    OUT GT_U32                  *startBitPtr,
    OUT GT_U32                  *regIndexPtr
)
{
    /* Note: for RX this function expect 'local port' (to port group) */
    /* Note: for TX this function expect 'global port' (to device)    */

    if(((0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum)) ||
       (rxMirrorEnable == GT_TRUE)) && (portNum == CPSS_CPU_PORT_NUM_CNS))
    {
        /* for RX : CPU port - for all devices          */
        /* for TX : CPU port - for ch1,diamond,2,3,xcat */
        portNum = 28;/* lead to register 2 bit 24..26 */
    }

    /*************************************************/
    /* for RX : 3  registers .. 10 ports in register */
    /*************************************************/

    /********************************************************************************/
    /* for TX : 14 registers .. 10 ports in register .. lower 64 in registers 0..6  */
    /*                                               .. next  64 in registers 7..13 */
    /*    ch1,diamond,2,3,xcat : 3 registers --> supported like RX register         */
    /*    Lion , xcat2 : 7 registers --> support lower 64 ports                     */
    /*    Lion2        : 14 registers --> support 128 ports                         */
    /********************************************************************************/

    *startBitPtr = ((portNum & 0x3f) % 10) * 3;
    *regIndexPtr = ((portNum & 0x3f) / 10) + (((portNum & BIT_6) >> 6) * 7);
}

/**
* @internal prvCpssDxChMirrorToAnalyzerForwardingModeGet function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - pointer to mode of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChMirrorToAnalyzerForwardingModeGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
)
{
    GT_U32      regAddr;      /* register address */
    GT_U32      regData;      /* register data */
    GT_STATUS   rc;           /* return status */

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 2, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        switch(regData)
        {
            case 0:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E ;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Unsupported value[%d]",regData);
        }
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(regData)
        {
            case 0:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E ;
                break;
            case 1:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E ;
                break;
            case 2:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E ;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Unsupported value[%d]",regData);
        }
    }
    else
    {
        switch(regData)
        {
            case 0:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E ;
                break;
            case 3:
                *modePtr = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChMirrorRxPortSet function
* @endinternal
*
* @brief   Sets a specific ePort to be Rx mirrored port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] enable                   - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static GT_STATUS prvCpssDxChMirrorRxPortSet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  index
)
{
    GT_U32      data;       /* data set to hw */

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, mirrPort);

    if(index > CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Set index to Analyzer interface for the ePort */
    if(enable == GT_TRUE)
    {
        data = index + 1;
    }
    else /* No mirroring for the ePort */
    {
        data = 0;
    }

    /* set EQ-ingress-eport table */
    return prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E,
                                       mirrPort,
                                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       4, /* start bit */
                                       3, /* 3 bit */
                                       data);
}

/**
* @internal internal_cpssDxChMirrorRxPortSet function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    IN  GT_BOOL         enable,
    IN  GT_U32          index
)
{
    GT_U32      data;        /* data from Ports VLAN and QoS table entry */
    GT_U32      offset;      /* offset in VLAN and QoS table entry */
    GT_STATUS   rc = GT_OK;  /* function call return value */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode; /* forwarding mode */
    GT_U32      regIndex; /* the index of Port Ingress Mirror Index register */
    GT_U32      regAddr;  /* pp memory address for hw access*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* device support physical port mirroring only */
        isPhysicalPort = GT_TRUE;
    }

    if (isPhysicalPort == GT_FALSE)
    {
        /* configure ePort mirroring */
        return prvCpssDxChMirrorRxPortSet(devNum, mirrPort, enable, index);
    }

    /* configure physical port mirroring */
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,mirrPort);

    if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Get Analyzer forwarding mode */
        rc = prvCpssDxChMirrorToAnalyzerForwardingModeGet(devNum, &mode);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Source-based-forwarding mode. */
        if((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)||
           (mode != CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E))
        {
            if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            /* Set index to Analyzer interface for the port */
            if(enable == GT_TRUE)
            {
                data = index + 1;
            }
            /* No mirroring for the port */
            else
            {
                data = 0;
            }

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                offset = (mirrPort % 8) * 4;/* 8 ports in entry , in steps of 4 bits*/

                rc = prvCpssDxChWriteTableEntryField(devNum,
                                                    CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E,
                                                    (mirrPort >> 3), /* global port / 8 */
                                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                    offset,
                                                    3,
                                                    data);
            }
            else
            {
                /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
                portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, mirrPort);
                /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,mirrPort);

                prvCpssDxChMirrorPortIndexRegDataCalculate(devNum, localPort, GT_TRUE,
                                                           &offset, &regIndex);

                /* getting register address */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bufferMng.eqBlkCfgRegs.portRxMirrorIndex[regIndex];

                /* Set Analyzer destination interface index  */
                rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, offset,
                                                     3, data);
            }

            if(rc != GT_OK)
            {
                return rc;
            }
        }
        offset = 7;
    }
    else
    {
        offset = 23;
    }

    /* For xCat and above hop-by-hop forwarding mode and other DxCh devices */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        data = BOOL2BIT_MAC(enable);

        /* configure the Ports VLAN and QoS configuration entry,
           enable MirrorToIngressAnalyzerPort field */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            mirrPort,
                                            0,
                                            offset,
                                            1,
                                            data);
    }

    return rc;
}

/**
* @internal cpssDxChMirrorRxPortSet function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based. Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    IN  GT_BOOL         enable,
    IN  GT_U32          index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mirrPort, isPhysicalPort, enable, index));

    rc = internal_cpssDxChMirrorRxPortSet(devNum, mirrPort, isPhysicalPort, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mirrPort, isPhysicalPort, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal prvCpssDxChMirrorRxPortGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific ePort
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
*
* @param[out] enablePtr                - (pointer to) Rx mirror mode
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index.
*                                      Supported for xCat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*                                        GT_TRUE - Rx mirroring enabled, packets
*                                                  received on a mirrPort are
*                                                  mirrored to Rx analyzer.
*                                        GT_FALSE - Rx mirroring disabled.
*                                        Supported for xCat and above device.
*/
static GT_STATUS prvCpssDxChMirrorRxPortGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *indexPtr
)
{
    GT_STATUS   rc;    /* function return code */
    GT_U32      data;  /* data read */

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,mirrPort);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    /* get EQ-ingress-eport table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E,
                                       mirrPort,
                                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       4, /* start bit */
                                       3, /* 3 bit */
                                       &data);

    if(rc != GT_OK)
        return rc;

    /* No mirroring for the ePort */
    if(!data)
    {
        *enablePtr = GT_FALSE;
    }
    /* Get index to Analyzer interface for the ePort */
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = data - 1;
    }

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxPortGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Rx mirror mode
*                                        GT_TRUE - Rx mirroring enabled, packets
*                                                  received on a mirrPort are
*                                                  mirrored to Rx analyzer.
*                                        GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index.
*                                        Supported for xCat and above device.
*                                        In xCat3 the parameter is used only if
*                                        forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    OUT GT_BOOL         *enablePtr,
    OUT GT_U32          *indexPtr
)
{
    GT_U32    data;     /* HW data */
    GT_U32    offset;   /* field offset */
    GT_STATUS rc;       /* function call return value */
    GT_U32  regIndex;   /* the index of Port Ingress Mirror Index register */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode; /* forwarding mode */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;  /* local port - support multi-port-groups device */
    GT_U32  regAddr;    /* pp memory address for hw access*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* device support physical port mirroring only */
        isPhysicalPort = GT_TRUE;
    }

    if (isPhysicalPort == GT_FALSE)
    {
        /* get ePort mirroring configuration */
        return prvCpssDxChMirrorRxPortGet(devNum, mirrPort, enablePtr, indexPtr);
    }

    /* get phisical port mirroring configuration */
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, mirrPort);

    if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Get Analyzer forwarding mode */
        rc = prvCpssDxChMirrorToAnalyzerForwardingModeGet(devNum, &mode);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Source-based-forwarding mode. */
        if((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)||
           (mode != CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E))
        {
            CPSS_NULL_PTR_CHECK_MAC(indexPtr);

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                offset = (mirrPort % 8) * 4;/* 8 ports in entry , in steps of 4 bits*/

                rc = prvCpssDxChReadTableEntryField(devNum,
                                                    CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E,
                                                    (mirrPort >> 3), /* global port / 8 */
                                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                    offset,
                                                    3,
                                                    &data);
            }
            else
            {
                /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
                portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, mirrPort);
                /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,mirrPort);

                prvCpssDxChMirrorPortIndexRegDataCalculate(devNum, localPort, GT_TRUE,
                                                           &offset, &regIndex);

                /* getting register address */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bufferMng.eqBlkCfgRegs.portRxMirrorIndex[regIndex];

                /* Get Analyzer destination interface index  */
                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, offset,
                                               3, &data);
            }

            if(rc != GT_OK)
            {
                return rc;
            }

            /* No mirroring for the port */
            if(!data)
            {
                *enablePtr = GT_FALSE;
            }
            /* Get index to Analyzer interface for the port */
            else
            {
                *enablePtr = GT_TRUE;
                *indexPtr = data - 1;
            }

            return GT_OK;
        }
        /* xCat and above hop-by-hop forwarding mode */
        else
        {
            offset = 7;
        }
    }
    else
    {
        offset = 23;
    }

    /* For xCat and above hop-by-hop forwarding mode and other DxCh devices */
    /* Get configuration MirrorToIngressAnalyzerPort field */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                        mirrPort,
                                        0,
                                        offset,
                                        1,
                                        &data);
    if(rc != GT_OK)
        return rc;

    *enablePtr = (data != 0) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChMirrorRxPortGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Rx mirror mode
*                                        GT_TRUE - Rx mirroring enabled, packets
*                                                  received on a mirrPort are
*                                                  mirrored to Rx analyzer.
*                                        GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index.
*                                        Supported for xCat and above device.
*                                        In xCat3 the parameter is used only if
*                                        forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    OUT GT_BOOL         *enablePtr,
    OUT GT_U32          *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mirrPort, isPhysicalPort, enablePtr, indexPtr));

    rc = internal_cpssDxChMirrorRxPortGet(devNum, mirrPort, isPhysicalPort, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mirrPort, isPhysicalPort, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChMirrorTxPortSet function
* @endinternal
*
* @brief   Enable or disable Tx mirroring per Eport.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number.
* @param[in] enable                   - enable/disable Tx mirror on this ePort
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChMirrorTxPortSet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  index
)
{
    GT_STATUS   rc;
    GT_U32      data;       /* data set to hw */

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, mirrPort);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Set index to Analyzer interface for the ePort */
    if(enable == GT_TRUE)
    {
        data = index + 1;
    }
    /* No mirroring for the port */
    else
    {
        data = 0;
    }

    /* NOTE: EQ not need <MDB> issues */


    rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    mirrPort,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   data);
    return rc;
}

/**
* @internal prvCpssDxChMirrorTxPortGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per ePort .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number.
*
* @param[out] enablePtr                - (pointer to) Tx mirror mode on this eport
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChMirrorTxPortGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *indexPtr
)
{

    GT_STATUS   rc;   /* function return code */
    GT_U32      data; /* data read */

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, mirrPort);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    mirrPort,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &data);

    /* No mirroring for the ePort */
    if(!data)
    {
        *enablePtr = GT_FALSE;
    }
    /* Get index to Analyzer interface for the ePort */
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = data - 1;
    }

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxPortSet function
* @endinternal
*
* @brief   Enable or disable Tx mirroring per port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Tx mirror on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based. Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxPortSet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    IN  GT_BOOL                 isPhysicalPort,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  index
)
{
    GT_STATUS   rc;     /* function call return value              */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  regIndex;   /* the index of Port Egress Mirror Index register */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode; /* forwarding mode */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  regAddr1, regAddr2 = 0, regAddr3; /* pp memory address for hw access*/
    GT_U32  startBit1,startBit2 = 0, startBit3; /* bits to start to write data for registers 1 & 2 & 3*/
    GT_U32  regData2 = 0;  /* register2 data */
    GT_U32  stcLimit;   /* Egress STC limit */
    GT_BOOL readBeforeTxQWrite; /* read previous register before TxQ register write */
    GT_U32  readPortGroupId; /* port group id iterator for read after write */
    GT_U32  readRegData;     /* register data */
    GT_U32  analyzerIndex;/* analyzer index - xcat and above */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* check index parameter for E_ARCH devices */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* device support physical port mirroring only */
        isPhysicalPort = GT_TRUE;
    }

    if (isPhysicalPort == GT_FALSE)
    {
        /* configure ePort mirroring */
        return prvCpssDxChMirrorTxPortSet(devNum, mirrPort, enable, index);
    }

    if(enable == GT_TRUE)
    {
        analyzerIndex = index + 1;
    }
    /* No mirroring for the port */
    else
    {
        analyzerIndex = 0;
    }

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, mirrPort);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortGroupWriteTableEntryField(
            devNum,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
            mirrPort,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            analyzerIndex);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, mirrPort);
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,mirrPort);

    /* check if need to set 'per port' in the EQ unit */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE &&
       PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Get Analyzer forwarding mode */
        rc = prvCpssDxChMirrorToAnalyzerForwardingModeGet(devNum, &mode);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Source-based-forwarding mode. */
        if(mode != CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
        {
            if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            prvCpssDxChMirrorPortIndexRegDataCalculate(devNum, mirrPort, GT_FALSE,
                                                       &startBit2, &regIndex);

            /* getting register address */
            regAddr2 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.eqBlkCfgRegs.portTxMirrorIndex[regIndex];

            regData2 = analyzerIndex;
        }
    }

    readBeforeTxQWrite = GT_FALSE;
    /* configure EgressMirrorToAnalyzerEn field in Port<n> Txq
       Configuration Register */
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_TXQ_REG_MAC(devNum,mirrPort,&regAddr1);
        startBit1 = 20;

    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr1 = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrAnalyzerEnableIndex[(mirrPort>>3)];
        }
        else
        {
            regAddr1 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.
                statisticalAndCpuTriggeredEgressMirroringToAnalyzerPort.
                egressAnalyzerEnable;
        }

        startBit1 = localPort;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            startBit1 = (mirrPort % 8) * 3;
        }
        else
        if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            startBit1 &= 0xf;
            readBeforeTxQWrite = GT_TRUE;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* in eArch this per physical is not used */
        regAddr3 = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        startBit3 = 0;
    }
    else
    {
        regAddr3 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.trSrcSniff;

        startBit3 = mirrPort % 32;

        if(PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
        {
            regAddr3 += 0x64 * (mirrPort >> 5);
        }
        else
        {
            if (mirrPort  == CPSS_CPU_PORT_NUM_CNS)
            {
                /* Determine CPU port bit offset */
                /* Cheetah 2,3.. devices */
                if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
                {
                    startBit3 = 28;
                }
                else /* Cheetah and Cheetah+ devices */
                {
                    startBit3 = 27;
                }
            }
        }
    }

    /* For CPU port STC is not relevant, therefore for CPU port STC is considered as */
    /* disabled by forcing the limit indication to 0. */
    if (mirrPort  == CPSS_CPU_PORT_NUM_CNS)
    {
        stcLimit = 0;
    }
    else
    {
        /* the mirrPort is 'physical' port so no problem calling API of physical port */
        rc = cpssDxChStcPortLimitGet(devNum, mirrPort, CPSS_DXCH_STC_EGRESS_E, &stcLimit);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    if(enable == GT_TRUE)
    {

        /* Prior to configuring any non-cascade port configured for egress mirroring,
           for source based mirroring set Port Egress Mirror Index,
           for hop-by-hop mirroring set the corresponding bit
           in the <PortEgress MonitorEn[28:0]> field of the
           Egress Monitoring Enable Configuration Register.
           The Egress Monitoring Enable Configuration Register be set according
           to the global port number and must be set in all the cores.
        */

        /* If Egress STC is disabled on port enable Egress monitoring on port. */
        if( 0 == stcLimit )
        {
            if(regAddr3 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = prvCpssHwPpSetRegField(devNum, regAddr3, startBit3, 1, 1);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* In multi port group devices need to guaranty that all registers
                   were updated before TxQ register write. Because EQ registers
                   are per port group but TxQ register is only one in device.
                   This is possible that EQ registers in some port groups are
                   updated after TxQ. Need to avoid such problem by read EQ
                   register after write to TxQ one. */
                if (readBeforeTxQWrite != GT_FALSE)
                {
                    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,readPortGroupId)
                    {
                        rc = prvCpssHwPpPortGroupReadRegister (devNum, readPortGroupId, regAddr3, &readRegData);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,readPortGroupId)
                }
            }
        }

        if( regAddr2 != 0 )
        {
            rc = prvCpssHwPpSetRegField(devNum, regAddr2,
                                           startBit2, 3, regData2);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (readBeforeTxQWrite != GT_FALSE)
            {
                PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,readPortGroupId)
                {
                    rc = prvCpssHwPpPortGroupReadRegister (devNum, readPortGroupId, regAddr2, &readRegData);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,readPortGroupId)
            }
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr1, startBit1, 3, analyzerIndex);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr1, startBit1, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        /* The order must be opposite from above. */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr1, startBit1, 3, analyzerIndex);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr1, startBit1, 1, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* read TxQ register to guaranty that it was updated before EQ registers change. */
        if (readBeforeTxQWrite != GT_FALSE)
        {
            rc = prvCpssHwPpPortGroupReadRegister (devNum, portGroupId, regAddr1, &readRegData);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if( regAddr2 != 0 )
        {
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr2,
                                                    startBit2, 3, regData2);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* If Egress STC is disabled on port disable Egress monitoring on port. */
        if( 0 == stcLimit )
        {
            if(regAddr3 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = prvCpssHwPpSetRegField(devNum, regAddr3, startBit3, 1, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChMirrorTxPortSet function
* @endinternal
*
* @brief   Enable or disable Tx mirroring per port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Tx mirror on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based. Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortSet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    IN  GT_BOOL                 isPhysicalPort,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mirrPort, isPhysicalPort, enable, index));

    rc = internal_cpssDxChMirrorTxPortSet(devNum, mirrPort, isPhysicalPort, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mirrPort, isPhysicalPort, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChMirrorTxPortGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Tx mirror mode on this port
*                                        GT_TRUE - Tx mirroring enabled, packets
*                                                  transmitted from a mirrPort are
*                                                  mirrored to Tx analyzer.
*                                        GT_FALSE - Tx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index.
*                                        Supported for xCat and above device.
*                                        In xCat3 the parameter is used only if
*                                        forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxPortGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    IN  GT_BOOL                 isPhysicalPort,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *indexPtr
)
{

    GT_U32      regAddr;
    GT_STATUS   rc;
    GT_U32      value=0;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  regIndex;   /* the index of Port Egress Mirror Index register */
    GT_U32  regData;    /* register data */
    GT_U32  startBit , startBit1;       /* bit to start to write data */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT mode; /* forwarding mode */
    GT_U32   localPort; /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* device support physical port mirroring only */
        isPhysicalPort = GT_TRUE;
    }

    if (isPhysicalPort == GT_FALSE)
    {
        /* get ePort mirroring configuration */
        return prvCpssDxChMirrorTxPortGet(devNum, mirrPort, enablePtr, indexPtr);
    }

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, mirrPort);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChPortGroupReadTableEntryField(
            devNum,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
            mirrPort,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &value);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (value == 0)
        {
            *enablePtr = GT_FALSE;
            *indexPtr = 0;
        }
        else
        {
            *enablePtr = GT_TRUE;
            *indexPtr = value - 1;
        }
        return GT_OK;
    }


    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, mirrPort);
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,mirrPort);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_TXQ_REG_MAC(devNum,mirrPort,&regAddr);
        startBit1 = 20;
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrAnalyzerEnableIndex[(mirrPort>>3)];
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.
                statisticalAndCpuTriggeredEgressMirroringToAnalyzerPort.
                egressAnalyzerEnable;
        }

        startBit1 = localPort;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            startBit1  = (mirrPort % 8) * 3;
        }
        else
        if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            startBit1 &= 0xf;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, startBit1, 3, &value);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, startBit1, 1, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(value != 0)
    {
        *enablePtr = GT_TRUE;
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            *indexPtr = value - 1;
        }
        else
        if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* Get Analyzer destination interface index for xCat only, if
               Forwarding mode is Source-based. */

            /* Get Analyzer forwarding mode */
            rc = prvCpssDxChMirrorToAnalyzerForwardingModeGet(devNum, &mode);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* Source-based-forwarding mode. */
            if(mode != CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
            {
                prvCpssDxChMirrorPortIndexRegDataCalculate(devNum, mirrPort, GT_FALSE,
                                                           &startBit, &regIndex);

                /* getting register address */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bufferMng.eqBlkCfgRegs.portTxMirrorIndex[regIndex];

                /* Get Analyzer destination interface index  */
                rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit,
                                                 3, &regData);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* No mirroring for the port */
                if(!regData)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
                /* Get index to Analyzer interface for the port */
                else
                {
                    *indexPtr = regData - 1;
                }
            }
        }
    }
    else
    {
        *enablePtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal cpssDxChMirrorTxPortGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Tx mirror mode on this port
*                                        GT_TRUE - Tx mirroring enabled, packets
*                                                  transmitted from a mirrPort are
*                                                  mirrored to Tx analyzer.
*                                        GT_FALSE - Tx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index.
*                                        Supported for xCat and above device.
*                                        In xCat3 the parameter is used only if
*                                        forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             mirrPort,
    IN  GT_BOOL                 isPhysicalPort,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mirrPort, isPhysicalPort, enablePtr, indexPtr));

    rc = internal_cpssDxChMirrorTxPortGet(devNum, mirrPort, isPhysicalPort, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mirrPort, isPhysicalPort, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrRxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] ratio                    - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
static GT_STATUS internal_cpssDxChMirrRxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   ratio
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);
    if(ratio >= BIT_11)/* 11 bits in hw */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.trapSniffed;
    }

    /* writing <IngressStatMirroringToAnalyzerPortRatio> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 11, ratio);

    return rc;
}

/**
* @internal cpssDxChMirrRxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] ratio                    - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrRxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   ratio
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrRxStatMirroringToAnalyzerRatioSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ratio));

    rc = internal_cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(devNum, ratio);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ratio));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrRxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Rx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] ratioPtr                 - (pointer to) Indicates the ratio of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
static GT_STATUS internal_cpssDxChMirrRxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *ratioPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;
    GT_U32      value=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(ratioPtr);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.trapSniffed;
    }

    /* writing <IngressStatMirroringToAnalyzerPortRatio> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 11, &value);
    if(rc != GT_OK)
        return rc;

    *ratioPtr = value;

    return rc;
}

/**
* @internal cpssDxChMirrRxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Rx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] ratioPtr                 - (pointer to) Indicates the ratio of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrRxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *ratioPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrRxStatMirroringToAnalyzerRatioGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ratioPtr));

    rc = internal_cpssDxChMirrRxStatMirroringToAnalyzerRatioGet(devNum, ratioPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ratioPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrRxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Ingress Statistical Mirroring to the Ingress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - enable\disable Ingress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrRxStatMirrorToAnalyzerEnable
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.trapSniffed;
    }


    /* enable/disable IngressStatMirroringToAnalyzerPortEn
       Ingress Statistic Mirroring to Analyzer Port Configuration Register */
    regData = (enable == 1) ? 1 : 0;

    /* writing <IngressStatMirroringToAnalyzerPortEn> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 1, regData);

    return rc;
}

/**
* @internal cpssDxChMirrRxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Ingress Statistical Mirroring to the Ingress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - enable\disable Ingress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxStatMirrorToAnalyzerEnable
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrRxStatMirrorToAnalyzerEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChMirrRxStatMirrorToAnalyzerEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrRxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get Ingress Statistical Mirroring to the Ingress Analyzer Port Mode
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Ingress Statistical Mirroring mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrRxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.trapSniffed;
    }

    /* getting <IngressStatMirroringToAnalyzerPortEn> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 11, 1, &regData);
    if(rc != GT_OK)
        return rc;

    /* enable/disable IngressStatMirroringToAnalyzerPortEn
       Ingress Statistic Mirroring to Analyzer Port Configuration Register */
    *enablePtr = ((regData == 1) ? GT_TRUE : GT_FALSE);

    return rc;
}

/**
* @internal cpssDxChMirrRxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get Ingress Statistical Mirroring to the Ingress Analyzer Port Mode
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Ingress Statistical Mirroring mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrRxStatMirrorToAnalyzerEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChMirrRxStatMirrorToAnalyzerEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrRxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrRxAnalyzerDpTcSet
(
    IN GT_U8             devNum,
    IN CPSS_DP_LEVEL_ENT analyzerDp,
    IN GT_U8             analyzerTc
)
{
    GT_U32      regData;
    GT_U32      regAddr;
    GT_U32      dp;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, analyzerDp, dp);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(analyzerTc);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrAndEgrMonitoringToAnalyzerQoSConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffQosCfg;
    }

    /* setting data */
    regData = (dp | (analyzerTc << 2));

    /* writing data to register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 5, 5, regData);

    return rc;
}

/**
* @internal cpssDxChMirrRxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxAnalyzerDpTcSet
(
    IN GT_U8             devNum,
    IN CPSS_DP_LEVEL_ENT analyzerDp,
    IN GT_U8             analyzerTc
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrRxAnalyzerDpTcSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerDp, analyzerTc));

    rc = internal_cpssDxChMirrRxAnalyzerDpTcSet(devNum, analyzerDp, analyzerTc);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerDp, analyzerTc));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrRxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerDpPtr            - pointer to the Drop Precedence.
* @param[out] analyzerTcPtr            - pointer to traffic class on analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrRxAnalyzerDpTcGet
(
    IN GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
)
{
    GT_U32      regData;
    GT_U32      regAddr;
    GT_U32      dp;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(analyzerDpPtr);
    CPSS_NULL_PTR_CHECK_MAC(analyzerTcPtr);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrAndEgrMonitoringToAnalyzerQoSConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffQosCfg;
    }

    /* reading data from register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 5, 5, &regData);

    /* setting data */
    dp = (regData & 0x3);

    PRV_CPSS_DXCH_COS_DP_TO_SW_CHECK_AND_CONVERT_MAC(
        devNum, dp, (*analyzerDpPtr));

    *analyzerTcPtr = (GT_U8)((regData >> 2 ) & 0x7);

    return rc;
}

/**
* @internal cpssDxChMirrRxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerDpPtr            - pointer to the Drop Precedence.
* @param[out] analyzerTcPtr            - pointer to traffic class on analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxAnalyzerDpTcGet
(
    IN GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrRxAnalyzerDpTcGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerDpPtr, analyzerTcPtr));

    rc = internal_cpssDxChMirrRxAnalyzerDpTcGet(devNum, analyzerDpPtr, analyzerTcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerDpPtr, analyzerTcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrTxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to egress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrTxAnalyzerDpTcSet
(
    IN GT_U8              devNum,
    IN CPSS_DP_LEVEL_ENT  analyzerDp,
    IN GT_U8              analyzerTc
)
{
    GT_U32      regData;
    GT_U32      regAddr;
    GT_U32      dp;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, analyzerDp, dp);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(analyzerTc);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrAndEgrMonitoringToAnalyzerQoSConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffQosCfg;
    }

    regData = (dp | (analyzerTc << 2));

    /* writing data to register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 5, regData);

    return rc;
}

/**
* @internal cpssDxChMirrTxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to egress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxAnalyzerDpTcSet
(
    IN GT_U8              devNum,
    IN CPSS_DP_LEVEL_ENT  analyzerDp,
    IN GT_U8              analyzerTc
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrTxAnalyzerDpTcSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerDp, analyzerTc));

    rc = internal_cpssDxChMirrTxAnalyzerDpTcSet(devNum, analyzerDp, analyzerTc);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerDp, analyzerTc));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrTxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to egress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerDpPtr            - pointer to the Drop Precedence.
* @param[out] analyzerTcPtr            - pointer to traffic class on analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrTxAnalyzerDpTcGet
(
    IN GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
)
{
    GT_U32      regData;
    GT_U32      regAddr;
    GT_U32      dp;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(analyzerDpPtr);
    CPSS_NULL_PTR_CHECK_MAC(analyzerTcPtr);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.ingrAndEgrMonitoringToAnalyzerQoSConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.eqBlkCfgRegs.sniffQosCfg;
    }

    /* reading data from register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 5, &regData);

    /* setting data */
    dp = (regData & 0x3);

    PRV_CPSS_DXCH_COS_DP_TO_SW_CHECK_AND_CONVERT_MAC(
        devNum, dp, (*analyzerDpPtr));

    *analyzerTcPtr = (GT_U8)((regData >> 2 ) & 0x7);

    return rc;
}

/**
* @internal cpssDxChMirrTxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to egress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerDpPtr            - pointer to the Drop Precedence.
* @param[out] analyzerTcPtr            - pointer to traffic class on analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxAnalyzerDpTcGet
(
    IN GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrTxAnalyzerDpTcGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerDpPtr, analyzerTcPtr));

    rc = internal_cpssDxChMirrTxAnalyzerDpTcGet(devNum, analyzerDpPtr, analyzerTcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerDpPtr, analyzerTcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrTxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Tx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] ratio                    - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
*
* @note Examples:
*       ratio 0 -> no Tx mirroring
*       ratio 1 -> all Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       ratio 10 -> 1 in 10 Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
static GT_STATUS internal_cpssDxChMirrTxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   ratio
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);

    if(ratio > 2047)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCStatisticalTxSniffConfig;
    }
    /* getting register address */
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.trStatSniffAndStcReg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.
            statisticalAndCpuTriggeredEgressMirroringToAnalyzerPort.stcStatisticalTxSniffConfig;
    }

    /* writing <EgressStatMirroringToAnalyzerPortRatio> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 5, 11, ratio);

    return rc;
}

/**
* @internal cpssDxChMirrTxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Tx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] ratio                    - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
*
* @note Examples:
*       ratio 0 -> no Tx mirroring
*       ratio 1 -> all Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       ratio 10 -> 1 in 10 Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrTxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   ratio
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrTxStatMirroringToAnalyzerRatioSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ratio));

    rc = internal_cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(devNum, ratio);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ratio));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChMirrTxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Tx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] ratioPtr                 - (pointer to)Indicates the ratio of egress mirrored to analyzer
*                                      port packets forwarded to the analyzer port. 1 of every 'ratio'
*                                      packets are forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Tx mirroring
*       ratio 1 -> all Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       ratio 10 -> 1 in 10 Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
static GT_STATUS internal_cpssDxChMirrTxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *ratioPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;
    GT_U32      value=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(ratioPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCStatisticalTxSniffConfig;
    }
    /* getting register address */
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.trStatSniffAndStcReg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.
            statisticalAndCpuTriggeredEgressMirroringToAnalyzerPort.stcStatisticalTxSniffConfig;
    }

    /* getting <EgressStatMirroringToAnalyzerPortRatio> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 5, 11, &value);
    if(rc != GT_OK)
        return rc;

    *ratioPtr = value;

    return rc;
}

/**
* @internal cpssDxChMirrTxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Tx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] ratioPtr                 - (pointer to)Indicates the ratio of egress mirrored to analyzer
*                                      port packets forwarded to the analyzer port. 1 of every 'ratio'
*                                      packets are forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Tx mirroring
*       ratio 1 -> all Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       ratio 10 -> 1 in 10 Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrTxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *ratioPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrTxStatMirroringToAnalyzerRatioGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ratioPtr));

    rc = internal_cpssDxChMirrTxStatMirroringToAnalyzerRatioGet(devNum, ratioPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ratioPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrTxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - enable\disable Egress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrTxStatMirrorToAnalyzerEnable
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCStatisticalTxSniffConfig;
    }
    /* getting register address */
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.trStatSniffAndStcReg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.
            statisticalAndCpuTriggeredEgressMirroringToAnalyzerPort.stcStatisticalTxSniffConfig;
    }


    regData = (enable == GT_TRUE) ? 1 : 0;

    /* writing <EgressStatMirrorEn> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 1, regData);

    return rc;
}

/**
* @internal cpssDxChMirrTxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - enable\disable Egress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxStatMirrorToAnalyzerEnable
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrTxStatMirrorToAnalyzerEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChMirrTxStatMirrorToAnalyzerEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrTxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get mode of Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)  Egress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrTxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCStatisticalTxSniffConfig;
    }
    /* getting register address */
    else if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.trStatSniffAndStcReg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.
            statisticalAndCpuTriggeredEgressMirroringToAnalyzerPort.stcStatisticalTxSniffConfig;
    }

    /* writing <EgressStatMirrorEn> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 4, 1, &regData);
    if(rc != GT_OK)
        return rc;

    *enablePtr = ((regData != 0) ? GT_TRUE : GT_FALSE);

    return rc;
}

/**
* @internal cpssDxChMirrTxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get mode of Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)  Egress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrTxStatMirrorToAnalyzerEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChMirrTxStatMirrorToAnalyzerEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorToAnalyzerForwardingModeSet function
* @endinternal
*
* @brief   Set Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     -  of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change Forwarding mode, applicaton should
*       disable Rx/Tx mirrorred ports.
*
*/
static GT_STATUS internal_cpssDxChMirrorToAnalyzerForwardingModeSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;         /* register address */
    GT_U32      regData;         /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_FALCON_E);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(mode)
        {
            case CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E:
                regData = 0;
                break;
            case CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E:
                regData = 1;
                break;
            case CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E:
                regData = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;
    }
    else
    {
        switch(mode)
        {
            case CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E:
                regData = 3;
                break;
            case CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E:
                regData = 0;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 2, regData);
    if(rc != GT_OK)
        return rc;

    if(mode != CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        regData = 1;
    }
    else
    {
        regData = 0;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* no need to set values in the EGF */

    }
    else
    if((PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E)&&
       (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum) != 0))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.egr.filterConfig.globalEnables;

        /* Enable / Disable End to End Sniffer. */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 9, 1, regData);
        if(rc != GT_OK)
            return rc;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.dist.global.distGlobalControlReg;

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 10, 1, regData);
    }
    return rc;
}

/**
* @internal cpssDxChMirrorToAnalyzerForwardingModeSet function
* @endinternal
*
* @brief   Set Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     -  of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change Forwarding mode, applicaton should
*       disable Rx/Tx mirrorred ports.
*
*/
GT_STATUS cpssDxChMirrorToAnalyzerForwardingModeSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorToAnalyzerForwardingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChMirrorToAnalyzerForwardingModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorToAnalyzerForwardingModeGet function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - pointer to mode of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorToAnalyzerForwardingModeGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    return prvCpssDxChMirrorToAnalyzerForwardingModeGet(devNum, modePtr);
}

/**
* @internal cpssDxChMirrorToAnalyzerForwardingModeGet function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - pointer to mode of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorToAnalyzerForwardingModeGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorToAnalyzerForwardingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChMirrorToAnalyzerForwardingModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorAnalyzerInterfaceSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerInterfaceSet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;         /* register address */
    GT_U32      regData = 0;         /* register data */
    GT_U32      hwDev, hwPort;
    GT_U32      dataLength;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);
    CPSS_NULL_PTR_CHECK_MAC(interfacePtr);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(interfacePtr->interface.type != CPSS_INTERFACE_PORT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        hwDev = interfacePtr->interface.devPort.hwDevNum;
        hwPort = interfacePtr->interface.devPort.portNum;

        if(hwPort > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum) ||
           hwDev  > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        regData = (hwDev | hwPort << 10);

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            dataLength = 25;
        }
        else
        {
            dataLength = 23;
        }

        /* getting register address */
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index];

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, dataLength, regData);

    }
    else
    {
        PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
            interfacePtr->interface.devPort.hwDevNum,
            interfacePtr->interface.devPort.portNum);
        hwDev =  PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(interfacePtr->interface.devPort.hwDevNum,
                                                     interfacePtr->interface.devPort.portNum);
        hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(interfacePtr->interface.devPort.hwDevNum,
                                                      interfacePtr->interface.devPort.portNum);

        if((hwDev > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)) ||
           (hwPort > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* set 1 bit for MonitorType (value = 0 incase of portType)
           5 bits for devNum and 13 bits for portNum */
        U32_SET_FIELD_IN_ENTRY_MAC(&regData,0,2,0);
        U32_SET_FIELD_IN_ENTRY_MAC(&regData,2,5,hwDev);
        U32_SET_FIELD_IN_ENTRY_MAC(&regData,7,13,hwPort);

        /* getting register address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.mirrorInterfaceParameterReg[index];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 20, regData);
    }

    return rc;
}

/**
* @internal cpssDxChMirrorAnalyzerInterfaceSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerInterfaceSet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerInterfaceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, interfacePtr));

    rc = internal_cpssDxChMirrorAnalyzerInterfaceSet(devNum, index, interfacePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, interfacePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorAnalyzerInterfaceGet function
* @endinternal
*
* @brief   This function gets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (APPLICABLE RANGES: 0..6)
*
* @param[out] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerInterfaceGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */
    GT_STATUS   rc;         /* return status */
    GT_U32      dataLength;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(interfacePtr);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* getting register address */
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index];

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            dataLength = 25;
        }
        else
        {
            dataLength = 23;
        }

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, dataLength, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        interfacePtr->interface.type = CPSS_INTERFACE_PORT_E;
        interfacePtr->interface.devPort.hwDevNum = regData & 0x3FF;
        interfacePtr->interface.devPort.portNum = regData >> 10;
    }
    else
    {
        /* getting register address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.mirrorInterfaceParameterReg[index];

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(0 != U32_GET_FIELD_MAC(regData,0,2))/*monitorType*/
        {
            /* this monitorType is not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        interfacePtr->interface.type = CPSS_INTERFACE_PORT_E;
        interfacePtr->interface.devPort.hwDevNum  = U32_GET_FIELD_MAC(regData,2,5);
        interfacePtr->interface.devPort.portNum = U32_GET_FIELD_MAC(regData,7,13);

    }

    return GT_OK;
}

/**
* @internal cpssDxChMirrorAnalyzerInterfaceGet function
* @endinternal
*
* @brief   This function gets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (APPLICABLE RANGES: 0..6)
*
* @param[out] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerInterfaceGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerInterfaceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, interfacePtr));

    rc = internal_cpssDxChMirrorAnalyzerInterfaceGet(devNum, index, interfacePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, interfacePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for ingress
*         mirroring from all engines except
*         port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode, Policy-Based,
*                                      VLAN-Based, FDB-Based, Router-Based.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig;
    }

    /* 0 is used for no mirroring */
    regData = (enable == GT_TRUE) ? (index + 1) : 0;

    return prvCpssHwPpSetRegField(devNum, regAddr, 2, 3, regData);
}

/**
* @internal cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for ingress
*         mirroring from all engines except
*         port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode, Policy-Based,
*                                      VLAN-Based, FDB-Based, Router-Based.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable, index));

    rc = internal_cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(devNum, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for ingress mirroring
*         from all engines except port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode, Policy-Based,
*                                      VLAN-Based, FDB-Based, Router-Based.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
{
    GT_U32      regAddr;       /* register address */
    GT_U32      regData;       /* register data */
    GT_STATUS   rc;            /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 3, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* No mirroring */
    if(regData == 0)
    {
        *enablePtr = GT_FALSE;
    }
    /* mirroring is enabled */
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = regData - 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for ingress mirroring
*         from all engines except port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode, Policy-Based,
*                                      VLAN-Based, FDB-Based, Router-Based.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr, indexPtr));

    rc = internal_cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(devNum, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig;
    }

    /* 0 is used for no mirroring */
    regData = (enable == GT_TRUE) ? (index + 1) : 0;

    return prvCpssHwPpSetRegField(devNum, regAddr, 5, 3, regData);
}

/**
* @internal cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable, index));

    rc = internal_cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
{
    GT_U32      regAddr;      /* register address */
    GT_U32      regData;      /* register data */
    GT_STATUS   rc;           /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    /* getting register address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.analyzerPortGlobalConfig;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 5, 3, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* No mirroring */
    if(regData == 0)
    {
        *enablePtr = GT_FALSE;
    }
    /* mirroring is enabled */
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = regData - 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr, indexPtr));

    rc = internal_cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(devNum, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorAnalyzerMirrorOnDropEnableSet function
* @endinternal
*
* @brief   Enable / Disable mirroring of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - GT_TRUE  - dropped packets are mirrored to analyzer interface.
*                                      GT_FALSE - dropped packets are not mirrored to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerMirrorOnDropEnableSet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN GT_BOOL   enable
)
{
    GT_U32      regAddr;         /* register address */
    GT_U32      regData;         /* register data */
    GT_U32      fieldOffset;     /* register field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regData = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* getting register address */
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index];

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            fieldOffset = 25;
        }
        else
        {
            fieldOffset = 23;
        }

        return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, regData);
    }
    else
    {
        /* getting register address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.mirrorInterfaceParameterReg[index];

        return prvCpssHwPpSetRegField(devNum, regAddr, 20, 1, regData);
    }

}

/**
* @internal cpssDxChMirrorAnalyzerMirrorOnDropEnableSet function
* @endinternal
*
* @brief   Enable / Disable mirroring of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - GT_TRUE  - dropped packets are mirrored to analyzer interface.
*                                      GT_FALSE - dropped packets are not mirrored to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerMirrorOnDropEnableSet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerMirrorOnDropEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, enable));

    rc = internal_cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(devNum, index, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorAnalyzerMirrorOnDropEnableGet function
* @endinternal
*
* @brief   Get mirroring status of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @param[out] enablePtr                - Pointer to mirroring status of dropped packets.
*                                      - GT_TRUE  - dropped packets are mirrored to
*                                      analyzer interface.
*                                      - GT_FALSE - dropped packets are not mirrored to
*                                      analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerMirrorOnDropEnableGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *enablePtr
)
{
    GT_U32      regAddr;      /* register address */
    GT_U32      regData;      /* register data */
    GT_STATUS   rc;           /* return code */
    GT_U32      fieldOffset;  /* register field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* getting register address */
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index];

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            fieldOffset = 25;
        }
        else
        {
            fieldOffset = 23;
        }

        rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &regData);
    }
    else
    {
        /* getting register address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bufferMng.eqBlkCfgRegs.mirrorInterfaceParameterReg[index];

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 20, 1, &regData);
    }

    *enablePtr = (regData == 1) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal cpssDxChMirrorAnalyzerMirrorOnDropEnableGet function
* @endinternal
*
* @brief   Get mirroring status of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @param[out] enablePtr                - Pointer to mirroring status of dropped packets.
*                                      - GT_TRUE  - dropped packets are mirrored to
*                                      analyzer interface.
*                                      - GT_FALSE - dropped packets are not mirrored to
*                                      analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerMirrorOnDropEnableGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerMirrorOnDropEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, enablePtr));

    rc = internal_cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(devNum, index, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - VLAN tag is removed from mirrored traffic.
*                                      GT_FALSE - VLAN tag isn't removed from mirrored traffic.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32  regAddr;            /* register address     */
    GT_U32  data;               /* reg subfield data    */
    GT_U32  portGroupId;        /* the port group Id - support multi-port-groups device */
    GT_U32  fieldOffset;        /* register field offset */
    GT_U32   localPort;          /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    data = (enable == GT_TRUE) ? 0 : 1;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                               portNum,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                               SIP5_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS_E, /* field name */
                                               PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        fieldOffset = OFFSET_TO_BIT_MAC(localPort);

        /* getting register address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
            mirrorToAnalyzerHeaderConfReg[OFFSET_TO_WORD_MAC(localPort)];

        /* Enable/Disable VLAN tag removal of mirrored traffic. */
        return  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                    fieldOffset, 1, data);
    }
}

/**
* @internal cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - VLAN tag is removed from mirrored traffic.
*                                      GT_FALSE - VLAN tag isn't removed from mirrored traffic.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet function
* @endinternal
*
* @brief   Get status of enabling/disabling VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - pointer to status of VLAN tag removal of mirrored traffic.
*                                      - GT_TRUE  - VLAN tag is removed from mirrored traffic.
*                                      GT_FALSE - VLAN tag isn't removed from mirrored traffic.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32      regAddr;      /* register address */
    GT_U32      regData;      /* register data */
    GT_U32      portGroupId;  /* the port group Id - support multi-port-groups device */
    GT_U32      fieldOffset;  /* register field offset */
    GT_STATUS   rc;           /* return code */
    GT_U32       localPort;    /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                               CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                               portNum,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                               SIP5_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS_E,/* field name */
                                               PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               &regData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
        fieldOffset = OFFSET_TO_BIT_MAC(localPort);

            /* getting register address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
            mirrorToAnalyzerHeaderConfReg[OFFSET_TO_WORD_MAC(localPort)];

        /* Get status of enabling/disabling VLAN tag removal of mirrored traffic. */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                    fieldOffset, 1, &regData);
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (regData == 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet function
* @endinternal
*
* @brief   Get status of enabling/disabling VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - pointer to status of VLAN tag removal of mirrored traffic.
*                                      - GT_TRUE  - VLAN tag is removed from mirrored traffic.
*                                      GT_FALSE - VLAN tag isn't removed from mirrored traffic.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorEnhancedMirroringPriorityModeSet function
* @endinternal
*
* @brief   Setting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Enhanced Mirroring Priority selected working mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorEnhancedMirroringPriorityModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT  mode
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        PRV_CPSS_DXCH_XCAT_A3_ONLY_DEV_CHECK_MAC(devNum);
    }

    switch(mode)
    {
        case CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_TDM_E: regData = 0x0;
                                                       break;
        case CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E:
                                                       regData = 0xE;
                                                       break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        bufferMng.eqBlkCfgRegs.preEgrEngineGlobal;

    /* write bits [19:16] in Pre-Egress Engine Global Configuration Register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 16, 4, regData);
}

/**
* @internal cpssDxChMirrorEnhancedMirroringPriorityModeSet function
* @endinternal
*
* @brief   Setting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Enhanced Mirroring Priority selected working mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorEnhancedMirroringPriorityModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorEnhancedMirroringPriorityModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChMirrorEnhancedMirroringPriorityModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorEnhancedMirroringPriorityModeGet function
* @endinternal
*
* @brief   Getting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Enhanced Mirroring Priority selected working mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorEnhancedMirroringPriorityModeGet
(
    IN GT_U8                                         devNum,
    OUT CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT  *modePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        PRV_CPSS_DXCH_XCAT_A3_ONLY_DEV_CHECK_MAC(devNum);
    }

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        bufferMng.eqBlkCfgRegs.preEgrEngineGlobal;

    /* read bits [19:16] in Pre-Egress Engine Global Configuration Register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 16, 4, &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(regData)
    {
        case 0x0: *modePtr = CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_TDM_E;
                  break;
        case 0xE: *modePtr = CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E;
                  break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChMirrorEnhancedMirroringPriorityModeGet function
* @endinternal
*
* @brief   Getting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Enhanced Mirroring Priority selected working mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorEnhancedMirroringPriorityModeGet
(
    IN GT_U8                                         devNum,
    OUT CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT  *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorEnhancedMirroringPriorityModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChMirrorEnhancedMirroringPriorityModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet function
* @endinternal
*
* @brief   Set the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] dropCode                 - the drop code. (values like 'cpu code')
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad dropCode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;
    GT_U32      regAddr;        /* register address */
    GT_U32  regVal;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* convert dropCode to dsaCpuCode */
    rc = prvCpssDxChNetIfCpuToDsaCode(dropCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    regVal = dsaCpuCode;

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 8, 8, regVal);
    return rc;
}

/**
* @internal cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet function
* @endinternal
*
* @brief   Set the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] dropCode                 - the drop code. (values like 'cpu code')
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad dropCode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCode));

    rc = internal_cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet(devNum, dropCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet function
* @endinternal
*
* @brief   Get the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] dropCodePtr              - (pointer to) the drop code. (values like 'cpu code')
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;
    GT_U32      regAddr;        /* register address */
    GT_U32  regVal;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(dropCodePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 8, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    dsaCpuCode = regVal;

    /* convert dsaCpuCode to dropCode */
    rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,dropCodePtr);

    return rc;
}

/**
* @internal cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet function
* @endinternal
*
* @brief   Get the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] dropCodePtr              - (pointer to) the drop code. (values like 'cpu code')
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCodePtr));

    rc = internal_cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet(devNum, dropCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChMirrorTxPortVlanEnableSet function
* @endinternal
*
* @brief   Enable or disable Tx vlan mirroring from specific physical port.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - enable/disable Tx vlan mirroring from this physical port
*                                      GT_TRUE  - Tx vlan mirroring enabled  for this physical port.
*                                      GT_FALSE - Tx vlan mirroring disabled for this physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad portNum.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxPortVlanEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;        /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    BOOL2BIT_MAC(enable));
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).
            distributor.evlanMirrEnable[OFFSET_TO_WORD_MAC(portNum)];

        rc = prvCpssHwPpSetRegField(devNum, regAddr, OFFSET_TO_BIT_MAC(portNum), 1, BOOL2BIT_MAC(enable));
    }

    return rc;
}

/**
* @internal cpssDxChMirrorTxPortVlanEnableSet function
* @endinternal
*
* @brief   Enable or disable Tx vlan mirroring from specific physical port.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - enable/disable Tx vlan mirroring from this physical port
*                                      GT_TRUE  - Tx vlan mirroring enabled  for this physical port.
*                                      GT_FALSE - Tx vlan mirroring disabled for this physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad portNum.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortVlanEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxPortVlanEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChMirrorTxPortVlanEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxPortVlanEnableGet function
* @endinternal
*
* @brief   Get is the Tx vlan mirroring from specific physical port enabled/disabled.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to)enable/disable Tx vlan mirroring from this physical port
*                                      GT_TRUE  - Tx vlan mirroring enabled  for this physical port.
*                                      GT_FALSE - Tx vlan mirroring disabled for this physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxPortVlanEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    &regValue);
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).
            distributor.evlanMirrEnable[OFFSET_TO_WORD_MAC(portNum)];

        rc = prvCpssHwPpGetRegField(devNum, regAddr, OFFSET_TO_BIT_MAC(portNum), 1, &regValue);
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return rc;
}

/**
* @internal cpssDxChMirrorTxPortVlanEnableGet function
* @endinternal
*
* @brief   Get is the Tx vlan mirroring from specific physical port enabled/disabled.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to)enable/disable Tx vlan mirroring from this physical port
*                                      GT_TRUE  - Tx vlan mirroring enabled  for this physical port.
*                                      GT_FALSE - Tx vlan mirroring disabled for this physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortVlanEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxPortVlanEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChMirrorTxPortVlanEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief  This function sets analyzer interface index, used for
*         egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] egressMirroringMode      - can be one of the following 2 values: tail_drop or congestion
* @param[in] enable                   - global enable/disable egress mirroring
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    IN GT_BOOL                          enable,
    IN GT_U32                           index
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */
    GT_U32      fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if((index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS) || (egressMirroringMode != CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E && egressMirroringMode != CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* getting register address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.mirroringGlobalConfig;

    /* 0 is used for no mirroring */
    regData = (enable == GT_TRUE) ? (index + 1) : 0;
    fieldOffset = (egressMirroringMode == CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E) ? 0 : 3;

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 3, regData);
}

/**
* @internal cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief  This function sets analyzer interface index, used for
*         egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] egressMirroringMode      - can be one of the following 2 values: tail_drop or congestion
* @param[in] enable                   - global enable/disable egress mirroring
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    IN GT_BOOL                          enable,
    IN GT_U32                           index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, egressMirroringMode, enable, index));

    rc = internal_cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet(devNum, egressMirroringMode, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, egressMirroringMode, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief  This function gets analyzer interface index, used for
*         egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] egressMirroringMode      - can be one of the following 2 values: tail_drop or congestion
* @param[out] enablePtr               - pointer to global enable/disable egress mirroring.
* @param[out] indexPtr                - pointer to analyzer interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - reading HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    OUT GT_BOOL                         *enablePtr,
    OUT GT_U32                          *indexPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */
    GT_U32      fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(egressMirroringMode != CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E && egressMirroringMode != CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    /* getting register address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.mirroringGlobalConfig;

    fieldOffset = (egressMirroringMode == CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E) ? 0 : 3;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 3, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* No mirroring */
    if(regData == 0)
    {
        *enablePtr = GT_FALSE;
    }
    /* mirroring is enabled */
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = regData - 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief  This function gets analyzer interface index, used for
*         egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] egressMirroringMode      - can be one of the following 2 values: tail_drop or congestion
* @param[out] enablePtr               - pointer to global enable/disable egress mirroring.
* @param[out] indexPtr                - pointer to analyzer interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - reading HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    OUT GT_BOOL                         *enablePtr,
    OUT GT_U32                          *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, egressMirroringMode, enablePtr, indexPtr));

    rc = internal_cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(devNum, egressMirroringMode, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, egressMirroringMode, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxPortModeSet function
* @endinternal
*
* @brief   Set Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number
* @param[in] mirrPort                   - port number.
* @param[in] isPhysicalPort             - defines type of mirrPort parameter to be either
*                                         physical port or ePort. GT_TRUE - mirrPort is physical port,
*                                         GT_FALSE - mirrPort is ePort.
* @param[in] egressMirroringMode      - one of 4 options of the enum mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, mirrPort or egressMirroringMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxPortModeSet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      mirrPort,
    IN  GT_BOOL                          isPhysicalPort,
    IN  CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
)
{
    GT_U32  hwValue;  /* HW value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_HW_VAL_MAC(hwValue, egressMirroringMode);

    if (isPhysicalPort == GT_FALSE)
    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, mirrPort);

        return prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
            mirrPort,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            hwValue);
    }
    else
    {
        PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, mirrPort);

        return prvCpssDxChPortGroupWriteTableEntryField(devNum,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
            mirrPort,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_EGRESS_MIRRORING_CODE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            hwValue);
    }
}

/**
* @internal cpssDxChMirrorTxPortModeSet function
* @endinternal
*
* @brief  Set Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - the device number
* @param[in] mirrPort                - port number.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
* @param[in] egressMirroringMode     - one of 4 options of the mode enum.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortModeSet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      mirrPort,
    IN  GT_BOOL                          isPhysicalPort,
    IN  CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxPortModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mirrPort, isPhysicalPort, egressMirroringMode));

    rc = internal_cpssDxChMirrorTxPortModeSet(devNum, mirrPort, isPhysicalPort, egressMirroringMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mirrPort, isPhysicalPort, egressMirroringMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorTxPortModeGet function
* @endinternal
*
* @brief   Get Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - device number
* @param[in] mirrPort                - port number.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
* @param[out] egressMirroringModePtr – (pointer to) Egress mirroring mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, mirrPort or NULL pointer egressMirroringModePtr
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorTxPortModeGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      mirrPort,
    IN  GT_BOOL                          isPhysicalPort,
    OUT CPSS_DXCH_MIRROR_EGRESS_MODE_ENT *egressMirroringModePtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;  /* HW value */
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egress_mirroring_mode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(egressMirroringModePtr);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (isPhysicalPort == GT_FALSE)
    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, mirrPort);

        rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
            mirrPort,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);
    }
    else
    {
        PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, mirrPort);

        rc = prvCpssDxChPortGroupReadTableEntryField(
            devNum,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
            mirrPort,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_EGRESS_MIRRORING_CODE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);
    }

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_SW_VAL_MAC(hwValue, egress_mirroring_mode);
    *egressMirroringModePtr = egress_mirroring_mode;

    return rc;
}

/**
* @internal cpssDxChMirrorTxPortModeGet function
* @endinternal
*
* @brief  Get Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - the device number
* @param[in] mirrPort                - port number.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
* @param[out] egressMirroringModePtr – (pointer to) Egress mirroring mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortModeGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      mirrPort,
    IN  GT_BOOL                          isPhysicalPort,
    OUT CPSS_DXCH_MIRROR_EGRESS_MODE_ENT *egressMirroringModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorTxPortModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mirrPort, isPhysicalPort, egressMirroringModePtr));

    rc = internal_cpssDxChMirrorTxPortModeGet(devNum, mirrPort, isPhysicalPort, egressMirroringModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mirrPort, isPhysicalPort, egressMirroringModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet function
* @endinternal
*
* @brief  This function sets Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - enable/disable statistical mirroring.
* @param[in] ratio                    - Indicates the  ratio of  mirrored traffic to analyzer port.
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    IN GT_BOOL    enable,
    IN GT_U32     ratio
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortEnableConfig[index];

    /* enable/disable IngressStatMirroringToAnalyzerPortEn
       Ingress Statistic Mirroring to Analyzer Port Configuration Register */
    regData = (enable == 1) ? 1 : 0;

    /* writing <IngressStatMirroringToAnalyzerPortEn> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, regData);
    if(rc != GT_OK)
        return rc;

    /* getting register address IngressStatMirroringToAnalyzerPortRatioConfiguration */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortRatioConfig[index];

    /* writing <IngressStatMirroringToAnalyzerPortRatio> field */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, ratio);

    return rc;
}

/**
* @internal cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet function
* @endinternal
*
* @brief  This function sets Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - enable/disable statistical mirroring.
* @param[in] ratio                    - Indicates the  ratio of  mirrored traffic to analyzer port.
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    IN GT_BOOL    enable,
    IN GT_U32     ratio
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, enable, ratio));

    rc = internal_cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(devNum, index, enable, ratio);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, enable, ratio));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet function
* @endinternal
*
* @brief  This function gets Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] enablePtr               - (pointer to) enable/disable statistical mirroring.
* @param[out] ratioPtr                - (pointer to) Indicates the  ratio of  mirrored traffic to analyzer port
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - reading HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *ratioPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data */
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(ratioPtr);

    /* getting register address IngressStatMirroringToAnalyzerPortRatioConfiguration */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortRatioConfig[index];

    rc = prvCpssHwPpReadRegister(devNum, regAddr, ratioPtr);
    if(rc != GT_OK)
        return rc;

    /* getting register address */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortEnableConfig[index];

    /* enable/disable IngressStatMirroringToAnalyzerPortEn
       Ingress Statistic Mirroring to Analyzer Port Configuration Register */

    /* reading <IngressStatMirroringToAnalyzerPortEn> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &regData);

    if( rc != GT_OK )
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regData);

    return GT_OK;
}

/**
* @internal cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet function
* @endinternal
*
* @brief  This function gets Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] enablePtr               - (pointer to) enable/disable statistical mirroring.
* @param[out] ratioPtr                - (pointer to) Indicates the  ratio of  mirrored traffic to analyzer port
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - reading HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *ratioPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, enablePtr, ratioPtr));

    rc = internal_cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(devNum, index, enablePtr, ratioPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, enablePtr, ratioPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorToAnalyzerTruncateSet function
* @endinternal
*
* @brief  This function sets if to truncate TO_ANALYZER packets per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] truncate                 - Indicates if truncate TO_ANALYZER packates to 128B
*                                       GT_TRUE – truncate, GT_FALSE – do not truncate.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorToAnalyzerTruncateSet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    IN GT_BOOL    truncate
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index];

    regData = (truncate == 1) ? 1 : 0;

    /* writing <Monitor_Truncate> field */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 26, 1, regData);

    return rc;
}

/**
* @internal cpssDxChMirrorToAnalyzerTruncateSet function
* @endinternal
*
* @brief  This function sets if to truncate TO_ANALYZER packets per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] truncate                 - Indicates if truncate TO_ANALYZER packates to 128B
*                                       GT_TRUE – truncate, GT_FALSE – do not truncate.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorToAnalyzerTruncateSet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    IN GT_BOOL    truncate
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorToAnalyzerTruncateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, truncate));

    rc = internal_cpssDxChMirrorToAnalyzerTruncateSet(devNum, index, truncate);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, truncate));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorToAnalyzerTruncateGet function
* @endinternal
*
* @brief  This function gets truncate field per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] trucnatePtr             - (pointer to) Indicates the  truncate field in TO_ANALYZER packets
*                                       GT_TRUE – truncated, GT_FALSE – not truncated.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorToAnalyzerTruncateGet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *truncatePtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data */
    GT_STATUS   rc;             /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(truncatePtr);

    /* getting register address */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
        mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index];

    /* reading <Monitor_Truncate> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 26, 1, &regData);

    if( rc != GT_OK )
    {
        return rc;
    }

    *truncatePtr = BIT2BOOL_MAC(regData);

    return GT_OK;
}

/**
* @internal cpssDxChMirrorToAnalyzerTruncateGet function
* @endinternal
*
* @brief  This function gets truncate field per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] trucnatePtr             - (pointer to) Indicates the  truncate field in TO_ANALYZER packets
*                                       GT_TRUE – truncated, GT_FALSE – not truncated.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorToAnalyzerTruncateGet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *truncatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorToAnalyzerTruncateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, truncatePtr));

    rc = internal_cpssDxChMirrorToAnalyzerTruncateGet(devNum, index, truncatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, truncatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorOnCongestionModeSet function
* @endinternal
*
* @brief   Defines which congestion limits are used for
*          mirror-on-congestion(QCN/ECN).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - the device number
* @param[in] congestionMode           - congestion mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or congestionMode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorOnCongestionModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT  congestionMode
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;        /* register address */
    GT_U32      value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    switch(congestionMode)
    {
        case CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E:
            value     = 0;
        break;

        case CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_ECN_E:
            value     = 1;
        break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 21, 1, value);

    return rc;
}

/**
* @internal cpssDxChMirrorOnCongestionModeSet function
* @endinternal
*
* @brief   Defines which congestion limits are used for
*          mirror on congestion (QCN/ECN).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - the device number
* @param[in] congestionMode           - congestion mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or congestionMode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorOnCongestionModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT  congestionMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorOnCongestionModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, congestionMode));

    rc = internal_cpssDxChMirrorOnCongestionModeSet(devNum, congestionMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, congestionMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMirrorOnCongestionModeGet function
* @endinternal
*
* @brief   Get the congestion limits used for
*          mirror on congestion (QCN/ECN).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - the device number
* @param[in] congestionModePtr        - (pointer to)congestion mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMirrorOnCongestionModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT  *congestionModePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(congestionModePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PREQ.globalConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 21, 1, &regValue);

    *congestionModePtr = (regValue == 0) ? CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E :
                                      CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_ECN_E;

    return rc;
}

/**
* @internal cpssDxChMirrorOnCongestionModeGet function
* @endinternal
*
* @brief   Get the congestion limits used for
*          mirror on congestion (QCN/ECN).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] congestionModePtr        - (pointer to)congestion mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorOnCongestionModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT  *congestionModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMirrorOnCongestionModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, congestionModePtr));

    rc = internal_cpssDxChMirrorOnCongestionModeGet(devNum, congestionModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, congestionModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

