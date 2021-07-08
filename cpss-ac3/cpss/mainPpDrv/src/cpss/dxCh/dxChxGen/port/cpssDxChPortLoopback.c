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
* @file cpssDxChPortLoopback.c
*
* @brief Forwarding to loopback/service port APIs.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortLoopback.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>

#define PRV_CPSS_DXCH_ANALYZER_MAX_INDEX_CNS 6

/* check a specified VIDX value is in applicable range */
#define PRV_CPSS_DXCH_VIDX_NUM_CHECK_MAC(_devNum,vidx)                \
        if((vidx) >= PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.vidxNum)     \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vidx limited to max[%d] got[%d]",\
                PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.vidxNum-1,vidx);    \


/**
* @internal internal_cpssDxChPortLoopbackEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for specified combination of
*         source/target loopback profiles and traffic class.
*         NOTE: bobcat2 A0 device enables/disables the feature for all
*         traffic classes regardless 'tc' input parameter value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcProfile               - source loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] trgProfile               - target loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] tc                       - traffic class.
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE  -  forwarding to loopback
*                                      GT_FALSE - disable forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is general trigger.
*       You need to enable forwarding to loopback per desirable packet types
*       additionally. See
*       cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeToCpuSet
*       cpssDxChPortLoopbackPktTypeToAnalyzerSet
*
*/
static GT_STATUS internal_cpssDxChPortLoopbackEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  srcProfile,
    IN GT_U32  trgProfile,
    IN GT_U32  tc,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if (srcProfile >= CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS ||
        trgProfile >= CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
    {
        rc = prvCpssHwPpSetRegField(
            devNum,
            PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackEnableConfig[0],
            srcProfile * 4 + trgProfile,
            1,
            BOOL2BIT_MAC(enable));
    }
    else
    {
        rc = prvCpssHwPpSetRegField(
            devNum,
            PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackEnableConfig[srcProfile],
            trgProfile * 8 + tc,
            1,
            BOOL2BIT_MAC(enable));
    }
    return rc;
}

/**
* @internal cpssDxChPortLoopbackEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for specified combination of
*         source/target loopback profiles and traffic class.
*         NOTE: bobcat2 A0 device enables/disables the feature for all
*         traffic classes regardless 'tc' input parameter value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcProfile               - source loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] trgProfile               - target loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] tc                       - traffic class.
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE  -  forwarding to loopback
*                                      GT_FALSE - disable forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is general trigger.
*       You need to enable forwarding to loopback per desirable packet types
*       additionally. See
*       cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeToCpuSet
*       cpssDxChPortLoopbackPktTypeToAnalyzerSet
*
*/
GT_STATUS cpssDxChPortLoopbackEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  srcProfile,
    IN GT_U32  trgProfile,
    IN GT_U32  tc,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortLoopbackEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcProfile, trgProfile, tc, enable));

    rc = internal_cpssDxChPortLoopbackEnableSet(devNum, srcProfile, trgProfile, tc, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcProfile, trgProfile, tc, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackEnableGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status for specified combination of
*         source/target loopback profiles and traffic class.
*         bobcat2 a0 is not able enable forwarding-to-loopback per particular
*         traffic class so it ignores input parameter 'traffic class'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcProfile               - source loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] trgProfile               - target loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] tc                       - traffic class
*                                      (APPLICABLE RANGES: 0..7)
*
* @param[out] enablePtr               - GT_TRUE  - forwarding to loopback is enabled
*                                       GT_FALSE - forwarding to loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  srcProfile,
    IN  GT_U32  trgProfile,
    IN  GT_U32  tc,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32    value;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if (srcProfile >= CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS ||
        trgProfile >= CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
    {
        rc = prvCpssHwPpGetRegField(
            devNum,
            PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackEnableConfig[0],
            srcProfile * 4 + trgProfile,
            1,
            &value);

    }
    else
    {
        rc = prvCpssHwPpGetRegField(
            devNum,
            PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackEnableConfig[srcProfile],
            trgProfile * 8 + tc,
            1,
            &value);
    }
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChPortLoopbackEnableGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status for specified combination of
*         source/target loopback profiles and traffic class.
*         bobcat2 a0 is not able enable forwarding-to-loopback per particular
*         traffic class so it ignores input parameter 'traffic class'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcProfile               - source loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] trgProfile               - target loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] tc                       - traffic class
*                                      (APPLICABLE RANGES: 0..7)
*
* @param[out] enablePtr               - GT_TRUE  - forwarding to loopback is enabled
*                                       GT_FALSE - forwarding to loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  srcProfile,
    IN  GT_U32  trgProfile,
    IN  GT_U32  tc,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortLoopbackEnableGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, srcProfile, trgProfile, tc, enablePtr));

    rc = internal_cpssDxChPortLoopbackEnableGet(
        devNum, srcProfile, trgProfile, tc, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, srcProfile, trgProfile, tc, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for FORWARD and FROM_CPU packet type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] fromCpuEn                - GT_TRUE  - enable forwarding to loopback
