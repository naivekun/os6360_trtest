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
* @file cpssDxChLatencyMonitoring.c
*
* @brief CPSS DxCh Latency monitoring API implementation.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/latencyMonitoring/cpssDxChLatencyMonitoring.h>
#include <cpss/dxCh/dxChxGen/latencyMonitoring/private/prvCpssDxChLatencyMonitoringLog.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>

/**
* @internal internal_cpssDxChLatencyMonitoringPortCfgSet function
* @endinternal
*
* @brief  Set index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress, egress or both.
* @param[in] index                 - index (APPLICABLE RANGES: 0..255)
* @param[in] profile               - latency profile (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index or profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortCfgSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DIRECTION_ENT                   portType,
    IN GT_U32                               index,
    IN GT_U32                               profile
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if(index > 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(profile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    if((CPSS_DIRECTION_INGRESS_E == portType) || (CPSS_DIRECTION_BOTH_E == portType))
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0, 17, (index | (profile << 8))
        );
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    if((CPSS_DIRECTION_EGRESS_E == portType) || (CPSS_DIRECTION_BOTH_E == portType))
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0, 17, (index | (profile << 8))
        );
    }
    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringPortCfgSet function
* @endinternal
*
* @brief  Set index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress, egress or both.
* @param[in] index                 - index (APPLICABLE RANGES: 0..255)
* @param[in] profile               - latency profile (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index or profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringPortCfgSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DIRECTION_ENT                   portType,
    IN GT_U32                               index,
    IN GT_U32                               profile
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortCfgSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portType, index, profile));

    rc = internal_cpssDxChLatencyMonitoringPortCfgSet(devNum, portNum, portType, index, profile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portType, index, profile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortCfgGet function
* @endinternal
*
* @brief  Get index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress or egress
*
* @param[out] indexPtr             - (pointer to) index
* @param[out] profilePtr           - (pointer to) latency profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortCfgGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DIRECTION_ENT                   portType,
    OUT GT_U32                               *indexPtr,
    OUT GT_U32                               *profilePtr
)
{
    GT_STATUS rc;
    GT_U32    value;
    CPSS_DXCH_TABLE_ENT table;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    table = (CPSS_DIRECTION_INGRESS_E == portType)?
                CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E:
            (CPSS_DIRECTION_EGRESS_E == portType)?
                CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E:
                0;
    if(0 == table)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc =  prvCpssDxChReadTableEntryField(devNum, table, portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        0, 17, &value
    );

    if(rc != GT_OK)
    {
        return rc;
    }

    *indexPtr   = U32_GET_FIELD_MAC(value, 0, 8);
    *profilePtr = U32_GET_FIELD_MAC(value, 8, 9);

    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringPortCfgGet function
* @endinternal
*
* @brief  Get index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress or egress
*
* @param[out] indexPtr             - (pointer to) index
* @param[out] profilePtr           - (pointer to) latency profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringPortCfgGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DIRECTION_ENT                   portType,
    OUT GT_U32                               *indexPtr,
    OUT GT_U32                               *profilePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortCfgGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portType, indexPtr, profilePtr));

    rc = internal_cpssDxChLatencyMonitoringPortCfgGet(devNum, portNum, portType, indexPtr, profilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portType, indexPtr, profilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringEnableSet function
* @endinternal
*
* @brief  Set latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
* @param[in] enabled               - latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    IN GT_BOOL              enabled
)
{
    GT_U32    index;
    GT_U32    offset;
    GT_U32    srcIdx;
    GT_U32    dstIdx;
    GT_U32    profile;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, srcPortNum,
        CPSS_DIRECTION_INGRESS_E, &srcIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, trgPortNum,
        CPSS_DIRECTION_EGRESS_E, &dstIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    index = srcIdx * 256 + dstIdx;
    offset = index & 0x1F;
    index >>= 5;

    return prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E,
            index,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            offset, 1, BOOL2BIT_MAC(enabled));
}

/**
* @internal cpssDxChLatencyMonitoringEnableSet function
* @endinternal
*
* @brief  Set latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
* @param[in] enabled               - latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    IN GT_BOOL              enabled
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcPortNum, trgPortNum, enabled));

    rc = internal_cpssDxChLatencyMonitoringEnableSet(devNum, srcPortNum, trgPortNum, enabled);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcPortNum, trgPortNum, enabled));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringEnableGet function
* @endinternal
*
* @brief  Get latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
*
* @param[out] enabledPtr           - (pointer to) latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    OUT GT_BOOL             *enabledPtr
)
{
    GT_U32    index;
    GT_U32    offset;
    GT_U32    value;
    GT_U32    srcIdx;
    GT_U32    dstIdx;
    GT_U32    profile;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(enabledPtr);

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, srcPortNum,
        CPSS_DIRECTION_INGRESS_E, &srcIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, trgPortNum,
        CPSS_DIRECTION_EGRESS_E, &dstIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    index = srcIdx * 256 + dstIdx;
    offset = index & 0x1F;
    index >>= 5;

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E,
        index,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        offset, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    *enabledPtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringEnableGet function
* @endinternal
*
* @brief  Get latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
*
* @param[out] enabledPtr           - (pointer to) latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    OUT GT_BOOL             *enabledPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringEnableGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcPortNum, trgPortNum, enabledPtr));

    rc = internal_cpssDxChLatencyMonitoringEnableGet(devNum, srcPortNum, trgPortNum, enabledPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcPortNum, trgPortNum, enabledPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingProfileSet function
* @endinternal
*
* @brief  Set latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingProfileSet
(
    IN GT_U8        devNum,
    IN GT_U32       latencyProfile,
    IN GT_U32       samplingProfile
)
{
    GT_U32 regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(samplingProfile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringProfile2SamplingProfile[latencyProfile >> 3];

    return prvCpssHwPpSetRegField(devNum, regAddr, (latencyProfile & 7) * 3,
        3, samplingProfile);
}

/**
* @internal cpssDxChLatencyMonitoringSamplingProfileSet function
* @endinternal
*
* @brief  Set latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingProfileSet
(
    IN GT_U8        devNum,
    IN GT_U32       latencyProfile,
    IN GT_U32       samplingProfile
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingProfileSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, latencyProfile, samplingProfile));

    rc = internal_cpssDxChLatencyMonitoringSamplingProfileSet(devNum, latencyProfile, samplingProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, latencyProfile, samplingProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingProfileGet function
* @endinternal
*
* @brief  Get latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] samplingProfilePtr   - (pointer to) sampling profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingProfileGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       latencyProfile,
    OUT GT_U32       *samplingProfilePtr
)
{
    GT_U32 regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(samplingProfilePtr);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringProfile2SamplingProfile[latencyProfile >> 3];

    return prvCpssHwPpGetRegField(devNum, regAddr, (latencyProfile & 7) * 3,
        3, samplingProfilePtr);
}

/**
* @internal cpssDxChLatencyMonitoringSamplingProfileGet function
* @endinternal
*
* @brief  Get latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] samplingProfilePtr   - (pointer to) sampling profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingProfileGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       latencyProfile,
    OUT GT_U32       *samplingProfilePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingProfileGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, latencyProfile, samplingProfilePtr));

    rc = internal_cpssDxChLatencyMonitoringSamplingProfileGet(devNum, latencyProfile, samplingProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, latencyProfile, samplingProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingConfigurationSet function
* @endinternal
*
* @brief  Set latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
* @param[in] samplingMode          - sampling mode.
* @param[in] samplingThreshold     - sampling threshold. Value 0 - every packet sampled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingConfigurationSet
(
    IN GT_U8                                          devNum,
    IN GT_U32                                         samplingProfile,
    IN CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode,
    IN GT_U32                                         samplingThreshold
)
{
    GT_U32 regAddr;
    GT_U32 rc;
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if(samplingProfile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(samplingMode)
    {
        case CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E:
           value = 0;
           break;
        case CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E:
           value = 1;
           break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringSamplingConfig[samplingProfile];
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, samplingThreshold);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).latencyMonitoringControl;
    return prvCpssHwPpSetRegField(devNum, regAddr, samplingProfile, 1, value);
}

/**
* @internal cpssDxChLatencyMonitoringSamplingConfigurationSet function
* @endinternal
*
* @brief  Set latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
* @param[in] samplingMode          - sampling mode.
* @param[in] samplingThreshold     - sampling threshold. Value 0 - every packet sampled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingConfigurationSet
(
    IN GT_U8                                          devNum,
    IN GT_U32                                         samplingProfile,
    IN CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode,
    IN GT_U32                                         samplingThreshold
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingConfigurationSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, samplingProfile, samplingMode, samplingThreshold));

    rc = internal_cpssDxChLatencyMonitoringSamplingConfigurationSet(devNum, samplingProfile, samplingMode, samplingThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, samplingProfile, samplingMode, samplingThreshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingConfigurationGet function
* @endinternal
*
* @brief  Get latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @param[out] samplingModePtr      - (pointer to)sampling mode.
* @param[out] samplingThresholdPtr - (pointer to)sampling threshold
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingConfigurationGet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         samplingProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT *samplingModePtr,
    OUT GT_U32                                         *samplingThresholdPtr
)
{
    GT_U32 regAddr;
    GT_U32 rc;
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(samplingThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(samplingModePtr);

    if(samplingProfile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringSamplingConfig[samplingProfile];

    rc = prvCpssHwPpReadRegister(devNum, regAddr, samplingThresholdPtr);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringControl;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, samplingProfile, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
           *samplingModePtr = CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E;
           break;
        case 1:
           *samplingModePtr = CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E;
           break;
        default:
            /* NOT REACHED */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringSamplingConfigurationGet function
