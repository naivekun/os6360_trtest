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
* @file tgfIpfixGen.c
*
* @brief Generic API implementation for IPFIX
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfIpfixGen.h>

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChIpfixEntry function
* @endinternal
*
* @brief   Convert generic into device specific IPFIX entry
*
* @param[out] dxChEntryPtr             - (pointer to) DxCh IPFIX entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIpfixEntry
(
    IN  PRV_TGF_IPFIX_ENTRY_STC    *ipfixEntryPtr,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC  *dxChEntryPtr
)
{
    cpssOsMemSet(dxChEntryPtr, 0, sizeof(CPSS_DXCH_IPFIX_ENTRY_STC));

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, timeStamp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, packetCount);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, byteCount);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, dropCounter);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, randomOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, lastSampledValue);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, samplingWindow);

    switch(ipfixEntryPtr->samplingAction)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->samplingAction,
                                PRV_TGF_IPFIX_SAMPLING_ACTION_ALARM_E,
                                CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->samplingAction,
                                PRV_TGF_IPFIX_SAMPLING_ACTION_MIRROR_E,
                                CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, logSamplingRange);

    switch(ipfixEntryPtr->randomFlag)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->randomFlag,
                                PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E,
                                CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->randomFlag,
                                PRV_TGF_IPFIX_SAMPLING_DIST_RANDOM_E,
                                CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E);
        default:
            return GT_BAD_PARAM;
    }

    switch(ipfixEntryPtr->samplingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->samplingMode,
                                PRV_TGF_IPFIX_SAMPLING_MODE_DISABLE_E,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->samplingMode,
                                PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->samplingMode,
                                PRV_TGF_IPFIX_SAMPLING_MODE_BYTE_E,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->samplingMode,
                                PRV_TGF_IPFIX_SAMPLING_MODE_TIME_E,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, cpuSubCode);


    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericIpfixEntry function