*                                      FORWARD, FROM_CPU traffic
*                                      GT_FALSE - enable forwarding to loopback
*                                      FORWARD traffic only
* @param[in] singleTargetEn           - GT_TRUE  - enable forwarding to loopback for
*                                      single-target traffic
*                                      GT_FALSE - disable forwarding to loopback for
*                                      single target traffic
* @param[in] multiTargetEn            - GT_TRUE  - enable forwarding to loopback for
*                                      multi-target traffic
*                                      GT_FALSE - disable forwarding to loopback for
*                                      multi-target traffic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
(
    IN GT_U8                devNum,
    IN GT_BOOL              fromCpuEn,
    IN GT_BOOL              singleTargetEn,
    IN GT_BOOL              multiTargetEn
)
{
    GT_STATUS rc;
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    value = (BOOL2BIT_MAC(singleTargetEn)       | /* bit 14 */
             (BOOL2BIT_MAC(multiTargetEn) << 1) | /* bit 15 */
             (BOOL2BIT_MAC(fromCpuEn))    << 2);  /* bit 16 */

    rc = prvCpssHwPpSetRegField(
        devNum,
        PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackForwardingGlobalConfig,
        14, 3,
        value);
    return rc;
}


/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for FORWARD and FROM_CPU packet type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] fromCpuEn                - GT_TRUE  - enable forwarding to loopback
*                                      FORWARD, FROM_CPU traffic
*                                      GT_FALSE - enable forwarding to loopback
*                                      FORWARD traffic only
* @param[in] singleTargetEn           - GT_TRUE  - enable forwarding to loopback for
*                                      single-target traffic
*                                      GT_FALSE - disable forwarding to loopback for
*                                      single target traffic
* @param[in] multiTargetEn            - GT_TRUE  - enable forwarding to loopback for
*                                      multi-target traffic
*                                      GT_FALSE - disable forwarding to loopback for
*                                      multi-target traffic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
(
    IN GT_U8                devNum,
    IN GT_BOOL              fromCpuEn,
    IN GT_BOOL              singleTargetEn,
    IN GT_BOOL              multiTargetEn
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fromCpuEn, singleTargetEn, multiTargetEn));

    rc = internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        devNum, fromCpuEn, singleTargetEn, multiTargetEn);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fromCpuEn, singleTargetEn, multiTargetEn));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}


/**
* @internal internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet function
* @endinternal
*
* @brief   Get status of loopback for FORWARD and FROM_CPU packet type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] fromCpuEnPtr             - if forwarding-to-loopback is enabled for FROM_CPU
*                                      packet type
*                                      Can be NULL
* @param[out] singleTargetEnPtr        - if single-target traffic is forwarded to loopback
*                                      Can be NULL
* @param[out] multiTargetEnPtr         - if multi-target traffic is forwarded to loopback
*                                      Can be NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                        packet type.
*                                        Can be NULL
*                                        Can be NULL
*                                        Can be NULL
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *fromCpuEnPtr,
    OUT GT_BOOL              *singleTargetEnPtr,
    OUT GT_BOOL              *multiTargetEnPtr
)
{
    GT_STATUS rc;
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    rc = prvCpssHwPpGetRegField(
        devNum,
        PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackForwardingGlobalConfig,
        14, 3, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (singleTargetEnPtr)
    {
        *singleTargetEnPtr = value & 1;      /* bit 14 */
    }

    if (multiTargetEnPtr)
    {
        *multiTargetEnPtr = (value >> 1) & 1; /* bit 15 */
    }

    if (fromCpuEnPtr)
    {
        *fromCpuEnPtr =  (value >> 2) & 1;   /* bit 16 */
    }

    return GT_OK;

}