* @endinternal
*
* @brief  Get latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @param[out] samplingModePtr      - (pointer to)sampling mode.
* @param[out] samplingThresholdPtr - (pointer to)sampling threshold
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingConfigurationGet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         samplingProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT *samplingModePtr,
    OUT GT_U32                                         *samplingThresholdPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingConfigurationGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, samplingProfile, samplingModePtr, samplingThresholdPtr));

    rc = internal_cpssDxChLatencyMonitoringSamplingConfigurationGet(devNum, samplingProfile, samplingModePtr, samplingThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, samplingProfile, samplingModePtr, samplingThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringStatGet function
* @endinternal
*
* @brief  Get latency monitoring statistics.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[out] statisticsPtr        - (pointer to) latency monitoring statistics
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note In case if multiple LMUs selected, packet counts are summed, average
* latency calculated as average of average latency values for all selected LMUs.
* Minimal latency is the lowest value of minimal latency values for all
* selected LMUs. Maximal latency is the highest value of maximal latency values
* for all selected LMUs.
*
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringStatGet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_STAT_STC  *statisticsPtr
)
{
    GT_U32 regAddr;
    GT_U32 ii;
    GT_U32 lmuNum;
    GT_U32 ravenNum;
    GT_U32 tileNum;
    GT_U32 value[8];
    GT_STATUS rc;
    GT_U64 totalAvgLatency;
    GT_U64 tmp;
    GT_U64 tmp2;
    GT_U32 number = 0;
    GT_STATUS st;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(statisticsPtr);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == lmuBmp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    statisticsPtr->packetsOutOfRange.l[0] = 0;
    statisticsPtr->packetsOutOfRange.l[1] = 0;
    statisticsPtr->packetsInRange.l[0] = 0;
    statisticsPtr->packetsInRange.l[1] = 0;
    totalAvgLatency.l[0] = 0;
    totalAvgLatency.l[1] = 0;
    statisticsPtr->maxLatency = 0;
    statisticsPtr->minLatency = 0x3FFFFFFF;

    for(ii = 0; ii < 32; ii++)
    {
        if((lmuBmp & (1 << ii)) == 0)
        {
            continue;
        }

        lmuNum   =  U32_GET_FIELD_MAC(ii, 0, 1);
        ravenNum =  U32_GET_FIELD_MAC(ii, 1, 2);
        tileNum  =  U32_GET_FIELD_MAC(ii, 3, 2);

        if(lmuBmp != CPSS_LMU_UNAWARE_MODE_CNS)
        {
            TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
        }
        else
        {
            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                continue;
            }
        }

        rc = prvCpssDxChReadTableEntry(
            devNum,
            CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + U32_GET_FIELD_MAC(ii, 0, 5),
            latencyProfile, &value[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        tmp.l[1] = 0;
        regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
            profileStatisticsReadData[0];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }
        statisticsPtr->packetsOutOfRange = prvCpssMathAdd64(
            statisticsPtr->packetsOutOfRange, tmp);


        regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
            profileStatisticsReadData[1];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
            profileStatisticsReadData[2];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &tmp.l[1]);
        if(GT_OK != rc)
        {
            return rc;
        }
        statisticsPtr->packetsInRange = prvCpssMathAdd64(
            statisticsPtr->packetsInRange, tmp);

        tmp.l[1] = 0;
        regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
            profileStatisticsReadData[3];
        rc = prvCpssHwPpReadRegister(devNum, regAddr,&tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        totalAvgLatency = prvCpssMathAdd64(totalAvgLatency, tmp);

        regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
            profileStatisticsReadData[4];
        rc = prvCpssHwPpReadRegister(devNum, regAddr,
            &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        if(tmp.l[0] > statisticsPtr->maxLatency)
        {
            statisticsPtr->maxLatency = tmp.l[0];
        }

        regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
            profileStatisticsReadData[5];
        rc = prvCpssHwPpReadRegister(devNum, regAddr,
            &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        if(tmp.l[0] < statisticsPtr->minLatency)
        {
            statisticsPtr->minLatency = tmp.l[0];
        }

        number++;
    }

    tmp.l[0] = number;
    tmp.l[1] = 0;
    st = prvCpssMathDiv64(totalAvgLatency, tmp, &tmp2, NULL);
    if(GT_OK != st)
    {
        return st;
    }
    statisticsPtr->avgLatency = tmp2.l[0];

    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringStatGet function
* @endinternal
*
* @brief  Get latency monitoring statistics.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[out] statisticsPtr        - (pointer to) latency monitoring statistics
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note In case if multiple LMUs selected, packet counts are summed, average
* latency calculated as average of average latency values for all selected LMUs.
* Minimal latency is the lowest value of minimal latency values for all
* selected LMUs. Maximal latency is the highest value of maximal latency values
* for all selected LMUs.
*
*/
GT_STATUS cpssDxChLatencyMonitoringStatGet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_STAT_STC  *statisticsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringStatGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lmuBmp, latencyProfile, statisticsPtr));

    rc = internal_cpssDxChLatencyMonitoringStatGet(devNum, lmuBmp, latencyProfile, statisticsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lmuBmp, latencyProfile, statisticsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringCfgSet function
* @endinternal
*
* @brief  Set latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] lmuConfigPtr          - (pointer to)latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringCfgSet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    IN  CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    value[4];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == lmuBmp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(lmuConfigPtr->rangeMax > 0x3FFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(lmuConfigPtr->rangeMin > 0x3FFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(lmuConfigPtr->notificationThresh > 0x3FFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_IN_ENTRY_MAC(&value[0],  0, 30, lmuConfigPtr->notificationThresh);
    U32_SET_FIELD_IN_ENTRY_MAC(&value[0], 30, 30, lmuConfigPtr->rangeMax);
    U32_SET_FIELD_IN_ENTRY_MAC(&value[0], 60, 30, lmuConfigPtr->rangeMin);

    for(ii = 0; ii < 32; ii++)
    {
        if(0 == (lmuBmp & (1 << ii)))
        {
            continue;
        }

        if(lmuBmp != CPSS_LMU_UNAWARE_MODE_CNS)
        {
            TXQ_SIP_6_CHECK_TILE_NUM_MAC(U32_GET_FIELD_MAC(ii, 3, 2));
        }
        else
        {
            if(ii >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * FALCON_RAVENS_PER_TILE * 2)
            {
                continue;
            }
        }

        rc = prvCpssDxChWriteTableEntry(
            devNum, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + ii,
            latencyProfile, &value[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringCfgSet function
* @endinternal
*
* @brief  Set latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] lmuConfigPtr          - (pointer to)latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringCfgSet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    IN  CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringCfgSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lmuBmp, latencyProfile, lmuConfigPtr));

    rc = internal_cpssDxChLatencyMonitoringCfgSet(devNum, lmuBmp, latencyProfile, lmuConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lmuBmp, latencyProfile, lmuConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringCfgGet function
* @endinternal
*
* @brief  Get latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. Only one LMU
*                                    can be selected.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] lmuConfigPtr         - (pointer to) latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringCfgGet
(
    IN  GT_U8                                 devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP        lmuBmp,
    IN  GT_U32                                latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_U32 ii;
    GT_U32 value[4];
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(lmuConfigPtr);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == lmuBmp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(ii = 0; (lmuBmp & 1) == 0; ii++) lmuBmp >>= 1;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(U32_GET_FIELD_MAC(ii, 3, 2));

    rc = prvCpssDxChReadTableEntry(
        devNum, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + ii,
        latencyProfile, &value[0]);
    if(GT_OK != rc)
    {
        return rc;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(&value[0],  0, 30, lmuConfigPtr->notificationThresh);
    U32_GET_FIELD_IN_ENTRY_MAC(&value[0], 30, 30, lmuConfigPtr->rangeMax);
    U32_GET_FIELD_IN_ENTRY_MAC(&value[0], 60, 30, lmuConfigPtr->rangeMin);

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringCfgGet function
* @endinternal
*
* @brief  Get latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. Only one LMU
*                                    can be selected.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] lmuConfigPtr         - (pointer to) latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringCfgGet
(
    IN  GT_U8                                 devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP        lmuBmp,
    IN  GT_U32                                latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringCfgGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lmuBmp, latencyProfile, lmuConfigPtr));

    rc = internal_cpssDxChLatencyMonitoringCfgGet(devNum, lmuBmp, latencyProfile, lmuConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lmuBmp, latencyProfile, lmuConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortEnableSet function
* @endinternal
*
* @brief  Enable/Disable egress port for latency monitoring.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enable                - latency monitoring enabled state
*                                    GT_TRUE - enable
*                                    GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortEnableSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  GT_BOOL                                  enable
)
{
    GT_STATUS                       rc;
    CPSS_DXCH_SHADOW_PORT_MAP_STC   detailedPortMap;
    GT_U32                          localDp;
    GT_U32                          lmuNum;
    GT_U32                          channel;
    GT_U32                          regAddr;
    GT_U32                          ravenNum;
    GT_U32                          tileNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    if (portNum > PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapGet(devNum, portNum, &detailedPortMap);
    if (GT_OK != rc )
    {
        return rc;
    }
    rc = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(
        devNum, detailedPortMap.txDmaNum, &tileNum, &localDp, &channel);
    if (GT_OK != rc )
    {
        return rc;
    }
    ravenNum = localDp / 2;
    lmuNum = localDp & 1;

    regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
        channelEnable;

    return prvCpssHwPpSetRegField(devNum, regAddr, channel, 1,
        BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChLatencyMonitoringPortEnableSet function
* @endinternal
*
* @brief  Enable/Disable egress port for latency monitoring.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enable                - latency monitoring enabled state
*                                    GT_TRUE - enable
*                                    GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChLatencyMonitoringPortEnableSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  GT_BOOL                                  enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChLatencyMonitoringPortEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortEnableGet function
* @endinternal
*
* @brief  Get enabled state of latency monitoring for port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] enablePtr            - (pointer to)latency monitoring enabled state
*                                    GT_TRUE  - enabled
*                                    GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortEnableGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    OUT GT_BOOL                                  *enablePtr
)
{
    GT_U32                          lmuNum;
    GT_U32                          channel;
    GT_U32                          value;
    GT_U32                          regAddr;
    GT_U32                          ravenNum;
    GT_U32                          tileNum;
    CPSS_DXCH_SHADOW_PORT_MAP_STC   detailedPortMap;
    GT_U32                          localDp;
    GT_STATUS                       rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    if (portNum > PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapGet(devNum, portNum, &detailedPortMap);
    if (GT_OK != rc )
    {
        return rc;
    }
    rc = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(
        devNum, detailedPortMap.txDmaNum, &tileNum, &localDp, &channel);
    if (GT_OK != rc )
    {
        return rc;
    }
    ravenNum = localDp / 2;
    lmuNum = localDp & 1;
    regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, tileNum, ravenNum, lmuNum).
        channelEnable;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, channel, 1, &value);
    *enablePtr = BIT2BOOL_MAC(value);
    return rc;
}


/**
* @internal cpssDxChLatencyMonitoringPortEnableGet function
* @endinternal
*
* @brief  Get enabled state of latency monitoring for port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] enablePtr            - (pointer to)latency monitoring enabled state
*                                    GT_TRUE  - enabled
*                                    GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChLatencyMonitoringPortEnableGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortEnableGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChLatencyMonitoringPortEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