* @endinternal
*
* @brief   Convert device specific IPFIX entry into generic
*
* @param[in] dxChEntryPtr             - (pointer to) DxCh IPFIX entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericIpfixEntry
(
    IN CPSS_DXCH_IPFIX_ENTRY_STC  *dxChEntryPtr,
    OUT PRV_TGF_IPFIX_ENTRY_STC    *ipfixEntryPtr

)
{
    cpssOsMemSet(ipfixEntryPtr, 0, sizeof(PRV_TGF_IPFIX_ENTRY_STC));

    /* convert entry info into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, timeStamp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, packetCount);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, byteCount);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, dropCounter);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, randomOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, lastSampledValue);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, samplingWindow);

    switch(dxChEntryPtr->samplingAction)
    {
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->samplingAction,
                                CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E,
                                PRV_TGF_IPFIX_SAMPLING_ACTION_ALARM_E);
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->samplingAction,
                                CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E,
                                PRV_TGF_IPFIX_SAMPLING_ACTION_MIRROR_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, logSamplingRange);

    switch(dxChEntryPtr->randomFlag)
    {
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->randomFlag,
                                CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E,
                                PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E);
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->randomFlag,
                                CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E,
                                PRV_TGF_IPFIX_SAMPLING_DIST_RANDOM_E);
        default:
            return GT_BAD_PARAM;
    }

    switch(dxChEntryPtr->samplingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->samplingMode,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E,
                                PRV_TGF_IPFIX_SAMPLING_MODE_DISABLE_E);
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->samplingMode,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E,
                                PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E);
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->samplingMode,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E,
                                PRV_TGF_IPFIX_SAMPLING_MODE_BYTE_E);
        PRV_TGF_SWITCH_CASE_MAC(ipfixEntryPtr->samplingMode,
                                CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E,
                                PRV_TGF_IPFIX_SAMPLING_MODE_TIME_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, ipfixEntryPtr, cpuSubCode);

    return GT_OK;
}
#endif

/**
* @internal prvTgfIpfixEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixEntrySet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                            entryIndex,
    IN  PRV_TGF_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_IPFIX_ENTRY_STC           dxChEntry;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    /* convert policer entry into device specific format */
    rc = prvTgfConvertGenericToDxChIpfixEntry(ipfixEntryPtr, &dxChEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIpfixEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerCountingWriteBackCacheFlush FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChIpfixEntrySet(devNum, dxChStage, entryIndex, &dxChEntry);
    }
    else
    {
        rc = cpssDxChIpfixPortGroupEntrySet(devNum, currPortGroupsBmp, dxChStage, entryIndex, &dxChEntry);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixEntrySet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
* @param[in] reset                    -  flag:
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read entry operation only.
*
* @param[out] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixEntryGet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                            entryIndex,
    IN  GT_BOOL                           reset,
    OUT PRV_TGF_IPFIX_ENTRY_STC           *ipfixEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_IPFIX_ENTRY_STC           dxChEntry;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChIpfixEntryGet(devNum, dxChStage, entryIndex, reset, &dxChEntry);
    }
    else
    {
        rc = cpssDxChIpfixPortGroupEntryGet(devNum, currPortGroupsBmp, dxChStage, entryIndex, reset, &dxChEntry);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert policer entry into generic format */
    rc = prvTgfConvertDxChToGenericIpfixEntry(&dxChEntry, ipfixEntryPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericIpfixEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixDropCountModeSet function
* @endinternal
*
* @brief   Configures IPFIX drop counting mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixDropCountModeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_IPFIX_DROP_COUNT_MODE_ENT    mode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT dxChMode;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_IPFIX_DROP_COUNT_MODE_METER_ONLY_E,
                                CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_IPFIX_DROP_COUNT_MODE_ALL_E,
                                CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E);
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChIpfixDropCountModeSet(devNum, dxChStage, dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixDropCountModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixDropCountModeGet function
* @endinternal
*
* @brief   Gets IPFIX drop counting mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixDropCountModeGet
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    OUT PRV_TGF_IPFIX_DROP_COUNT_MODE_ENT     *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT dxChMode;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpfixDropCountModeGet(devNum, dxChStage, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixDropCountModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(dxChMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E,
                                PRV_TGF_IPFIX_DROP_COUNT_MODE_METER_ONLY_E);
        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E,
                                PRV_TGF_IPFIX_DROP_COUNT_MODE_ALL_E);
        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixCpuCodeSet function
* @endinternal
*
* @brief   Sets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] cpuCode                  - A cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or cpu code.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixCpuCodeSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_NET_RX_CPU_CODE_ENT       cpuCode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpfixCpuCodeSet(devNum, dxChStage, cpuCode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixCpuCodeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixCpuCodeGet function
* @endinternal
*
* @brief   Gets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] cpuCodePtr               - pointer to a cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixCpuCodeGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT      *cpuCodePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpfixCpuCodeGet(devNum, dxChStage, cpuCodePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixCpuCodeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixAgingEnableSet function
* @endinternal
*
* @brief   Enables or disabled the activation of aging for IPfix.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixAgingEnableSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpfixAgingEnableSet(devNum, dxChStage, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixAgingEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixAgingEnableGet function
* @endinternal
*
* @brief   Gets enabling status of aging for IPfix.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - pointer to: GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixAgingEnableGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                       *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpfixAgingEnableGet(devNum, dxChStage, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixAgingEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixWraparoundConfSet function
* @endinternal
*
* @brief   Configures IPFIX wraparound parameters.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, action or threshold.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixWraparoundConfSet
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN PRV_TGF_IPFIX_WRAPAROUND_CFG_STC       *confPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC  dxChConf, *dxChConfPtr;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    dxChConfPtr = &dxChConf;

    switch(confPtr->action)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChConfPtr->action,
                                PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E,
                                CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChConfPtr->action,
                                PRV_TGF_IPFIX_WRAPAROUND_ACTION_CLEAR_E,
                                CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChConfPtr, confPtr, dropThreshold);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChConfPtr, confPtr, packetThreshold);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChConfPtr, confPtr, byteThreshold);

    /* call device specific API */
    rc = cpssDxChIpfixWraparoundConfSet(devNum, dxChStage, &dxChConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixWraparoundConfSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixWraparoundConfGet function
* @endinternal
*
* @brief   Get IPFIX wraparound configuration.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixWraparoundConfGet
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    OUT PRV_TGF_IPFIX_WRAPAROUND_CFG_STC      *confPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC  dxChConf, *dxChConfPtr;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpfixWraparoundConfGet(devNum, dxChStage, &dxChConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixWraparoundConfGet FAILED, rc = [%d]", rc);

        return rc;
    }

    dxChConfPtr = &dxChConf;
    switch(dxChConfPtr->action)
    {
        PRV_TGF_SWITCH_CASE_MAC(confPtr->action,
                                CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E,
                                PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E);
        PRV_TGF_SWITCH_CASE_MAC(confPtr->action,
                                CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E,
                                PRV_TGF_IPFIX_WRAPAROUND_ACTION_CLEAR_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChConfPtr, confPtr, dropThreshold);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChConfPtr, confPtr, packetThreshold);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChConfPtr, confPtr, byteThreshold);

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get wraparound indication on.
* @param[in] endIndex                 - the last entry index to get wraparound indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication wraparound.
*                                      each bit indicate: 0 - no WA occured, 1 - WA occured.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS prvTgfIpfixWraparoundStatusGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                         startIndex,
    IN GT_U32                         endIndex,
    IN GT_BOOL                        reset,
    OUT GT_U32                        *bmpPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChIpfixWraparoundStatusGet(devNum, dxChStage, startIndex, endIndex, reset, bmpPtr);
    }
    else
    {
        rc = cpssDxChIpfixPortGroupWraparoundStatusGet(devNum, currPortGroupsBmp, dxChStage, startIndex, endIndex, reset, bmpPtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixWraparoundStatusGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get aging indication on.
* @param[in] endIndex                 - the last entry index to get aging indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication aging.
*                                      each bit indicate: 0 - no packet recieved on flow (aged).
*                                      1 - packet recieved on flow.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS prvTgfIpfixAgingStatusGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                         startIndex,
    IN GT_U32                         endIndex,
    IN GT_BOOL                        reset,
    OUT GT_U32                        *bmpPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChIpfixAgingStatusGet(devNum, dxChStage, startIndex, endIndex, reset, bmpPtr);
    }
    else
    {
        rc = cpssDxChIpfixPortGroupAgingStatusGet(devNum, currPortGroupsBmp, dxChStage, startIndex, endIndex, reset, bmpPtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixAgingStatusGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixTimestampUploadSet function
* @endinternal
*
* @brief   Configures IPFIX timestamp upload per stages.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or upload mode.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Actual upload operation is triggered by
*       "prvTgfIpfixTimestampUploadTrigger".
*
*/
GT_STATUS prvTgfIpfixTimestampUploadSet
(
    IN GT_U8                                      devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    IN PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    *uploadPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC  dxChUpload, *dxChUploadPtr;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    dxChUploadPtr = &dxChUpload;

    switch(uploadPtr->uploadMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChUploadPtr->uploadMode,
                                PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E,
                                CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChUploadPtr->uploadMode,
                                PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E,
                                CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChUploadPtr, uploadPtr, timer.nanoSecondTimer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChUploadPtr, uploadPtr, timer.secondTimer);

    /* call device specific API */
    rc = cpssDxChIpfixTimestampUploadSet(devNum, dxChStage, dxChUploadPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixTimestampUploadSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixTimestampUploadGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload configuration for a stage.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimestampUploadGet
(
    IN GT_U8                                      devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    OUT PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC   *uploadPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC  dxChUpload, *dxChUploadPtr;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    dxChUploadPtr = &dxChUpload;
    /* call device specific API */
    rc = cpssDxChIpfixTimestampUploadGet(devNum, dxChStage, dxChUploadPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixTimestampUploadGet FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(dxChUploadPtr->uploadMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(uploadPtr->uploadMode,
                                CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E,
                                PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E);
        PRV_TGF_SWITCH_CASE_MAC(uploadPtr->uploadMode,
                                CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E,
                                PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E);
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChUploadPtr, uploadPtr, timer.nanoSecondTimer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChUploadPtr, uploadPtr, timer.secondTimer);

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixTimestampUploadTrigger function
* @endinternal
*
* @brief   Triggers the IPFIX timestamp upload operation.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_STATE             - if previous upload operation is not
*                                       finished yet.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimestampUploadTrigger
(
    IN GT_U8    devNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    /* call device specific API */
    rc = cpssDxChIpfixTimestampUploadTrigger(devNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixTimestampUploadTrigger FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixTimestampUploadStatusGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload operation status.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] uploadStatusPtr          - pointer to indication whether upload was done.
*                                      GT_TRUE: upload is finished.
*                                      GT_FALSE: upload still in progess.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimestampUploadStatusGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *uploadStatusPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    /* call device specific API */
    rc = cpssDxChIpfixTimestampUploadStatusGet(devNum, uploadStatusPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixTimestampUploadStatusGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixTimerGet function
* @endinternal
*
* @brief   Gets IPFIX timer
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] timerPtr                 - pointer to IPFIX timer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimerGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT PRV_TGF_IPFIX_TIMER_STC       *timerPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_IPFIX_TIMER_STC  dxChTimer, *dxChTimerPtr;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    dxChTimerPtr = &dxChTimer;
    /* call device specific API */
    rc = cpssDxChIpfixTimerGet(devNum, dxChStage, dxChTimerPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixTimerGet FAILED, rc = [%d]", rc);

        return rc;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTimerPtr, timerPtr, nanoSecondTimer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTimerPtr, timerPtr, secondTimer);

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[PRV_TGF_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      Range: 0..15
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixAlarmEventsGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                        eventsArr[PRV_TGF_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                        *eventsNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;

    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChIpfixAlarmEventsGet(devNum, dxChStage, eventsArr, eventsNumPtr);
    }
    else
    {
        rc = cpssDxChIpfixPortGroupAlarmEventsGet(devNum, currPortGroupsBmp, dxChStage, eventsArr, eventsNumPtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixAlarmEventsGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixTimestampToCpuEnableSet function
* @endinternal
*
* @brief   Configures all TO_CPU DSA tags to include a Timestamp.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimestampToCpuEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpfixTimestampToCpuEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixTimestampToCpuEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixTimestampToCpuEnableGet function
* @endinternal
*
* @brief   Gets enabling status of TO_CPU DSA tags including a timestamp.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimestampToCpuEnableGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpfixTimestampToCpuEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpfixTimestampToCpuEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpfixIsResetOnReadErratum function
* @endinternal
*
*
* @retval GT_TRUE                  - on erratum exist
* @retval GT_FALSE                 - on erratum does not exist
*/
GT_BOOL prvTgfIpfixIsResetOnReadErratum
(
    IN GT_U8     devNum
)
{
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && (GT_TRUE == prvUtfIsGmCompilation()))
    {
        /* GM does not have the erratum yet */
        return GT_FALSE;
    }

    /* erratum is in devices starting from BC3 */
    return PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);
}