/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet function
* @endinternal
*
* @brief   Get status of loopback for FORWARD and FROM_CPU packet type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] fromCpuEnPtr             - if forwarding-to-loopback is enabled for FROM_CPU
*                                      packet type
*                                      Can be NULL
* @param[out] singleTargetEnPtr        - if single-target traffic is forwarded to loopback
*                                      Can be NULL
* @param[out] multiTargetEnPtr         - if multi-target traffic is forwarded to loopback
*                                      Can be NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                        packet type.
*                                        Can be NULL
*                                        Can be NULL
*                                        Can be NULL
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *fromCpuEnPtr,
    OUT GT_BOOL              *singleTargetEnPtr,
    OUT GT_BOOL              *multiTargetEnPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, fromCpuEnPtr, singleTargetEnPtr, multiTargetEnPtr));

    rc = internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(
        devNum, fromCpuEnPtr, singleTargetEnPtr, multiTargetEnPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, fromCpuEnPtr, singleTargetEnPtr, multiTargetEnPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}


/**
* @internal internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet function
* @endinternal
*
* @brief   Specify loopback port for physical port number. This loopback port is
*         used for FORWARD and FROM_CPU packets targeted to this physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - target physical port number
* @param[in] loopbackPortNum          - loopback port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       cpssDxChPortInternalLoopbackEnableSet can be used to configure it.
*
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_PHYSICAL_PORT_NUM   loopbackPortNum
)
{
    GT_STATUS rc;
    GT_U32 value;
    GT_U32 txqPortNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, loopbackPortNum);

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ELOOPBACK_PORT_E,
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            loopbackPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* SIP 5.0, SIP 5.1x */
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
            devNum, loopbackPortNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
            &txqPortNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        value  = (txqPortNum & 0xFF) << 8; /* bits 15:8 */
        value |= (loopbackPortNum & 0xFF); /* bits 7:0 */
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0,
            16,
            value);

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet function
* @endinternal
*
* @brief   Specify loopback port for physical port number. This loopback port is
*         used for FORWARD and FROM_CPU packets targeted to this physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - target physical port number
* @param[in] loopbackPortNum          - loopback port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       You can use cpssDxChPortInternalLoopbackEnableSet to congifure it.
*
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_PHYSICAL_PORT_NUM   loopbackPortNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, loopbackPortNum));
    rc = internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
        devNum, portNum, loopbackPortNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, loopbackPortNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}


/**
* @internal internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet function
* @endinternal
*
* @brief   Get loopback port assigned to physical port number. The loopback port is
*         used for by FORWARD and FROM_CPU packet targeted to this physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] loopbackPortNumPtr       - loopback port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *loopbackPortNumPtr
)
{
    GT_STATUS           rc;
    GT_U32              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(loopbackPortNumPtr);
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ELOOPBACK_PORT_E,
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &value);
    }
    else
    {
        /* SIP 5.0, SIP 5.1x */
        rc = prvCpssDxChReadTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0,
            8,
            &value);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    *loopbackPortNumPtr = value;

    return GT_OK;
}

/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet function
* @endinternal
*
* @brief   Get loopback port assigned to physical port number. The loopback port is
*         used for by FORWARD and FROM_CPU packet targeted to this physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] loopbackPortNumPtr       - loopback port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *loopbackPortNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, loopbackPortNumPtr));

    rc = internal_cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
        devNum, portNum, loopbackPortNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, loopbackPortNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackPktTypeToCpuSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code. Can be CPSS_NET_ALL_CPU_OPCODES_E too.
* @param[in] loopbackPortNum          - loopback port number
* @param[in] enable                   - forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       You can use cpssDxChPortInternalLoopbackEnableSet to congifure it.
*
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeToCpuSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN GT_PHYSICAL_PORT_NUM     loopbackPortNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    GT_U32    value;
    GT_U32    mask;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT    firstCpuCode;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT    lastCpuCode;
    GT_U32    txqPortNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, loopbackPortNum);

    if (cpuCode == CPSS_NET_ALL_CPU_OPCODES_E)
    {
        firstCpuCode = 0;
        lastCpuCode  = 255;
    }
    else
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &firstCpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }

        lastCpuCode = firstCpuCode;
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        mask   =  (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ?
                   ((0x3FF << 2) | 1) :   /* bits 11:2, 0  */
                   ((0x1FF << 2) | 1));   /* bits 10:2, 0  */

        value  = loopbackPortNum << 2;  /* bits 10:2 (sip5.20) or 11:2(sip6)) */
        value |= BOOL2BIT_MAC(enable);  /* bit  0 */
    }
    else
    {
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
            devNum, loopbackPortNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
            &txqPortNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        mask   = 0x1FFFF;                    /* bits 16:0 */
        value  = BOOL2BIT_MAC(enable) << 16; /* bit  16 */
        value |= txqPortNum << 8;            /* bits 15:8 */
        value |= (loopbackPortNum & 0xFF);   /* bits 7:0 */
    }
    for (; firstCpuCode <= lastCpuCode; firstCpuCode++)
    {
        rc = prvCpssDxChWriteTableEntryMasked(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E,
            firstCpuCode,
            &value,
            &mask);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortLoopbackPktTypeToCpuSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code. Can be CPSS_NET_ALL_CPU_OPCODES_E
* @param[in] loopbackPortNum          - loopback port number
* @param[in] enable                   - forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       You can use cpssDxChPortInternalLoopbackEnableSet to congifure it.
*
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToCpuSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN GT_PHYSICAL_PORT_NUM     loopbackPortNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeToCpuSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, cpuCode, loopbackPortNum, enable));

    rc = internal_cpssDxChPortLoopbackPktTypeToCpuSet(
        devNum, cpuCode, loopbackPortNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, cpuCode, loopbackPortNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackPktTypeToCpuGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*
* @param[out] loopbackPortNumPtr       - loopback port number
* @param[out] enablePtr                - if forwarding to loopback is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeToCpuGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT GT_PHYSICAL_PORT_NUM     *loopbackPortNumPtr,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS                              rc;
    GT_U32                                 value;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(loopbackPortNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChReadTableEntry(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E,
            dsaCpuCode,
            &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        *enablePtr          = value & 0x1;           /* bit  0 */
        *loopbackPortNumPtr = (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ?
                               ((value >> 2) & 0x1FF) :  /* bits 10:2 */
                               ((value >> 2) & 0x3FF));  /* bits 11:2 */
    }
    else
    {
        /* SIP 5.0, SIP 5.1x */
        *enablePtr          = (value >> 16) & 1; /* bit  16 */
        *loopbackPortNumPtr = value & 0xFF;      /* bits 7:0 */
    }

    return GT_OK;
}


/**
* @internal cpssDxChPortLoopbackPktTypeToCpuGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*
* @param[out] loopbackPortNumPtr       - loopback port number
* @param[out] enablePtr                - if forwarding to loopback is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToCpuGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT GT_PHYSICAL_PORT_NUM     *loopbackPortNumPtr,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeToCpuGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, cpuCode, loopbackPortNumPtr, enablePtr));

    rc = internal_cpssDxChPortLoopbackPktTypeToCpuGet(
        devNum, cpuCode, loopbackPortNumPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, cpuCode, loopbackPortNumPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackPktTypeToAnalyzerSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] analyzerIndex            - analyzer index
*                                      (APPLICABLE RANGE: 0..6)
* @param[in] loopbackPortNum          - loopback port number
* @param[in] enable                   -  forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       You can use cpssDxChPortInternalLoopbackEnableSet to congifure it.
*
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeToAnalyzerSet
(
    IN GT_U8                devNum,
    IN GT_U32               analyzerIndex,
    IN GT_PHYSICAL_PORT_NUM loopbackPortNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    value;
    GT_U32    txqPortNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, loopbackPortNum);

    if (analyzerIndex > PRV_CPSS_DXCH_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    analyzerIndex++; /* hw analyzer index range is 1..7, not 0..6 */

    regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.toAnalyzerLoopback[analyzerIndex];

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        U32_SET_FIELD_MAC(value, 0, 1, BOOL2BIT_MAC(enable));
        U32_SET_FIELD_MAC(value, 2, 10, loopbackPortNum & 0x3FF);

    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        U32_SET_FIELD_MAC(value, 0, 1, BOOL2BIT_MAC(enable));
        U32_SET_FIELD_MAC(value, 2, 9, loopbackPortNum & 0x1FF);
    }
    else
    {
        /* SIP 5.0, SIP 5.1x */
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
            devNum, loopbackPortNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
            &txqPortNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        U32_SET_FIELD_MAC(value, 0,  8, loopbackPortNum & 0xFF);
        U32_SET_FIELD_MAC(value, 8,  8, txqPortNum & 0xFF);
        U32_SET_FIELD_MAC(value, 16, 1, BOOL2BIT_MAC(enable));
    }

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortLoopbackPktTypeToAnalyzerSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] analyzerIndex            - analyzer index
*                                      (APPLICABLE RANGE: 0..6)
* @param[in] loopbackPortNum          - loopback port number
* @param[in] enable                   -  forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       You can use cpssDxChPortInternalLoopbackEnableSet to congifure it.
*
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToAnalyzerSet
(
    IN GT_U8                devNum,
    IN GT_U32               analyzerIndex,
    IN GT_PHYSICAL_PORT_NUM loopbackPortNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeToAnalyzerSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, analyzerIndex, loopbackPortNum, enable));

    rc = internal_cpssDxChPortLoopbackPktTypeToAnalyzerSet(
        devNum, analyzerIndex, loopbackPortNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, analyzerIndex, loopbackPortNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackPktTypeToAnalyzerGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] analyzerIndex            - analyzer index
*                                      (APPLICABLE RANGE: 0..6)
*
* @param[out] loopbackPortNumPtr       - loopback port number
* @param[out] enablePtr                - if forwarding to loopback is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackPktTypeToAnalyzerGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               analyzerIndex,
    OUT GT_PHYSICAL_PORT_NUM *loopbackPortNumPtr,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(loopbackPortNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (analyzerIndex > PRV_CPSS_DXCH_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    analyzerIndex++; /* hw analyzer index range is 1..7, not 0..6 */

    regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.toAnalyzerLoopback[analyzerIndex];

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *enablePtr = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 0, 1));
        *loopbackPortNumPtr = U32_GET_FIELD_MAC(value, 2, 10);

    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        *enablePtr = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 0, 1));
        *loopbackPortNumPtr = U32_GET_FIELD_MAC(value, 2, 9);
    }
    else
    {
        /* SIP 5.0, SIP 5.1x */
        *enablePtr = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value, 16,1));
        *loopbackPortNumPtr = U32_GET_FIELD_MAC(value, 0, 8);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortLoopbackPktTypeToAnalyzerGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] analyzerIndex            - analyzer index
*                                      (APPLICABLE RANGE: 0..6)
*
* @param[out] loopbackPortNumPtr       - loopback port number
* @param[out] enablePtr                - if forwarding to loopback is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToAnalyzerGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               analyzerIndex,
    OUT GT_PHYSICAL_PORT_NUM *loopbackPortNumPtr,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackPktTypeToAnalyzerGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, analyzerIndex, loopbackPortNumPtr, enablePtr));

    rc = internal_cpssDxChPortLoopbackPktTypeToAnalyzerGet(
        devNum, analyzerIndex, loopbackPortNumPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, analyzerIndex, loopbackPortNumPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackProfileSet function
* @endinternal
*
* @brief   Bind source or target loopback profiles to specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - specifies which one loopback profile to configure
*                                      CPSS_DIRECTION_INGRESS_E - source loopback profile
*                                      CPSS_DIRECTION_EGRESS_E  - target loopback profile
*                                      CPSS_DIRECTION_BOTH_E    - both source/target
*                                      loopback profiles
* @param[in] profile                  - loopback profile
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DIRECTION_ENT   direction,
    IN  GT_U32               profile
)
{
    GT_STATUS           rc = GT_BAD_PARAM;

    GT_U32              regAddr;
    CPSS_DXCH_TABLE_ENT tableType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if (profile >= CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* it is not required to check "direction" explicitly.
       if no one "if" below will be visited (because of invalid direction),
       initial rc value (GT_BAD_PARAM) will not returned.
    */
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if (direction == CPSS_DIRECTION_INGRESS_E ||
            direction == CPSS_DIRECTION_BOTH_E)
        {
            tableType = CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E;
            rc = prvCpssDxChWriteTableEntryField(
                devNum, tableType, portNum,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E,
                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                profile);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (direction == CPSS_DIRECTION_EGRESS_E ||
            direction == CPSS_DIRECTION_BOTH_E)
        {
            tableType = CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E;
            rc = prvCpssDxChWriteTableEntryField(
                devNum, tableType, portNum,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E,
                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                profile);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        if (direction == CPSS_DIRECTION_INGRESS_E ||
            direction == CPSS_DIRECTION_BOTH_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.perSrcPhyPortLbProfile[portNum / 16];
            rc = prvCpssHwPpSetRegField(devNum, regAddr, portNum % 16 * 2, 2, profile);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (direction == CPSS_DIRECTION_EGRESS_E ||
            direction == CPSS_DIRECTION_BOTH_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.perTrgPhyPortLbProfile[portNum / 16];
            rc = prvCpssHwPpSetRegField(devNum, regAddr, portNum % 16 * 2, 2, profile);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortLoopbackProfileSet function
* @endinternal
*
* @brief   Bind source or target loopback profiles to specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - specifies which one loopback profile to configure
*                                      CPSS_DIRECTION_INGRESS_E - source loopback profile
*                                      CPSS_DIRECTION_EGRESS_E  - target loopback profile
*                                      CPSS_DIRECTION_BOTH_E    - both source/target
*                                      loopback profiles
* @param[in] profile                  - loopback profile
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DIRECTION_ENT   direction,
    IN  GT_U32               profile
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackProfileSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, portNum, direction, profile));

    rc = internal_cpssDxChPortLoopbackProfileSet(
        devNum, portNum, direction, profile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, portNum, direction, profile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackProfileGet function
* @endinternal
*
* @brief   Get source or target loopback profile of specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - specifies which one loopback profile get
*                                      CPSS_DIRECTION_INGRESS_E - source loopback profile
*                                      CPSS_DIRECTION_EGRESS_E  - target loopback profile
*
* @param[out] profilePtr              - loopback profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DIRECTION_ENT   direction,
    OUT GT_U32               *profilePtr
)
{
    GT_STATUS           rc;
    CPSS_DXCH_TABLE_ENT tableType; /* used fo >= SIP5.20 */
    GT_U32              regAddr;   /* used for < SIP5.20 */
    GT_U32              fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            regAddr   = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.perSrcPhyPortLbProfile[portNum / 16];
            tableType = CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E;
            fieldOffset = SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E;
            break;
        case CPSS_DIRECTION_EGRESS_E:
            regAddr   = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.perTrgPhyPortLbProfile[portNum / 16];
            tableType = CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E;
            fieldOffset = SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntryField(
            devNum, tableType, portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            fieldOffset,
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            profilePtr);
    }
    else
    {
        rc = prvCpssHwPpGetRegField(devNum, regAddr, portNum % 16 * 2, 2, profilePtr);
    }

    return rc;
}

/**
* @internal cpssDxChPortLoopbackProfileGet function
* @endinternal
*
* @brief   Get source or target loopback profile of specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - specifies which one loopback profile get
*                                      CPSS_DIRECTION_INGRESS_E - source loopback profile
*                                      CPSS_DIRECTION_EGRESS_E  - target loopback profile
*
* @param[out] profilePtr              - loopback profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DIRECTION_ENT   direction,
    OUT GT_U32               *profilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackProfileGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, portNum, direction, profilePtr));

    rc = internal_cpssDxChPortLoopbackProfileGet(
        devNum, portNum, direction, profilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, portNum, direction, profilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackEvidxMappingSet function
* @endinternal
*
* @brief   Enable eVIDX mapping for multi-target traffic forwarded to loopback and
*         specify Loopback eVIDX offset.
*         If enabled multi-target packet forwarded to loopback (this is done after
*         the replication to the port distribution list) will be assigned with
*         new eVIDX=<Loopback eVIDX offset> + <target port>.
*         The plan is to allow you configure every eVIDX in the all target ports
*         range as physical-port-distribution-list that includes only the
*         corresponding physical port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidxOffset               - a Loopback eVIDX offset.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] enable                   -  eVIDX mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEnableEgressMirroringSet
*
*/
static GT_STATUS internal_cpssDxChPortLoopbackEvidxMappingSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               vidxOffset,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VIDX_NUM_CHECK_MAC(devNum,vidxOffset);

    /* Loopback Forwarding Global Configuration Register
       13:2 Loopback eVIDX Offset
       1    Multi-Target eVIDX Mapping Enable */

    value = BOOL2BIT_MAC(enable) | (vidxOffset << 1);
    rc = prvCpssHwPpSetRegField(
        devNum,
        PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackForwardingGlobalConfig,
        1, 13, value);

    return rc;
}

/**
* @internal cpssDxChPortLoopbackEvidxMappingSet function
* @endinternal
*
* @brief   Enable eVIDX mapping for multi-target traffic forwarded to loopback and
*         specify Loopback eVIDX offset.
*         If enabled multi-target packet forwarded to loopback (this is done after
*         the replication to the port distribution list) will be assigned with
*         new eVIDX=<Loopback eVIDX offset> + <target port>.
*         The plan is to allow you configure every eVIDX in the all target ports
*         range as physical-port-distribution-list that includes only the
*         corresponding physical port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidxOffset               - a Loopback eVIDX offset.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] enable                   -  eVIDX mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEnableEgressMirroringSet
*
*/
GT_STATUS cpssDxChPortLoopbackEvidxMappingSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               vidxOffset,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackEvidxMappingSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, vidxOffset, enable));

    rc = internal_cpssDxChPortLoopbackEvidxMappingSet(
        devNum, vidxOffset, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, vidxOffset, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackEvidxMappingGet function
* @endinternal
*
* @brief   Get status and eVIDX mapping for multi-target traffic forwarded to loopback
*         and get specified specify Loopback eVIDX offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidxOffsetPtr            - a Loopback eVIDX offset.
* @param[out] enablePtr                - enable eVIDX mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortLoopbackEvidxMappingGet
(
    IN  GT_U8                devNum,
    OUT GT_U32               *vidxOffsetPtr,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(vidxOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Loopback Forwarding Global Configuration Register
       13:2 Loopback eVIDX Offset
       1    Multi-Target eVIDX Mapping Enable */
    rc = prvCpssHwPpGetRegField(
        devNum,
        PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackForwardingGlobalConfig,
        1, 13, &value);

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr     = BIT2BOOL_MAC(value & 1);
    *vidxOffsetPtr = (value >> 1);
    return GT_OK;
}

/**
* @internal cpssDxChPortLoopbackEvidxMappingGet function
* @endinternal
*
* @brief   Get status of VIDX mapping for multi-target traffic forwarded to loopback
*         and get a Loopback eVIDX offset value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidxOffsetPtr            - a Loopback eVIDX offset.
* @param[out] enablePtr                - enable eVIDX mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackEvidxMappingGet
(
    IN  GT_U8                devNum,
    OUT GT_U32               *vidxOffsetPtr,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
                                       cpssDxChPortLoopbackEvidxMappingGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,
                            devNum, vidxOffsetPtr, enablePtr));

    rc = internal_cpssDxChPortLoopbackEvidxMappingGet(
        devNum, vidxOffsetPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,
                              devNum, vidxOffsetPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackEnableEgressMirroringSet function
* @endinternal
*
* @brief   Enables egress mirroring for packets forwarded to loopback.
*          If 'egress mirroring' and 'forwarding to looback' are both enabled
*          on target port a packet destined for this port will go either to
*          loopback port (if this feature is disabled) or to
*          loopback port and analyzer port (if the feature is enabled) instead
*          of target port.
*          NOTE: the feature is relevant for ePort/eVlan egress mirroring.
*          If case of 'Physical Port Egress Mirroring' packet which didn't egress
*          via target port will not be mirrored in any case.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  -  egress mirroring
*                                      GT_FALSE - disable egress mirroring
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEvidxMappingSet
*
*/
static GT_STATUS internal_cpssDxChPortLoopbackEnableEgressMirroringSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    rc = prvCpssHwPpSetRegField(
        devNum,
        PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackForwardingGlobalConfig,
        0,1, (enable == GT_TRUE ? 0: 1));
    return rc;
}

/**
* @internal cpssDxChPortLoopbackEnableEgressMirroringSet function
* @endinternal
*
* @brief   Enables egress mirroring for packets forwarded to loopback.
*          If 'egress mirroring' and 'forwarding to looback' are both enabled
*          on target port a packet destined for this port will go either to
*          loopback port (if this feature is disabled) or to
*          loopback port and analyzer port (if the feature is enabled) instead
*          of target port.
*          NOTE: the feature is relevant for ePort/eVlan egress mirroring.
*          If case of 'Physical Port Egress Mirroring' packet which didn't egress
*          via target port will not be mirrored in any case.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  -  egress mirroring
*                                      GT_FALSE - disable egress mirroring
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEvidxMappingSet
*
*/
GT_STATUS cpssDxChPortLoopbackEnableEgressMirroringSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortLoopbackEnableEgressMirroringSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortLoopbackEnableEgressMirroringSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortLoopbackEnableEgressMirroringGet function
* @endinternal
*
* @brief   Get status of egress mirroring for packets forwarded to loopback.
*          If 'egress mirroring' and 'forwarding to looback' are both enabled
*          on target port a packet destined for this port will go either to
*          loopback port (if this feature is disabled) or to
*          loopback port and analyzer port (if the feature is enabled) instead
*          of target port.
*          NOTE: the feature is relevant for ePort/eVlan egress mirroring.
*          If case of 'Physical Port Egress Mirroring' packet which didn't egress
*          via target port will not be mirrored in any case.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE  - egress mirroring is enabled
*                                      GT_FALSE - egress mirroring is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                        GT_FALSE - egress mirroring is disabled
*/
static GT_STATUS internal_cpssDxChPortLoopbackEnableEgressMirroringGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    value;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E   | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT2_E       | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssHwPpGetRegField(
        devNum,
        PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.loopbackForwardingGlobalConfig,
        0,1, &value);

    *enablePtr =  (value ? GT_FALSE : GT_TRUE);

    return rc;
}
/**
* @internal cpssDxChPortLoopbackEnableEgressMirroringGet function
* @endinternal
*
* @brief   Get status of egress mirroring for packets forwarded to loopback.
*          If 'egress mirroring' and 'forwarding to looback' are both enabled
*          on target port a packet destined for this port will go either to
*          loopback port (if this feature is disabled) or to
*          loopback port and analyzer port (if the feature is enabled) instead
*          of target port.
*          NOTE: the feature is relevant for ePort/eVlan egress mirroring.
*          If case of 'Physical Port Egress Mirroring' packet which didn't egress
*          via target port will not be mirrored in any case.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE  - egress mirroring is enabled
*                                      GT_FALSE - egress mirroring is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                        GT_FALSE - egress mirroring is disabled
*/
GT_STATUS cpssDxChPortLoopbackEnableEgressMirroringGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortLoopbackEnableEgressMirroringGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortLoopbackEnableEgressMirroringGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
