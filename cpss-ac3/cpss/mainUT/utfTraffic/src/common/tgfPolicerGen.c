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
* @file tgfPolicerGen.c
*
* @brief Generic API for Ingress Policing Engine API.
*
* @version   41
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
#include <common/tgfPolicerGen.h>

#ifdef ASIC_SIMULATION
/* #include <asicSimulation/SKernel/smain/smain.h> */
extern GT_STATUS skernelPolicerConformanceLevelForce(
    IN  GT_U32      dp
);
#endif

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxCh3/policer/private/prvCpssDxCh3Policer.h>
#endif /*CHX_FAMILY*/

/* Flag indicating if eArch related metering entry fields must be configured
   explicitly */
GT_BOOL prvTgfPolicerEntryUseEarch;

/* Save configured metering resolution to be used in eArch metering entry set */
PRV_TGF_POLICER_METER_RESOLUTION_ENT
    prvTgfPolicerEarchMeterResolution[PRV_TGF_POLICER_STAGE_NUM];

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChPolicerEntry function
* @endinternal
*
* @brief   Convert generic into device specific policer entry
*
* @param[in] policerEntryPtr          - (pointer to) policer entry
*
* @param[out] dxChEntryPtr             - (pointer to) DxCh policer entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChPolicerEntry
(
    IN  PRV_TGF_POLICER_ENTRY_STC    *policerEntryPtr,
    OUT CPSS_DXCH_POLICER_ENTRY_STC  *dxChEntryPtr
)
{
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, policerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, meterColorMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, counterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, counterSetIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, qosProfile);

    /* convert command into device specific format */
    switch (policerEntryPtr->cmd)
    {
        case PRV_TGF_POLICER_CMD_NONE_E:
            dxChEntryPtr->cmd = CPSS_DXCH_POLICER_CMD_NONE_E;
            break;

        case PRV_TGF_POLICER_CMD_DROP_RED_E:
            dxChEntryPtr->cmd = CPSS_DXCH_POLICER_CMD_DROP_RED_E;
            break;

        case PRV_TGF_POLICER_CMD_QOS_MARK_BY_ENTRY_E:
            dxChEntryPtr->cmd = CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
            break;

        case PRV_TGF_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E:
            dxChEntryPtr->cmd = CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert DSCP modify mode into device specific format */
    switch (policerEntryPtr->modifyDscp)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyDscp,
                                PRV_TGF_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyDscp,
                                PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyDscp,
                                PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert UP modify mode into device specific format */
    switch (policerEntryPtr->modifyUp)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyUp,
                                PRV_TGF_POLICER_MODIFY_UP_KEEP_PREVIOUS_E,
                                CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyUp,
                                PRV_TGF_POLICER_MODIFY_UP_DISABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyUp,
                                PRV_TGF_POLICER_MODIFY_UP_ENABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert tbParams into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEntryPtr->tbParams),
                                   &(policerEntryPtr->tbParams.srTcmParams), cir);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChEntryPtr->tbParams),
                                   &(policerEntryPtr->tbParams.srTcmParams), cbs);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericPolicerEntry function
* @endinternal
*
* @brief   Convert generic from device specific policer entry
*
* @param[in] dxChEntryPtr             - (pointer to) DxCh policer entry parameters
*
* @param[out] policerEntryPtr          - (pointer to) policer entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericPolicerEntry
(
    IN  CPSS_DXCH_POLICER_ENTRY_STC  *dxChEntryPtr,
    OUT PRV_TGF_POLICER_ENTRY_STC    *policerEntryPtr

)
{
    /* convert entry info from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, policerEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, meterColorMode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, counterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, counterSetIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, qosProfile);

    /* convert command from device specific format */
    switch (dxChEntryPtr->cmd)
    {
        case CPSS_DXCH_POLICER_CMD_NONE_E:
            policerEntryPtr->cmd = PRV_TGF_POLICER_CMD_NONE_E;
            break;

        case CPSS_DXCH_POLICER_CMD_DROP_RED_E:
            policerEntryPtr->cmd = PRV_TGF_POLICER_CMD_DROP_RED_E;
            break;

        case CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E:
            policerEntryPtr->cmd = PRV_TGF_POLICER_CMD_QOS_MARK_BY_ENTRY_E;
            break;

        case CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E:
            policerEntryPtr->cmd = PRV_TGF_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert DSCP modify mode from device specific format */
    switch (dxChEntryPtr->modifyDscp)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyDscp,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E,
                                PRV_TGF_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyDscp,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E,
                                PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyDscp,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E,
                                PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert UP modify mode from device specific format */
    switch (dxChEntryPtr->modifyUp)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyUp,
                                CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E,
                                PRV_TGF_POLICER_MODIFY_UP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyUp,
                                CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E,
                                PRV_TGF_POLICER_MODIFY_UP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyUp,
                                CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E,
                                PRV_TGF_POLICER_MODIFY_UP_ENABLE_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert tbParams into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEntryPtr->tbParams),
                                   &(policerEntryPtr->tbParams.srTcmParams), cir);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChEntryPtr->tbParams),
                                   &(policerEntryPtr->tbParams.srTcmParams), cbs);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxCh3PolicerMeteringEntry function
* @endinternal
*
* @brief   Convert generic into device specific policer entry
*
* @param[in] policerEntryPtr          - (pointer to) policer entry
*
* @param[out] dxChEntryPtr             - (pointer to) DxCh policer entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxCh3PolicerMeteringEntry
(
    IN  PRV_TGF_POLICER_ENTRY_STC             *policerEntryPtr,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC *dxChEntryPtr
)
{
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, countingEntryIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, meterColorMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, modifyDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, modifyExp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, modifyTc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, packetSizeMode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, yellowEcnMarkingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, greenMirrorToAnalyzerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, yellowMirrorToAnalyzerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, redMirrorToAnalyzerEnable);

    /* convert mngCounterSet into device specific format */
    switch (policerEntryPtr->mngCounterSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            dxChEntryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            dxChEntryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            dxChEntryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            dxChEntryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert meterMode into device specific format */
    switch (policerEntryPtr->meterMode)
    {
        case PRV_TGF_POLICER_METER_MODE_SR_TCM_E:
            dxChEntryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;

            /* convert tbParams into device specific format */
            dxChEntryPtr->tokenBucketParams.srTcmParams.cir =
                  policerEntryPtr->tbParams.srTcmParams.cir;
            dxChEntryPtr->tokenBucketParams.srTcmParams.cbs =
                  policerEntryPtr->tbParams.srTcmParams.cbs;
            dxChEntryPtr->tokenBucketParams.srTcmParams.ebs =
                  policerEntryPtr->tbParams.srTcmParams.ebs;

            break;

        case PRV_TGF_POLICER_METER_MODE_TR_TCM_E:
            dxChEntryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E;

            /* convert tbParams into device specific format */
            dxChEntryPtr->tokenBucketParams.trTcmParams.cir =
                  policerEntryPtr->tbParams.trTcmParams.cir;
            dxChEntryPtr->tokenBucketParams.trTcmParams.cbs =
                  policerEntryPtr->tbParams.trTcmParams.cbs;
            dxChEntryPtr->tokenBucketParams.trTcmParams.pir =
                  policerEntryPtr->tbParams.trTcmParams.pir;
            dxChEntryPtr->tokenBucketParams.trTcmParams.pbs =
                  policerEntryPtr->tbParams.trTcmParams.pbs;

            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert yellowPcktCmd into device specific format */
    switch (policerEntryPtr->yellowPcktCmd)
    {
        case PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            dxChEntryPtr->yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E:
            dxChEntryPtr->yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E:
            dxChEntryPtr->yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            dxChEntryPtr->yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert redPcktCmd into device specific format */
    switch (policerEntryPtr->redPcktCmd)
    {
        case PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            dxChEntryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E:
            dxChEntryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E:
            dxChEntryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            dxChEntryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert remarkMode into device specific format */
    switch (policerEntryPtr->remarkMode)
    {
        case PRV_TGF_POLICER_REMARK_MODE_L2_E:
            dxChEntryPtr->remarkMode = CPSS_DXCH_POLICER_REMARK_MODE_L2_E;
            break;

        case PRV_TGF_POLICER_REMARK_MODE_L3_E:
            dxChEntryPtr->remarkMode = CPSS_DXCH_POLICER_REMARK_MODE_L3_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert greenPcktCmd into device specific format */
    switch (policerEntryPtr->greenPcktCmd)
    {
        case PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            dxChEntryPtr->greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E:
            dxChEntryPtr->greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E:
            dxChEntryPtr->greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;

        case PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            dxChEntryPtr->greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert resolution into device specific format */
    switch (policerEntryPtr->byteOrPacketCountingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->byteOrPacketCountingMode,
                                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E,
                                CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->byteOrPacketCountingMode,
                                PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E,
                                CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert TT/TS packet size mode into device specific format */
    switch (policerEntryPtr->tunnelTerminationPacketSizeMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->tunnelTerminationPacketSizeMode,
                                PRV_TGF_POLICER_TT_PACKET_SIZE_REGULAR_E,
                                CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->tunnelTerminationPacketSizeMode,
                                PRV_TGF_POLICER_TT_PACKET_SIZE_PASSENGER_E,
                                CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert DSA tag counting mode into device specific format */
    switch (policerEntryPtr->dsaTagCountingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->dsaTagCountingMode,
                                PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E,
                                CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->dsaTagCountingMode,
                                PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E,
                                CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert timestamp tag counting mode into device specific format */
    switch (policerEntryPtr->timeStampCountingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->timeStampCountingMode,
                                PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E,
                                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->timeStampCountingMode,
                                PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E,
                                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert DSCP modify mode into device specific format */
    switch (policerEntryPtr->modifyDscp)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyDscp,
                                PRV_TGF_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyDscp,
                                PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyDscp,
                                PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyDscp,
                                PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_INNER_E,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_INNER_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert UP modify mode into device specific format */
    switch (policerEntryPtr->modifyUp)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyUp,
                                PRV_TGF_POLICER_MODIFY_UP_KEEP_PREVIOUS_E,
                                CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyUp,
                                PRV_TGF_POLICER_MODIFY_UP_DISABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyUp,
                                PRV_TGF_POLICER_MODIFY_UP_ENABLE_E,
                                CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChEntryPtr->modifyUp,
                                PRV_TGF_POLICER_MODIFY_UP_ENABLE_TAG0_E,
                                CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_TAG0_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxCh3ToGenericPolicerEntry function
* @endinternal
*
* @brief   Convert generic from device specific policer entry
*
* @param[in] dxChEntryPtr             - (pointer to) DxCh policer entry parameters
*
* @param[out] policerEntryPtr          - (pointer to) policer entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxCh3ToGenericPolicerEntry
(
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC *dxChEntryPtr,
    OUT PRV_TGF_POLICER_ENTRY_STC             *policerEntryPtr
)
{
    /* convert entry info from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, countingEntryIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, meterColorMode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, modifyDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, modifyExp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, modifyTc);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, packetSizeMode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, yellowEcnMarkingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, policerEntryPtr, qosProfile);

    /* convert mngCounterSet from device specific format */
    switch (dxChEntryPtr->mngCounterSet)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
            break;

        case CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET1_E;
            break;

        case CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET2_E;
            break;

        case CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert meterMode from device specific format */
    switch (dxChEntryPtr->meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            policerEntryPtr->meterMode = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;

            /* convert tbParams from device specific format */
            policerEntryPtr->tbParams.srTcmParams.cir =
                  dxChEntryPtr->tokenBucketParams.srTcmParams.cir;
            policerEntryPtr->tbParams.srTcmParams.cbs =
                  dxChEntryPtr->tokenBucketParams.srTcmParams.cbs;
            policerEntryPtr->tbParams.srTcmParams.ebs =
                  dxChEntryPtr->tokenBucketParams.srTcmParams.ebs;

            break;

        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
            policerEntryPtr->meterMode = PRV_TGF_POLICER_METER_MODE_TR_TCM_E;

            /* convert tbParams from device specific format */
            policerEntryPtr->tbParams.trTcmParams.cir =
                  dxChEntryPtr->tokenBucketParams.trTcmParams.cir;
            policerEntryPtr->tbParams.trTcmParams.cbs =
                  dxChEntryPtr->tokenBucketParams.trTcmParams.cbs;
            policerEntryPtr->tbParams.trTcmParams.pir =
                  dxChEntryPtr->tokenBucketParams.trTcmParams.pir;
            policerEntryPtr->tbParams.trTcmParams.pbs =
                  dxChEntryPtr->tokenBucketParams.trTcmParams.pbs;

            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert yellowPcktCmd from device specific format */
    switch (dxChEntryPtr->yellowPcktCmd)
    {
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            policerEntryPtr->yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
            policerEntryPtr->yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
            policerEntryPtr->yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            policerEntryPtr->yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert redPcktCmd from device specific format */
    switch (dxChEntryPtr->redPcktCmd)
    {
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            policerEntryPtr->redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
            policerEntryPtr->redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
            policerEntryPtr->redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            policerEntryPtr->redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert remarkMode from device specific format */
    switch (dxChEntryPtr->remarkMode)
    {
        case CPSS_DXCH_POLICER_REMARK_MODE_L2_E:
            policerEntryPtr->remarkMode = PRV_TGF_POLICER_REMARK_MODE_L2_E;
            break;

        case CPSS_DXCH_POLICER_REMARK_MODE_L3_E:
            policerEntryPtr->remarkMode = PRV_TGF_POLICER_REMARK_MODE_L3_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert greenPcktCmd from device specific format */
    switch (dxChEntryPtr->greenPcktCmd)
    {
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            policerEntryPtr->greenPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
            policerEntryPtr->greenPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
            policerEntryPtr->greenPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;

        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            policerEntryPtr->greenPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert resolution from device specific format */
    switch (dxChEntryPtr->byteOrPacketCountingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->byteOrPacketCountingMode,
                                CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E,
                                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->byteOrPacketCountingMode,
                                CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E,
                                PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert TT/TS packet size mode from device specific format */
    switch (dxChEntryPtr->tunnelTerminationPacketSizeMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->tunnelTerminationPacketSizeMode,
                                CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E,
                                PRV_TGF_POLICER_TT_PACKET_SIZE_REGULAR_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->tunnelTerminationPacketSizeMode,
                                CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E,
                                PRV_TGF_POLICER_TT_PACKET_SIZE_PASSENGER_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert DSA tag counting mode from device specific format */
    switch (dxChEntryPtr->dsaTagCountingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->dsaTagCountingMode,
                                CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E,
                                PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->dsaTagCountingMode,
                                CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E,
                                PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert timestamp tag counting mode from device specific format */
    switch (dxChEntryPtr->timeStampCountingMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->timeStampCountingMode,
                                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E,
                                PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->timeStampCountingMode,
                                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E,
                                PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert DSCP modify mode from device specific format */
    switch (dxChEntryPtr->modifyDscp)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyDscp,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E,
                                PRV_TGF_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyDscp,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E,
                                PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyDscp,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E,
                                PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyDscp,
                                CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_INNER_E,
                                PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_INNER_E);

        default:
            return GT_BAD_PARAM;
    }

    /* convert UP modify mode from device specific format */
    switch (dxChEntryPtr->modifyUp)
    {
        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyUp,
                                CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E,
                                PRV_TGF_POLICER_MODIFY_UP_KEEP_PREVIOUS_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyUp,
                                CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E,
                                PRV_TGF_POLICER_MODIFY_UP_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyUp,
                                CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E,
                                PRV_TGF_POLICER_MODIFY_UP_ENABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(policerEntryPtr->modifyUp,
                                CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_TAG0_E,
                                PRV_TGF_POLICER_MODIFY_UP_ENABLE_TAG0_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChPolicerStage function
* @endinternal
*
* @brief   Convert generic into device specific policer stage
*
* @param[in] policerStage             - (pointer to) policer stage
*
* @param[out] dxChStagePtr             - (pointer to) DxCh policer stage
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChPolicerStage
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      policerStage,
    OUT CPSS_DXCH_POLICER_STAGE_TYPE_ENT    *dxChStagePtr
)
{
    /* convert command into device specific format */
    switch (policerStage)
    {
        case PRV_TGF_POLICER_STAGE_INGRESS_0_E:
            *dxChStagePtr = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            break;

        case PRV_TGF_POLICER_STAGE_INGRESS_1_E:
            *dxChStagePtr = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            break;

        case PRV_TGF_POLICER_STAGE_EGRESS_E:
            *dxChStagePtr = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}


/**
* @internal prvTgfConvertGenericToDxChTriggerEntry function
* @endinternal
*
* @brief   Convert generic into device specific Policer Trigger entry
*
* @param[in] triggerEntryPtr          - (pointer to) policer entry
*
* @param[out] dxChEntryPtr             - (pointer to) DxCh policer entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChTriggerEntry
(
    IN  PRV_TGF_POLICER_TRIGGER_ENTRY_STC    *triggerEntryPtr,
    OUT CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC  *dxChEntryPtr
)
{
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, meteringEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, countingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, policerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucKnownEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucKnownOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucUnknownEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucUnknownOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcRegisteredEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcRegisteredOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcUnregisteredEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcUnregisteredOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, bcEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, bcOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, tcpSynEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, tcpSynOffset);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericTriggerEntry function
* @endinternal
*
* @brief   Convert generic from device specific policer trigger entry
*
* @param[in] dxChEntryPtr             - (pointer to) DxCh policer entry parameters
*
* @param[out] triggerEntryPtr          - (pointer to) trigger entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericTriggerEntry
(
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC  *dxChEntryPtr,
    OUT PRV_TGF_POLICER_TRIGGER_ENTRY_STC    *triggerEntryPtr
)
{
    /* convert entry from device specific format into generic format*/
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, meteringEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, countingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, policerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucKnownEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucKnownOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucUnknownEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, ucUnknownOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcRegisteredEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcRegisteredOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcUnregisteredEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, mcUnregisteredOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, bcEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, bcOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, tcpSynEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChEntryPtr, triggerEntryPtr, tcpSynOffset);

    return GT_OK;
}
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
/**
* @internal prvTgfConvertGenericToExMxPmPolicerDirection function
* @endinternal
*
* @brief   Convert generic into device specific policer stage
*
* @param[in] policerStage             - (pointer to) policer stage
*
* @param[out] directionPtr             - (pointer to) policer direction
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToExMxPmPolicerDirection
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      policerStage,
    OUT CPSS_DIRECTION_ENT                  *directionPtr

)
{
    /* convert command into device specific format */
    switch (policerStage)
    {
        case PRV_TGF_POLICER_STAGE_INGRESS_0_E:
            *directionPtr = CPSS_DIRECTION_INGRESS_E;
            break;

        case PRV_TGF_POLICER_STAGE_EGRESS_E:
            *directionPtr = CPSS_DIRECTION_EGRESS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

static void prvTgfConvertModifyQosToBool
(
    IN CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyQos,
    OUT GT_BOOL  *enableModifyPtr
)
{
    switch (modifyQos)
    {
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            *enableModifyPtr = GT_TRUE;
            break;
        default: *enableModifyPtr = GT_FALSE; break;
    }
}

static void prvTgfConvertBoolToModifyQos
(
    IN GT_BOOL  enableModify,
    OUT CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT *modifyQosPtr
)
{
    *modifyQosPtr = (enableModify == GT_FALSE) ?
            CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
}
/**
* @internal prvTgfConvertGenericToExMxPmPolicerEntry function
* @endinternal
*
* @brief   Convert generic into device specific policer entry
*
* @param[in] policerEntryPtr          - (pointer to) policer entry
*
* @param[out] exMxPmEntryPtr           - (pointer to) ExMxPm policer entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmPolicerEntry
(
    IN  PRV_TGF_POLICER_ENTRY_STC               *policerEntryPtr,
    OUT CPSS_EXMXPM_POLICER_METERING_ENTRY_STC  *exMxPmEntryPtr
)
{
    /* convert entry info into device specific format */
    switch (policerEntryPtr->redPcktCmd)
    {
        case PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
        case PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E:
            exMxPmEntryPtr->dropRed = GT_FALSE;
            break;
        case PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E:
            exMxPmEntryPtr->dropRed = GT_TRUE;
            break;
        default: return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmEntryPtr, policerEntryPtr, meterColorMode);

    exMxPmEntryPtr->countingEntryIndex = policerEntryPtr->countingEntryIndex;

    prvTgfConvertModifyQosToBool(policerEntryPtr->modifyDp, &(exMxPmEntryPtr->modifyQosParams.modifyDp));
    prvTgfConvertModifyQosToBool(policerEntryPtr->modifyTc, &(exMxPmEntryPtr->modifyQosParams.modifyTc));
    prvTgfConvertModifyQosToBool(policerEntryPtr->modifyUp, &(exMxPmEntryPtr->modifyQosParams.modifyUp));
    prvTgfConvertModifyQosToBool(policerEntryPtr->modifyDscp, &(exMxPmEntryPtr->modifyQosParams.modifyDscp));
    exMxPmEntryPtr->modifyQosParams.modifyExp = GT_FALSE;

    /* convert remarkMode into device specific format */
    switch (policerEntryPtr->remarkMode)
    {
        case PRV_TGF_POLICER_REMARK_MODE_L2_E:
            exMxPmEntryPtr->remarkMode = CPSS_EXMXPM_POLICER_REMARK_MODE_L2_E;
            break;

        case PRV_TGF_POLICER_REMARK_MODE_L3_E:
            exMxPmEntryPtr->remarkMode = CPSS_EXMXPM_POLICER_REMARK_MODE_L3_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert meterMode into device specific format */
    switch (policerEntryPtr->meterMode)
    {
        case PRV_TGF_POLICER_METER_MODE_SR_TCM_E:
            exMxPmEntryPtr->meterMode = CPSS_EXMXPM_POLICER_METER_MODE_SR_TCM_E;

            /* convert tbParams into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.srTcmParams),
                                           &(policerEntryPtr->tbParams.srTcmParams), cir);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.srTcmParams),
                                           &(policerEntryPtr->tbParams.srTcmParams), cbs);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.srTcmParams),
                                           &(policerEntryPtr->tbParams.srTcmParams), ebs);

            break;

        case PRV_TGF_POLICER_METER_MODE_TR_TCM_E:
            exMxPmEntryPtr->meterMode = CPSS_EXMXPM_POLICER_METER_MODE_TR_TCM_E;

            /* convert tbParams into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), cir);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), cbs);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), pir);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), pbs);

            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCounterSet into device specific format */
    switch (policerEntryPtr->mngCounterSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            exMxPmEntryPtr->mngCounterSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            exMxPmEntryPtr->mngCounterSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            exMxPmEntryPtr->mngCounterSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            exMxPmEntryPtr->mngCounterSet = CPSS_EXMXPM_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericPolicerEntry function
* @endinternal
*
* @brief   Convert generic from device specific policer entry
*
* @param[in] exMxPmEntryPtr           - (pointer to) ExMxPm policer entry parameters
*
* @param[out] policerEntryPtr          - (pointer to) policer entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericPolicerEntry
(
    IN  CPSS_EXMXPM_POLICER_METERING_ENTRY_STC  *exMxPmEntryPtr,
    OUT PRV_TGF_POLICER_ENTRY_STC               *policerEntryPtr
)
{
    /* convert entry info from device specific format */
    policerEntryPtr->redPcktCmd = (exMxPmEntryPtr->dropRed == GT_FALSE) ?
                                PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E :
                                PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmEntryPtr, policerEntryPtr, meterColorMode);

    policerEntryPtr->counterSetIndex = exMxPmEntryPtr->countingEntryIndex;

    /* convert modifyQosParams from device specific format */
    prvTgfConvertBoolToModifyQos(exMxPmEntryPtr->modifyQosParams.modifyDp, &(policerEntryPtr->modifyDp));
    prvTgfConvertBoolToModifyQos(exMxPmEntryPtr->modifyQosParams.modifyTc, &(policerEntryPtr->modifyTc));

    policerEntryPtr->modifyUp = (exMxPmEntryPtr->modifyQosParams.modifyUp == GT_FALSE) ?
            PRV_TGF_POLICER_MODIFY_UP_KEEP_PREVIOUS_E:
            PRV_TGF_POLICER_MODIFY_UP_ENABLE_E;

    policerEntryPtr->modifyDscp = (exMxPmEntryPtr->modifyQosParams.modifyDscp == GT_FALSE) ?
            PRV_TGF_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E:
            PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_E;

    /* convert remarkMode from device specific format */
    switch (exMxPmEntryPtr->remarkMode)
    {
        case CPSS_EXMXPM_POLICER_REMARK_MODE_L2_E:
            policerEntryPtr->remarkMode = PRV_TGF_POLICER_REMARK_MODE_L2_E;
            break;

        case CPSS_EXMXPM_POLICER_REMARK_MODE_L3_E:
            policerEntryPtr->remarkMode = PRV_TGF_POLICER_REMARK_MODE_L3_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert meterMode from device specific format */
    switch (exMxPmEntryPtr->meterMode)
    {
        case CPSS_EXMXPM_POLICER_METER_MODE_SR_TCM_E:
            policerEntryPtr->meterMode = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;

            /* convert tbParams from device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.srTcmParams),
                                           &(policerEntryPtr->tbParams.srTcmParams), cir);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.srTcmParams),
                                           &(policerEntryPtr->tbParams.srTcmParams), cbs);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.srTcmParams),
                                           &(policerEntryPtr->tbParams.srTcmParams), ebs);

            break;

        case CPSS_EXMXPM_POLICER_METER_MODE_TR_TCM_E:
            policerEntryPtr->meterMode = PRV_TGF_POLICER_METER_MODE_TR_TCM_E;

            /* convert tbParams from device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), cir);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), cbs);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), pir);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmEntryPtr->tokenBucketsParams.trTcmParams),
                                           &(policerEntryPtr->tbParams.trTcmParams), pbs);

            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCounterSet from device specific format */
    switch (exMxPmEntryPtr->mngCounterSet)
    {
        case CPSS_EXMXPM_POLICER_MNG_CNTR_SET0_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
            break;

        case CPSS_EXMXPM_POLICER_MNG_CNTR_SET1_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET1_E;
            break;

        case CPSS_EXMXPM_POLICER_MNG_CNTR_SET2_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET2_E;
            break;

        case CPSS_EXMXPM_POLICER_MNG_CNTR_DISABLED_E:
            policerEntryPtr->mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfPolicerInit function
* @endinternal
*
* @brief   Init Traffic Conditioner facility on specified device
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - on illegal devNum
*/
GT_STATUS prvTgfPolicerInit
(
    GT_VOID
)
{
    GT_U8     devNum;
    PRV_TGF_POLICER_STAGE_TYPE_ENT  stage;
    GT_STATUS rc, rc1 = GT_OK;


    prvTgfPolicerEntryUseEarch = GT_FALSE;
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        prvTgfPolicerEarchMeterResolution[stage] =
                                    PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
    }

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerInit(devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerInit FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerInit(devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerInit FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables metering per device.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMeteringEnableSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerMeteringEnableSet(devNum, dxChStage, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerMeteringEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmPolicerDirectione FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerMeteringEnableSet(devNum, direction, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerMeteringEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringEnableGet function
* @endinternal
*
* @brief   Gets device metering status (Enable/Disable).
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - pointer to Enable/Disable metering
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMeteringEnableGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                        *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerMeteringEnableGet(devNum, dxChStage, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeteringEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmPolicerDirectione FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerMeteringEnableGet(devNum, direction, enablePtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerBillingCountingEnableSet function
* @endinternal
*
* @brief   Enables or disables Billing Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerBillingCountingEnableSet
(
    IN GT_BOOL  enable
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerBillingCountingEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerBillingCountingEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerBillingCountingEnableGet function
* @endinternal
*
* @brief   Gets device Billing Counting status (Enable/Disable).
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer to Enable/Disable Billing Counting
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerBillingCountingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* call device specific API */
    rc = cpssDxCh3PolicerBillingCountingEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerBillingCountingEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountingModeSet function
* @endinternal
*
* @brief   Configures counting mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     -  counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
*/
GT_STATUS prvTgfPolicerCountingModeSet
(
    IN GT_U8                             devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN PRV_TGF_POLICER_COUNTING_MODE_ENT mode
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_POLICER_COUNTING_MODE_ENT dxChMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert command into device specific format */
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_COUNTING_DISABLE_E,
                                CPSS_DXCH_POLICER_COUNTING_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E,
                                CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_COUNTING_POLICY_E,
                                CPSS_DXCH_POLICER_COUNTING_POLICY_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_COUNTING_VLAN_E,
                                CPSS_DXCH_POLICER_COUNTING_VLAN_E);

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChPolicerCountingModeSet(devNum, dxChStage, dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerCountingModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_COUNTING_MODE_ENT    pmMode;
    CPSS_EXMXPM_POLICER_CONFIG_STC           policerConf;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmPolicerDirectione FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert command into device specific format */
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(pmMode,
                                PRV_TGF_POLICER_COUNTING_DISABLE_E,
                                CPSS_EXMXPM_POLICER_COUNTING_DISABLE_E);

        PRV_TGF_SWITCH_CASE_MAC(pmMode,
                                PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E,
                                CPSS_EXMXPM_POLICER_COUNTING_BILLING_E);

        PRV_TGF_SWITCH_CASE_MAC(pmMode,
                                PRV_TGF_POLICER_COUNTING_POLICY_E,
                                CPSS_EXMXPM_POLICER_COUNTING_POLICY_E);

        PRV_TGF_SWITCH_CASE_MAC(pmMode,
                                PRV_TGF_POLICER_COUNTING_VLAN_E,
                                CPSS_EXMXPM_POLICER_COUNTING_VLAN_E);

        default:
            return GT_BAD_PARAM;
    }

    if (pmMode == CPSS_EXMXPM_POLICER_COUNTING_DISABLE_E)
    {
        return cpssExMxPmPolicerCountingModeSet(devNum, direction, pmMode);
    }


    rc = cpssExMxPmPolicerConfigGet(devNum,&policerConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    if (direction == CPSS_DIRECTION_INGRESS_E)
    {
        policerConf.ingrCountingMode = pmMode;
    }
    else
    {
        policerConf.egrCountingMode = pmMode;
    }

    if (CPSS_EXMXPM_POLICER_COUNTING_BILLING_E == pmMode)
    {
        policerConf.sharedCounters = GT_TRUE;
        policerConf.egrCountingEntries = policerConf.ingrCountingEntries = _4K;
    }
    else
    {
        policerConf.sharedCounters = GT_FALSE;
        policerConf.egrCountingEntries = policerConf.ingrCountingEntries = _1K;
    }

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_PUMA_E)
    {
        policerConf.sharedCounters = GT_FALSE;
    }


    /* call device specific API */
    rc = cpssExMxPmPolicerConfigSet(devNum,&policerConf);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerConfigSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountingModeGet function
* @endinternal
*
* @brief   Gets the couning mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to Counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerCountingModeGet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    OUT PRV_TGF_POLICER_COUNTING_MODE_ENT *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    dxChStage;
    CPSS_DXCH_POLICER_COUNTING_MODE_ENT dxChCntrMode;

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerCountingModeGet(devNum, dxChStage, &dxChCntrMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerCountingModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChCntrMode into device specific format */
    switch (dxChCntrMode)
    {
        case CPSS_DXCH_POLICER_COUNTING_DISABLE_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_DISABLE_E;
            break;

        case CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E;
            break;

        case CPSS_DXCH_POLICER_COUNTING_POLICY_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_POLICY_E;
            break;

        case CPSS_DXCH_POLICER_COUNTING_VLAN_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_VLAN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_COUNTING_MODE_ENT    pmMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmPolicerDirectione FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerCountingModeGet(devNum, direction, &pmMode);

    /* convert command into device specific format */
    switch (pmMode)
    {
        case CPSS_EXMXPM_POLICER_COUNTING_DISABLE_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_DISABLE_E;
            break;

        case CPSS_EXMXPM_POLICER_COUNTING_BILLING_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E;
            break;

        case CPSS_EXMXPM_POLICER_COUNTING_POLICY_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_POLICY_E;
            break;

        case CPSS_EXMXPM_POLICER_COUNTING_VLAN_E:
            *modePtr = PRV_TGF_POLICER_COUNTING_VLAN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerBillingCountersModeSet function
* @endinternal
*
* @brief   Sets the Billing Counters resolution.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrMode.
*/
GT_STATUS prvTgfPolicerBillingCountersModeSet
(
    IN PRV_TGF_POLICER_BILLING_CNTR_MODE_ENT cntrMode
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT dxChCntrMode;

    /* convert cntrMode into device specific format */
    switch (cntrMode)
    {
        case PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E:
            dxChCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E;
            break;

        case PRV_TGF_POLICER_BILLING_CNTR_16_BYTES_E:
            dxChCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E;
            break;

        case PRV_TGF_POLICER_BILLING_CNTR_PACKET_E:
            dxChCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerBillingCountersModeSet(devNum, dxChCntrMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerBillingCountersModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(cntrMode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerBillingCountersModeGet function
* @endinternal
*
* @brief   Gets the Billing Counters resolution.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] cntrModePtr              - pointer to the Billing Counters resolution
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerBillingCountersModeGet
(
    IN GT_U8                                  devNum,
    OUT PRV_TGF_POLICER_BILLING_CNTR_MODE_ENT *cntrModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT dxChCntrMode;

    CPSS_NULL_PTR_CHECK_MAC(cntrModePtr);

    /* call device specific API */
    rc = cpssDxCh3PolicerBillingCountersModeGet(devNum, &dxChCntrMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerBillingCountersModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChCntrMode into device specific format */
    switch (dxChCntrMode)
    {
        case CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E:
            *cntrModePtr = PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E;
            break;

        case CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E:
            *cntrModePtr = PRV_TGF_POLICER_BILLING_CNTR_16_BYTES_E;
            break;

        case CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E:
            *cntrModePtr = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cntrModePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Sets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or packetSize.
*/
GT_STATUS prvTgfPolicerPacketSizeModeSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSize
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerPacketSizeModeSet(devNum, dxChStage, packetSize);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerPacketSizeModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerPacketSizeModeSet(devNum, direction, packetSize);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerPacketSizeModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(packetSize);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPacketSizeModeGet function
* @endinternal
*
* @brief   Gets metered Packet Size Mode that metering and billing is done
*         according to.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] packetSizePtr            - pointer to the Type of packet size
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerPacketSizeModeGet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    OUT CPSS_POLICER_PACKET_SIZE_MODE_ENT *packetSizePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(packetSizePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerPacketSizeModeGet(devNum, dxChStage, packetSizePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerPacketSizeModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerPacketSizeModeGet(devNum, direction, packetSizePtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(packetSizePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeterResolutionSet function
* @endinternal
*
* @brief   Sets metering algorithm resolution
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or resolution.
*/
GT_STATUS prvTgfPolicerMeterResolutionSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_METER_RESOLUTION_ENT resolution
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        dxChStage;
    CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT dxChResolution;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert resolution into device specific format */
    switch (resolution)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChResolution,
                                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E,
                                CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChResolution,
                                PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E,
                                CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E);

        default:
            return GT_BAD_PARAM;
    }

    prvTgfPolicerEarchMeterResolution[stage] = resolution;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerMeterResolutionSet(devNum, dxChStage, dxChResolution);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeterResolutionSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_METER_RESOLUTION_ENT     pmResolution;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert resolution into device specific format */
    switch (resolution)
    {
        PRV_TGF_SWITCH_CASE_MAC(pmResolution,
                                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E,
                                CPSS_EXMXPM_POLICER_METER_RESOLUTION_BYTES_E);

        PRV_TGF_SWITCH_CASE_MAC(pmResolution,
                                PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E,
                                CPSS_EXMXPM_POLICER_METER_RESOLUTION_PACKETS_E);

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerMeterResolutionSet(devNum, direction, pmResolution);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerMeterResolutionSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(resolution);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeterResolutionGet function
* @endinternal
*
* @brief   Gets metering algorithm resolution
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] resolutionPtr            - pointer to the Meter resolution: packet or Byte based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMeterResolutionGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    OUT PRV_TGF_POLICER_METER_RESOLUTION_ENT *resolutionPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        dxChStage;
    CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT dxChResolution;

    CPSS_NULL_PTR_CHECK_MAC(resolutionPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerMeterResolutionGet(devNum, dxChStage, &dxChResolution);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeterResolutionGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChResolution into device specific format */
    switch (dxChResolution)
    {
        case CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E:
            *resolutionPtr = PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
            break;

        case CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E:
            *resolutionPtr = PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_METER_RESOLUTION_ENT     pmResolution;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerMeterResolutionGet(devNum, direction, &pmResolution);

    /* convert dxChResolution into device specific format */
    switch (pmResolution)
    {
        case CPSS_EXMXPM_POLICER_METER_RESOLUTION_BYTES_E:
            *resolutionPtr = PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
            break;

        case CPSS_EXMXPM_POLICER_METER_RESOLUTION_PACKETS_E:
            *resolutionPtr = PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(resolutionPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerDropTypeSet function
* @endinternal
*
* @brief   Sets the Policer out-of-profile drop command type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, dropType or stage.
*/
GT_STATUS prvTgfPolicerDropTypeSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_DROP_MODE_TYPE_ENT        dropType
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerDropTypeSet(devNum, dxChStage, dropType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerDropTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(dropType);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerDropTypeGet function
* @endinternal
*
* @brief   Gets the Policer out-of-profile drop command type.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*
* @param[out] dropTypePtr              - pointer to the Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerDropTypeGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DROP_MODE_TYPE_ENT        *dropTypePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(dropTypePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerDropTypeGet(devNum, dxChStage, dropTypePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerDropTypeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(dropTypePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountingColorModeSet function
* @endinternal
*
* @brief   Sets the Policer color counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
*/
GT_STATUS prvTgfPolicerCountingColorModeSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_COLOR_COUNT_MODE_ENT mode
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        dxChStage;
    CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT dxChMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mode into device specific format */
    switch (mode)
    {
        case PRV_TGF_POLICER_COLOR_COUNT_CL_E:
            dxChMode = CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E;
            break;

        case PRV_TGF_POLICER_COLOR_COUNT_DP_E:
            dxChMode = CPSS_DXCH3_POLICER_COLOR_COUNT_DP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerCountingColorModeSet(devNum, dxChStage, dxChMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerCountingColorModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_COLOR_COUNT_MODE_ENT     pmMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert mode into device specific format */
    switch (mode)
    {
        case PRV_TGF_POLICER_COLOR_COUNT_CL_E:
            pmMode = CPSS_EXMXPM_POLICER_COLOR_COUNT_CL_E;
            break;

        case PRV_TGF_POLICER_COLOR_COUNT_DP_E:
            pmMode = CPSS_EXMXPM_POLICER_COLOR_COUNT_DP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerCountingColorModeSet(devNum, direction, pmMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerCountingColorModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountingColorModeGet function
* @endinternal
*
* @brief   Gets the Policer color counting mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to the color counting mode:
*                                      Drop Precedence or Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerCountingColorModeGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    OUT PRV_TGF_POLICER_COLOR_COUNT_MODE_ENT *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        dxChStage;
    CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT dxChCntrMode;

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerCountingColorModeGet(devNum, dxChStage, &dxChCntrMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerCountingColorModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChCntrMode into device specific format */
    switch (dxChCntrMode)
    {
        case CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E:
            *modePtr = PRV_TGF_POLICER_COLOR_COUNT_CL_E;
            break;

        case CPSS_DXCH3_POLICER_COLOR_COUNT_DP_E:
            *modePtr = PRV_TGF_POLICER_COLOR_COUNT_DP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_COLOR_COUNT_MODE_ENT     pmMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerCountingColorModeGet(devNum, direction, &pmMode);

    /* convert pmMode into TGF format */
    switch (pmMode)
    {
        case CPSS_EXMXPM_POLICER_COLOR_COUNT_CL_E:
            *modePtr = PRV_TGF_POLICER_COLOR_COUNT_CL_E;
            break;

        case CPSS_EXMXPM_POLICER_COLOR_COUNT_DP_E:
            *modePtr = PRV_TGF_POLICER_COLOR_COUNT_DP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerManagementCntrsResolutionSet function
* @endinternal
*
* @brief   Sets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or
*                                       Management Counters Set or cntrResolution.
*/
GT_STATUS prvTgfPolicerManagementCntrsResolutionSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT        cntrSet,
    IN PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_ENT cntrResolution
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT           dxChStage;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT        dxChMngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT dxChCntrResolution;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert cntrSet into device specific format */
    switch (cntrSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert cntrResolution into device specific format */
    switch (cntrResolution)
    {
        case PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_1B_E:
            dxChCntrResolution = CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_16B_E:
            dxChCntrResolution = CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_16B_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerManagementCntrsResolutionSet(devNum, dxChStage, dxChMngCntrSet, dxChCntrResolution);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerManagementCntrsResolutionSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(cntrSet);
    TGF_PARAM_NOT_USED(cntrResolution);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerManagementCntrsResolutionGet function
* @endinternal
*
* @brief   Gets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] cntrSet                  - Management Counters Set [0..2].
*
* @param[out] cntrResolutionPtr        - pointer to the Management Counters
*                                      resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or Mng Counters Set.
*/
GT_STATUS prvTgfPolicerManagementCntrsResolutionGet
(
    IN  GT_U8                                    devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT         cntrSet,
    OUT PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_ENT  *cntrResolutionPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT           dxChStage;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT        dxChMngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT dxChCntrResolution;

    CPSS_NULL_PTR_CHECK_MAC(cntrResolutionPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert cntrSet into device specific format */
    switch (cntrSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerManagementCntrsResolutionGet(devNum, dxChStage, dxChMngCntrSet, &dxChCntrResolution);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerManagementCntrsResolutionGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChMngCntrSet into device specific format */
    switch (dxChCntrResolution)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E:
            *cntrResolutionPtr = PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_1B_E;
            break;

        case CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_16B_E:
            *cntrResolutionPtr = PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_16B_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(cntrSet);
    TGF_PARAM_NOT_USED(cntrResolutionPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Sets size mode for metering and counting of tunnel terminated packets.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or ttPacketSizeMode.
*/
GT_STATUS prvTgfPolicerPacketSizeModeForTunnelTermSet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    IN  PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT ttPacketSizeMode
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT           dxChStage;
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT dxChTtPacketSizeMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mode into device specific format */
    switch (ttPacketSizeMode)
    {
        case PRV_TGF_POLICER_TT_PACKET_SIZE_REGULAR_E:
            dxChTtPacketSizeMode = CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E;
            break;

        case PRV_TGF_POLICER_TT_PACKET_SIZE_PASSENGER_E:
            dxChTtPacketSizeMode = CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerPacketSizeModeForTunnelTermSet(devNum, dxChStage, dxChTtPacketSizeMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerPacketSizeModeForTunnelTermSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(ttPacketSizeMode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPacketSizeModeForTunnelTermGet function
* @endinternal
*
* @brief   Gets size mode for metering and counting of tunnel terminated packets.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] ttPacketSizeModePtr      - pointer to the Tunnel Termination
*                                      Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerPacketSizeModeForTunnelTermGet
(
    IN  GT_U8                                    devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    OUT PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT  *ttPacketSizeModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT           dxChStage;
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT dxChTtPacketSizeMode;

    CPSS_NULL_PTR_CHECK_MAC(ttPacketSizeModePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerPacketSizeModeForTunnelTermGet(devNum, dxChStage, &dxChTtPacketSizeMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerPacketSizeModeForTunnelTermGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChTtPacketSizeMode into device specific format */
    switch (dxChTtPacketSizeMode)
    {
        case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E:
            *ttPacketSizeModePtr = PRV_TGF_POLICER_TT_PACKET_SIZE_REGULAR_E;
            break;

        case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E:
            *ttPacketSizeModePtr = PRV_TGF_POLICER_TT_PACKET_SIZE_PASSENGER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(ttPacketSizeModePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringAutoRefreshScanEnableSet function
* @endinternal
*
* @brief   Enables or disables the metering Auto Refresh Scan mechanism.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshScanEnableSet
(
    IN GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringAutoRefreshScanEnableGet function
* @endinternal
*
* @brief   Gets metering Auto Refresh Scan mechanism status (Enabled/Disabled).
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer on Auto Refresh Scan mechanism status
*                                      (Enable/Disable) :
*                                      GT_TRUE  - Auto Refresh scan mechanism is enabled.
*                                      GT_FALSE - Auto Refresh scan mechanism is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshScanEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* call device specific API */
    rc = cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringAutoRefreshRangeSet function
* @endinternal
*
* @brief   Sets Metering Refresh Scan address range (Start and Stop addresses).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or illegal values
*                                       of startAddress and stopAddress.
* @retval GT_OUT_OF_RANGE          - on out of range of startAddress
*                                       or stopAddress.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshRangeSet
(
    IN GT_U32 startAddress,
    IN GT_U32 stopAddress
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerMeteringAutoRefreshRangeSet(devNum, startAddress, stopAddress);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerCountingColorModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(startAddress);
    TGF_PARAM_NOT_USED(stopAddress);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringAutoRefreshRangeGet function
* @endinternal
*
* @brief   Gets Metering Refresh Scan address range (Start and Stop addresses).
*
* @param[in] devNum                   - physical device number.
*
* @param[out] startAddressPtr          - pointer to the beginning address of Refresh
*                                      Scan address range.
* @param[out] stopAddressPtr           - pointer to the end address of Refresh Scan
*                                      address range.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshRangeGet
(
    IN GT_U8   devNum,
    OUT GT_U32 *startAddressPtr,
    OUT GT_U32 *stopAddressPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(startAddressPtr);
    CPSS_NULL_PTR_CHECK_MAC(stopAddressPtr);

    /* call device specific API */
    rc = cpssDxCh3PolicerMeteringAutoRefreshRangeGet(devNum, startAddressPtr, stopAddressPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeteringAutoRefreshRangeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(startAddressPtr);
    TGF_PARAM_NOT_USED(stopAddressPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringAutoRefreshIntervalSet function
* @endinternal
*
* @brief   Sets the time interval between two refresh access to metering table.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on out of range of interval value.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshIntervalSet
(
    IN GT_U32 interval
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerMeteringAutoRefreshIntervalSet(devNum, interval);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeteringAutoRefreshIntervalSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(interval);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringAutoRefreshIntervalGet function
* @endinternal
*
* @brief   Gets the time interval between two refresh access to metering table.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] intervalPtr              - pointer to the time interval between refresh of two
*                                      entries in micro seconds. Upon 270 Mhz core clock
*                                      value of DxCh3, the range [0..15907286 microS].
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshIntervalGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *intervalPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(intervalPtr);

    /* call device specific API */
    rc = cpssDxCh3PolicerMeteringAutoRefreshIntervalGet(devNum, intervalPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMeteringAutoRefreshIntervalGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(intervalPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
*/
GT_STATUS prvTgfPolicerMeteringEntryRefresh
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         entryIndex
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxCh3PolicerMeteringEntryRefresh(devNum, dxChStage, entryIndex);
        }
        else
        {
            rc = cpssDxChPolicerPortGroupMeteringEntryRefresh(devNum, currPortGroupsBmp, dxChStage, entryIndex);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupMeteringEntryRefresh FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPortMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables a port metering trigger for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] enable                   - Enable/Disable per-port metering for packets arriving
*                                      on this port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
*/
GT_STATUS prvTgfPolicerPortMeteringEnableSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerPortMeteringEnableSet(devNum, dxChStage, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerPortMeteringEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerPortMeteringEnableSet(devNum, direction, portNum, enable);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPortMeteringEnableGet function
* @endinternal
*
* @brief   Gets port status (Enable/Disable) of metering for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
*
* @param[out] enablePtr                - pointer on per-port metering status (Enable/Disable)
*                                      for packets arriving on specified port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
*/
GT_STATUS prvTgfPolicerPortMeteringEnableGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                          portNum,
    OUT GT_BOOL                        *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerPortMeteringEnableGet(devNum, dxChStage, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerPacketSizeModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerPortMeteringEnableGet(devNum, direction, portNum, enablePtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfPolicerManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set[0..2].
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] mngCntrPtr               - pointer to the Management Counters Entry must be set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for DxChXcat and above devices.
*
*/
GT_STATUS prvTgfPolicerManagementCountersSet
(
    IN GT_U8                              devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT   mngCntrSet,
    IN PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT  mngCntrType,
    IN PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC *mngCntrPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT      dxChStage;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT   dxChMngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT  dxChMngCntrType;
    CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC dxChMngCntr;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mngCntrSet into device specific format */
    switch (mngCntrSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCntrType into device specific format */
    switch (mngCntrType)
    {
        case PRV_TGF_POLICER_MNG_CNTR_GREEN_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_YELLOW_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_RED_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_RED_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DROP_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCntrPtr into device specific format */
    dxChMngCntr.duMngCntr.l[0] = mngCntrPtr->duMngCntr.l[0];
    dxChMngCntr.duMngCntr.l[1] = mngCntrPtr->duMngCntr.l[1];
    dxChMngCntr.packetMngCntr  = mngCntrPtr->packetMngCntr;

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxCh3PolicerManagementCountersSet(devNum, dxChStage,
                dxChMngCntrSet, dxChMngCntrType, &dxChMngCntr);
    }
    else
    {
        rc = cpssDxChPolicerPortGroupManagementCountersSet(devNum, currPortGroupsBmp, dxChStage,
                dxChMngCntrSet, dxChMngCntrType, &dxChMngCntr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupManagementCountersSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_MNG_CNTR_SET_ENT     pmMngCntrSet;
    CPSS_EXMXPM_POLICER_MNG_CNTR_TYPE_ENT    pmMngCntrType;
    CPSS_EXMXPM_POLICER_MNG_CNTR_ENTRY_STC   pmMmngCntr;


    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert mngCntrSet into device specific format */
    switch (mngCntrSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCntrType into device specific format */
    switch (mngCntrType)
    {
        case PRV_TGF_POLICER_MNG_CNTR_GREEN_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_GREEN_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_YELLOW_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_YELLOW_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_RED_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_RED_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DROP_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_DROP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCntrPtr into device specific format */
    pmMmngCntr.duMngCntr.l[0] = mngCntrPtr->duMngCntr.l[0];
    pmMmngCntr.duMngCntr.l[1] = mngCntrPtr->duMngCntr.l[1];
    pmMmngCntr.packetMngCntr  = mngCntrPtr->packetMngCntr;

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssExMxPmPolicerManagementCountersSet(devNum, direction,
                pmMngCntrSet, pmMngCntrType, &pmMmngCntr);
    }
    else
    {
        rc = cpssExMxPmPolicerPortGroupManagementCountersSet(devNum, currPortGroupsBmp, direction,
                pmMngCntrSet, pmMngCntrType, &pmMmngCntr);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mngCntrSet);
    TGF_PARAM_NOT_USED(mngCntrType);
    TGF_PARAM_NOT_USED(mngCntrPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set[0..2].
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* @param[out] mngCntrPtr               - pointer to the requested Management Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for DxChXcat and above devices.
*
*/
GT_STATUS prvTgfPolicerManagementCountersGet
(
    IN GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT   mngCntrSet,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT  mngCntrType,
    OUT PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC *mngCntrPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        dxChStage;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     dxChMngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    dxChMngCntrType;
    CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   dxChMngCntr;

    CPSS_NULL_PTR_CHECK_MAC(mngCntrPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mngCntrSet into device specific format */
    switch (mngCntrSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            dxChMngCntrSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCntrType into device specific format */
    switch (mngCntrType)
    {
        case PRV_TGF_POLICER_MNG_CNTR_GREEN_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_YELLOW_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_RED_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_RED_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DROP_E:
            dxChMngCntrType = CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxCh3PolicerManagementCountersGet(devNum, dxChStage,
                dxChMngCntrSet, dxChMngCntrType, &dxChMngCntr);
    }
    else
    {
        rc = cpssDxChPolicerPortGroupManagementCountersGet(devNum, currPortGroupsBmp, dxChStage,
                dxChMngCntrSet, dxChMngCntrType, &dxChMngCntr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupManagementCountersGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mngCntrPtr from device specific format */
    mngCntrPtr->duMngCntr.l[0] = dxChMngCntr.duMngCntr.l[0];
    mngCntrPtr->duMngCntr.l[1] = dxChMngCntr.duMngCntr.l[1];
    mngCntrPtr->packetMngCntr  = dxChMngCntr.packetMngCntr;

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_MNG_CNTR_SET_ENT     pmMngCntrSet;
    CPSS_EXMXPM_POLICER_MNG_CNTR_TYPE_ENT    pmMngCntrType;
    CPSS_EXMXPM_POLICER_MNG_CNTR_ENTRY_STC   pmMmngCntr;


    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert mngCntrSet into device specific format */
    switch (mngCntrSet)
    {
        case PRV_TGF_POLICER_MNG_CNTR_SET0_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET0_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET1_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET1_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_SET2_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_SET2_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DISABLED_E:
            pmMngCntrSet = CPSS_EXMXPM_POLICER_MNG_CNTR_DISABLED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mngCntrType into device specific format */
    switch (mngCntrType)
    {
        case PRV_TGF_POLICER_MNG_CNTR_GREEN_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_GREEN_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_YELLOW_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_YELLOW_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_RED_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_RED_E;
            break;

        case PRV_TGF_POLICER_MNG_CNTR_DROP_E:
            pmMngCntrType = CPSS_EXMXPM_POLICER_MNG_CNTR_DROP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssExMxPmPolicerManagementCountersGet(devNum, direction,
                pmMngCntrSet, pmMngCntrType, &pmMmngCntr);
    }
    else
    {
        rc = cpssExMxPmPolicerPortGroupManagementCountersGet(devNum, currPortGroupsBmp, direction,
                pmMngCntrSet, pmMngCntrType, &pmMmngCntr);
    }

    /* convert mngCntrPtr from device specific format */
    mngCntrPtr->duMngCntr.l[0] = pmMmngCntr.duMngCntr.l[0];
    mngCntrPtr->duMngCntr.l[1] = pmMmngCntr.duMngCntr.l[1];
    mngCntrPtr->packetMngCntr  = pmMmngCntr.packetMngCntr;

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mngCntrSet);
    TGF_PARAM_NOT_USED(mngCntrType);
    TGF_PARAM_NOT_USED(mngCntrPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEntryUseEarchEnable function
* @endinternal
*
* @brief   Setting flag indicating that the in the metering entry all
*         eArch related field must be explicitly configured.
* @param[in] enable                   - indication if eArch metering format is to be configured
*                                      explicitly.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfPolicerEntryUseEarchEnable
(
    IN  GT_BOOL enable
)
{
    prvTgfPolicerEntryUseEarch = enable;

    return GT_OK;
}

/**
* @internal prvTgfPolicerEntrySet function
* @endinternal
*
* @brief   Set Policer Entry configuration
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - policer entry index
* @param[in] entryPtr                 - (pointer to) policer entry
*
* @param[out] tbParamsPtr              - (pointer to) actual policer token bucket parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, entryIndex or entry parameter
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerEntrySet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN  GT_U32                               entryIndex,
    IN  PRV_TGF_POLICER_ENTRY_STC           *entryPtr,
    OUT PRV_TGF_POLICER_METER_TB_PARAMS_UNT *tbParamsPtr
)
{
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_POLICER_ENTRY_STC             dxChEntry;
    CPSS_DXCH_POLICER_TB_PARAMS_STC         dxChTbParams;
    CPSS_PP_FAMILY_TYPE_ENT                 devFamily;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        dxChStage;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC   dxCh3Entry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  dxCh3TbParams;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_POLICER_METERING_ENTRY_STC   exMxPmEntry;
    CPSS_EXMXPM_POLICER_METER_TB_PARAMS_UNT  exMxPmTbParams;
    CPSS_DIRECTION_ENT direction;
#endif /* EXMXPM_FAMILY */

    if(GT_FALSE == prvTgfPolicerEntryUseEarch)
    {
        entryPtr->byteOrPacketCountingMode =
                                prvTgfPolicerEarchMeterResolution[stage];

        rc = prvTgfPolicerPacketSizeModeGet(prvTgfDevNum,
                                            stage,
                                            &entryPtr->packetSizeMode);

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerPacketSizeModeGet FAILED, rc = [%d]", rc);

            return rc;
        }


        if( PRV_TGF_POLICER_STAGE_EGRESS_E == stage )
        {
            if( entryPtr->modifyExp ==
                                CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E )
            {
                entryPtr->modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            }

            if( entryPtr->modifyTc ==
                                CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E )
            {
                entryPtr->modifyTc = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            }
        }
    }

#ifdef CHX_FAMILY
    CPSS_NULL_PTR_CHECK_MAC(tbParamsPtr);

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    if (devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        TGF_PARAM_NOT_USED(stage);
        TGF_PARAM_NOT_USED(dxChStage);
        TGF_PARAM_NOT_USED(dxCh3Entry);
        TGF_PARAM_NOT_USED(dxCh3TbParams);

        /* convert policer entry into device specific format */
        rc = prvTgfConvertGenericToDxChPolicerEntry(entryPtr, &dxChEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerEntry FAILED, rc = [%d]", rc);

            return rc;
        }

        /* go over all active devices */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /* call device specific API */
            rc = cpssDxChPolicerEntrySet(devNum, entryIndex, &dxChEntry, &dxChTbParams);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEntrySet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }

        /* convert tbParams from device specific format */
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->srTcmParams),
                                       &(dxChEntry.tbParams), cir);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->srTcmParams),
                                       &(dxChEntry.tbParams), cbs);

        return rc1;
    }
    else
    {
        TGF_PARAM_NOT_USED(dxChEntry);
        TGF_PARAM_NOT_USED(dxChTbParams);

        /* clear dxCh3TbParams */
        cpssOsMemSet(&dxCh3TbParams, 0, sizeof(dxCh3TbParams));

        /* convert stage into device specific format */
        rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

            return rc;
        }

        /* convert policer entry into device specific format */
        rc = prvTgfConvertGenericToDxCh3PolicerMeteringEntry(entryPtr, &dxCh3Entry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerEntry FAILED, rc = [%d]", rc);

            return rc;
        }

        /* go over all active devices */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            /* call device specific API */
            if(usePortGroupsBmp == GT_FALSE)
            {
                rc = cpssDxCh3PolicerMeteringEntrySet(devNum, dxChStage, entryIndex, &dxCh3Entry, &dxCh3TbParams);
            }
            else
            {
                rc = cpssDxChPolicerPortGroupMeteringEntrySet(devNum, currPortGroupsBmp, dxChStage, entryIndex, &dxCh3Entry, &dxCh3TbParams);
            }

            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupMeteringEntrySet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }

        /* convert tbParams from device specific format */
        if (CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E == dxCh3Entry.meterMode)
        {
            tbParamsPtr->srTcmParams.cir = dxCh3TbParams.srTcmParams.cir;
            tbParamsPtr->srTcmParams.cbs = dxCh3TbParams.srTcmParams.cbs;
            tbParamsPtr->srTcmParams.ebs = dxCh3TbParams.srTcmParams.ebs;
        }
        else
        {
            tbParamsPtr->trTcmParams.cir = dxCh3TbParams.trTcmParams.cir;
            tbParamsPtr->trTcmParams.cbs = dxCh3TbParams.trTcmParams.cbs;
            tbParamsPtr->trTcmParams.pir = dxCh3TbParams.trTcmParams.pir;
            tbParamsPtr->trTcmParams.pbs = dxCh3TbParams.trTcmParams.pbs;
        }

        return rc1;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_NULL_PTR_CHECK_MAC(tbParamsPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* clear dxCh3TbParams */
    cpssOsMemSet(&exMxPmTbParams, 0, sizeof(exMxPmTbParams));

    /* convert policer entry into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerEntry(entryPtr, &exMxPmEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmPolicerEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerMeteringEntrySet(devNum, direction, entryIndex,
                                               &exMxPmEntry, &exMxPmTbParams);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerMeteringEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    /* convert tbParams from device specific format */
    if (CPSS_EXMXPM_POLICER_METER_MODE_SR_TCM_E == exMxPmEntry.meterMode)
    {
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->srTcmParams),
                                       &(exMxPmTbParams.srTcmParams), cir);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->srTcmParams),
                                       &(exMxPmTbParams.srTcmParams), cbs);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->srTcmParams),
                                       &(exMxPmTbParams.srTcmParams), ebs);
    }
    else
    {
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->trTcmParams),
                                       &(exMxPmTbParams.trTcmParams), cir);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->trTcmParams),
                                       &(exMxPmTbParams.trTcmParams), cbs);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->trTcmParams),
                                       &(exMxPmTbParams.trTcmParams), pir);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(tbParamsPtr->trTcmParams),
                                       &(exMxPmTbParams.trTcmParams), pbs);
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);
    TGF_PARAM_NOT_USED(tbParamsPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPolicerEntryGet function
* @endinternal
*
* @brief   Get Policer Entry parameters
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - policer entry index
*
* @param[out] entryPtr                 - (pointer to) policer entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - on max number of retries checking if PP ready
*/
GT_STATUS prvTgfPolicerEntryGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         entryIndex,
    OUT PRV_TGF_POLICER_ENTRY_STC     *entryPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_POLICER_ENTRY_STC           dxChEntry;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC dxCh3Entry;
    CPSS_PP_FAMILY_TYPE_ENT               devFamily;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT      dxChStage;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_POLICER_METERING_ENTRY_STC   exMxPmEntry;
    CPSS_DIRECTION_ENT direction;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* get device family */
    rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);

        return rc;
    }

    if (devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        TGF_PARAM_NOT_USED(stage);
        TGF_PARAM_NOT_USED(dxChStage);
        TGF_PARAM_NOT_USED(dxCh3Entry);

        /* call device specific API */
        rc = cpssDxChPolicerEntryGet(devNum, entryIndex, &dxChEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEntryGet FAILED, rc = [%d]", rc);

            return rc;
        }

        /* convert policer entry into device specific format */
        return prvTgfConvertDxChToGenericPolicerEntry(&dxChEntry, entryPtr);
    }
    else
    {
        TGF_PARAM_NOT_USED(dxChEntry);

        /* convert stage into device specific format */
        rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

            return rc;
        }

        /* reset entry */
        cpssOsMemSet(&dxCh3Entry, 0, sizeof(dxCh3Entry));

        /* call device specific API */
        if(usePortGroupsBmp == GT_FALSE)
        {
            rc = cpssDxCh3PolicerMeteringEntryGet(devNum, dxChStage, entryIndex, &dxCh3Entry);
        }
        else
        {
            rc = cpssDxChPolicerPortGroupMeteringEntryGet(devNum, currPortGroupsBmp, dxChStage, entryIndex, &dxCh3Entry);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupMeteringEntryGet FAILED, rc = [%d]", rc);

            return rc;
        }

        /* convert policer entry into device specific format */
        return prvTgfConvertDxCh3ToGenericPolicerEntry(&dxCh3Entry, entryPtr);
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerMeteringEntryGet(devNum, direction,
                                           entryIndex, &exMxPmEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert policer entry into device specific format */
    return prvTgfConvertExMxPmToGenericPolicerEntry(&exMxPmEntry, entryPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculates Token Bucket parameters in the Application format without
*         HW update.
* @param[in] devNum                   - device number
* @param[in] tbInParamsPtr            - pointer to Token bucket input parameters.
* @param[in] meterMode                - Meter mode (SrTCM or TrTCM).
*
* @param[out] tbOutParamsPtr           - pointer to Token bucket output paramters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong devNum or meterMode.
*/
GT_STATUS prvTgfPolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_METER_MODE_ENT       meterMode,
    IN  PRV_TGF_POLICER_METER_TB_PARAMS_UNT  *tbInParamsPtr,
    OUT PRV_TGF_POLICER_METER_TB_PARAMS_UNT  *tbOutParamsPtr
);

/**
* @internal prvTgfPolicerBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
*/
GT_STATUS prvTgfPolicerBillingEntrySet
(
    IN GT_U8                              devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                             entryIndex,
    IN PRV_TGF_POLICER_BILLING_ENTRY_STC *billingCntrPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT      dxChStage;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC  dxChBillingCntrSet;

    /* clear dxChBillingCntrSet */
    cpssOsMemSet(&dxChBillingCntrSet, 0, sizeof(dxChBillingCntrSet));

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert billingCntrPtr into device specific format */
    dxChBillingCntrSet.greenCntr.l[0]  = billingCntrPtr->greenCntr.l[0];
    dxChBillingCntrSet.greenCntr.l[1]  = billingCntrPtr->greenCntr.l[1];
    dxChBillingCntrSet.yellowCntr.l[0] = billingCntrPtr->yellowCntr.l[0];
    dxChBillingCntrSet.yellowCntr.l[1] = billingCntrPtr->yellowCntr.l[1];
    dxChBillingCntrSet.redCntr.l[0]    = billingCntrPtr->redCntr.l[0];
    dxChBillingCntrSet.redCntr.l[1]    = billingCntrPtr->redCntr.l[1];
    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        switch(billingCntrPtr->lmCntrCaptureMode)
        {
            case PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_COPY_E:
                dxChBillingCntrSet.lmCntrCaptureMode =
                    CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_COPY_E;
                break;
            case PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E:
                dxChBillingCntrSet.lmCntrCaptureMode =
                    CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_INSERT_E;
                break;
            default:
                return GT_BAD_PARAM;
        }

        dxChBillingCntrSet.packetSizeMode = billingCntrPtr->packetSizeMode;
    }

    /* convert billingCntrMode into device specific format */
    switch (billingCntrPtr->billingCntrMode)
    {
        case PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E:
            dxChBillingCntrSet.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E;
            break;

        case PRV_TGF_POLICER_BILLING_CNTR_16_BYTES_E:
            dxChBillingCntrSet.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E;
            break;

        case PRV_TGF_POLICER_BILLING_CNTR_PACKET_E:
            dxChBillingCntrSet.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxCh3PolicerBillingEntrySet(devNum, dxChStage, entryIndex, &dxChBillingCntrSet);
    }
    else
    {
        rc = cpssDxChPolicerPortGroupBillingEntrySet(devNum, currPortGroupsBmp, dxChStage,
                entryIndex, &dxChBillingCntrSet);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupBillingEntrySet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_BILLING_ENTRY_STC   pmBillingCntr;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* clear dxChBillingCntrSet */
    cpssOsMemSet(&pmBillingCntr, 0, sizeof(pmBillingCntr));

    /* convert billingCntrPtr into device specific format */
    pmBillingCntr.greenCntr.l[0]  = billingCntrPtr->greenCntr.l[0];
    pmBillingCntr.greenCntr.l[1]  = billingCntrPtr->greenCntr.l[1];
    pmBillingCntr.yellowCntr.l[0] = billingCntrPtr->yellowCntr.l[0];
    pmBillingCntr.yellowCntr.l[1] = billingCntrPtr->yellowCntr.l[1];
    pmBillingCntr.redCntr.l[0]    = billingCntrPtr->redCntr.l[0];
    pmBillingCntr.redCntr.l[1]    = billingCntrPtr->redCntr.l[1];

    /* convert billingCntrMode into device specific format */
    switch (billingCntrPtr->billingCntrMode)
    {
        case PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E:
            pmBillingCntr.cntrMode = CPSS_EXMXPM_POLICER_BILLING_CNTR_1_BYTE_E;
            break;

        case PRV_TGF_POLICER_BILLING_CNTR_16_BYTES_E:
            pmBillingCntr.cntrMode = CPSS_EXMXPM_POLICER_BILLING_CNTR_16_BYTES_E;
            break;

        case PRV_TGF_POLICER_BILLING_CNTR_PACKET_E:
            pmBillingCntr.cntrMode = CPSS_EXMXPM_POLICER_BILLING_CNTR_PACKET_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssExMxPmPolicerBillingEntrySet(devNum, direction, entryIndex, &pmBillingCntr);
    }
    else
    {
        rc = cpssExMxPmPolicerPortGroupBillingEntrySet(devNum, currPortGroupsBmp, direction,
                entryIndex, &pmBillingCntr);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(billingCntrPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
* @param[in] reset                    -  flag:
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read counters operation only.
*
* @param[out] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
*/
GT_STATUS prvTgfPolicerBillingEntryGet
(
    IN  GT_U8                              devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                             entryIndex,
    IN  GT_BOOL                            reset,
    OUT PRV_TGF_POLICER_BILLING_ENTRY_STC *billingCntrPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT     dxChStage;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC dxChBillingCntrSet;

    CPSS_NULL_PTR_CHECK_MAC(billingCntrPtr);

    /* clear billingCntrPtr */
    cpssOsMemSet(billingCntrPtr, 0, sizeof(*billingCntrPtr));

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxCh3PolicerBillingEntryGet(devNum, dxChStage,
                entryIndex, reset, &dxChBillingCntrSet);
    }
    else
    {
        rc = cpssDxChPolicerPortGroupBillingEntryGet(devNum, currPortGroupsBmp, dxChStage,
                entryIndex, reset, &dxChBillingCntrSet);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupBillingEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert billingCntrPtr from device specific format */
    billingCntrPtr->greenCntr.l[0]  = dxChBillingCntrSet.greenCntr.l[0];
    billingCntrPtr->greenCntr.l[1]  = dxChBillingCntrSet.greenCntr.l[1];
    billingCntrPtr->yellowCntr.l[0] = dxChBillingCntrSet.yellowCntr.l[0];
    billingCntrPtr->yellowCntr.l[1] = dxChBillingCntrSet.yellowCntr.l[1];
    billingCntrPtr->redCntr.l[0]    = dxChBillingCntrSet.redCntr.l[0];
    billingCntrPtr->redCntr.l[1]    = dxChBillingCntrSet.redCntr.l[1];

    /* convert billingCntrMode from device specific format */
    switch (dxChBillingCntrSet.billingCntrMode)
    {
        case CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E:
            billingCntrPtr->billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E;
            break;

        case CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E:
            billingCntrPtr->billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_16_BYTES_E;
            break;

        case CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E:
            billingCntrPtr->billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
    {
        switch(dxChBillingCntrSet.lmCntrCaptureMode)
        {
            case CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_COPY_E:
                billingCntrPtr->lmCntrCaptureMode =
                    PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_COPY_E;
                break;
            case CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_INSERT_E:
                billingCntrPtr->lmCntrCaptureMode =
                    PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E;
                break;
            default:
                return GT_BAD_PARAM;
        }

        billingCntrPtr->greenCntrSnapshot.l[0] =
            dxChBillingCntrSet.greenCntrSnapshot.l[0];
        billingCntrPtr->greenCntrSnapshot.l[1] =
            dxChBillingCntrSet.greenCntrSnapshot.l[1];
        billingCntrPtr->greenCntrSnapshotValid =
            dxChBillingCntrSet.greenCntrSnapshotValid;

        billingCntrPtr->packetSizeMode =
            dxChBillingCntrSet.packetSizeMode;

    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_BILLING_ENTRY_STC   pmBillingCntr;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* clear billingCntrPtr */
    cpssOsMemSet(billingCntrPtr, 0, sizeof(*billingCntrPtr));

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssExMxPmPolicerBillingEntryGet(devNum, direction, entryIndex, reset, &pmBillingCntr);
    }
    else
    {
        rc = cpssExMxPmPolicerPortGroupBillingEntryGet(devNum, currPortGroupsBmp, direction,
                entryIndex, reset, &pmBillingCntr);
    }

    /* convert billingCntrPtr from device specific format */
    billingCntrPtr->greenCntr.l[0]  = pmBillingCntr.greenCntr.l[0];
    billingCntrPtr->greenCntr.l[1]  = pmBillingCntr.greenCntr.l[1];
    billingCntrPtr->yellowCntr.l[0] = pmBillingCntr.yellowCntr.l[0];
    billingCntrPtr->yellowCntr.l[1] = pmBillingCntr.yellowCntr.l[1];
    billingCntrPtr->redCntr.l[0]    = pmBillingCntr.redCntr.l[0];
    billingCntrPtr->redCntr.l[1]    = pmBillingCntr.redCntr.l[1];

    /* convert billingCntrMode from device specific format */
    switch (pmBillingCntr.cntrMode)
    {
        case CPSS_EXMXPM_POLICER_BILLING_CNTR_1_BYTE_E:
            billingCntrPtr->billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E;
            break;

        case CPSS_EXMXPM_POLICER_BILLING_CNTR_16_BYTES_E:
            billingCntrPtr->billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_16_BYTES_E;
            break;

        case CPSS_EXMXPM_POLICER_BILLING_CNTR_PACKET_E:
            billingCntrPtr->billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(reset);
    TGF_PARAM_NOT_USED(billingCntrPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEntryInvalidate function
* @endinternal
*
* @brief   Invalidate Policer Entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex
*/
GT_STATUS prvTgfPolicerEntryInvalidate
(
    IN  GT_U32                        entryIndex
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerEntryInvalidate(devNum, entryIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEntryInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(entryIndex);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(entryIndex);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPolicerQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colors.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 Qos Remarking Entry contains three indexes [0..1023] of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of Qos Remarking Entry will be set.
* @param[in] greenQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to In-Profile packets with
*                                      the Green color. (APPLICABLE RANGES: 0..1023).
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[in] yellowQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Yellow color. (APPLICABLE RANGES: 0..127).
*                                      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 - (APPLICABLE RANGES: 0..1023).
* @param[in] redQosTableRemarkIndex   - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Red color. (APPLICABLE RANGES: 0..127).
*                                      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2 - (APPLICABLE RANGES: 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or qosProfileIndex.
* @retval GT_OUT_OF_RANGE          - on greenQosTableRemarkIndex,
*                                       yellowQosTableRemarkIndex and
*                                       redQosTableRemarkIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
GT_STATUS prvTgfPolicerQosRemarkingEntrySet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                         qosProfileIndex,
    IN GT_U32                         greenQosTableRemarkIndex,
    IN GT_U32                         yellowQosTableRemarkIndex,
    IN GT_U32                         redQosTableRemarkIndex
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerQosRemarkingEntrySet(
            devNum, dxChStage, qosProfileIndex,
            greenQosTableRemarkIndex, yellowQosTableRemarkIndex, redQosTableRemarkIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerQosRemarkingEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(qosProfileIndex);
    TGF_PARAM_NOT_USED(greenQosTableRemarkIndex);
    TGF_PARAM_NOT_USED(yellowQosTableRemarkIndex);
    TGF_PARAM_NOT_USED(redQosTableRemarkIndex);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colours.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 Qos Remarking Entry contains three indexes [0..1023] of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of requested Qos Remarking Entry.
*
* @param[out] greenQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      In-Profile packets with the
*                                      Green color.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[out] yellowQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the
*                                      Yellow color.
* @param[out] redQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the Red
*                                      color.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or qosProfileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
GT_STATUS prvTgfPolicerQosRemarkingEntryGet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN GT_U32                               qosProfileIndex,
    OUT GT_U32                              *greenQosTableRemarkIndexPtr,
    OUT GT_U32                              *yellowQosTableRemarkIndexPtr,
    OUT GT_U32                              *redQosTableRemarkIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(greenQosTableRemarkIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(yellowQosTableRemarkIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(redQosTableRemarkIndexPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerQosRemarkingEntryGet(
        devNum, dxChStage, qosProfileIndex,
        greenQosTableRemarkIndexPtr, yellowQosTableRemarkIndexPtr, redQosTableRemarkIndexPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerQosRemarkingEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(qosProfileIndex);
    TGF_PARAM_NOT_USED(greenQosTableRemarkIndexPtr);
    TGF_PARAM_NOT_USED(yellowQosTableRemarkIndexPtr);
    TGF_PARAM_NOT_USED(redQosTableRemarkIndexPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEgressQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Egress Policer Re-Marking table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
*/
GT_STATUS prvTgfPolicerEgressQosRemarkingEntrySet
(
    IN PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT remarkTableType,
    IN GT_U32                                remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                     confLevel,
    IN PRV_TGF_POLICER_QOS_PARAM_STC         *qosParamPtr
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT dxChRemarkTableType;
    CPSS_DXCH_POLICER_QOS_PARAM_STC         dxChQosParam;

    /* convert remarkTableType into device specific format */
    switch (remarkTableType)
    {
        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E:
            dxChRemarkTableType = CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_TC_UP_E;
            break;

        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_DSCP_E:
            dxChRemarkTableType = CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_DSCP_E;
            break;

        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_EXP_E:
            dxChRemarkTableType = CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_EXP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert dxChQosParam into device specific format */
    dxChQosParam.up   = qosParamPtr->up;
    dxChQosParam.dscp = qosParamPtr->dscp;
    dxChQosParam.exp  = qosParamPtr->exp;
    dxChQosParam.dp   = qosParamPtr->dp;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerEgressQosRemarkingEntrySet(devNum, dxChRemarkTableType,
                remarkParamValue, confLevel, &dxChQosParam);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerCountingColorModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;

    /* call device specific API */
    rc = prvTgfPolicerQosParamsRemarkEntrySet(PRV_TGF_POLICER_STAGE_EGRESS_E, 0,
                                              remarkTableType, remarkParamValue,
                                              confLevel, qosParamPtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(remarkTableType);
    TGF_PARAM_NOT_USED(remarkParamValue);
    TGF_PARAM_NOT_USED(confLevel);
    TGF_PARAM_NOT_USED(qosParamPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEgressQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Egress Policer Re-Marking table Entry.
*
* @param[in] devNum                   - device number.
* @param[in] remarkTableType          - Remark table type: DSCP, EXP or TC/UP.
* @param[in] remarkParamValue         - QoS parameter value.
* @param[in] confLevel                - Conformance Level: Green, Yellow, Red.
*
* @param[out] qosParamPtr              - pointer to the requested Re-Marking Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS prvTgfPolicerEgressQosRemarkingEntryGet
(
    IN  GT_U8                                 devNum,
    IN  PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT remarkTableType,
    IN  GT_U32                                remarkParamValue,
    IN  CPSS_DP_LEVEL_ENT                     confLevel,
    OUT PRV_TGF_POLICER_QOS_PARAM_STC         *qosParamPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT dxChRemarkTableType;
    CPSS_DXCH_POLICER_QOS_PARAM_STC         dxChQosParam;

    CPSS_NULL_PTR_CHECK_MAC(qosParamPtr);

    /* convert remarkTableType into device specific format */
    switch (remarkTableType)
    {
        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E:
            dxChRemarkTableType = CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_TC_UP_E;
            break;

        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_DSCP_E:
            dxChRemarkTableType = CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_DSCP_E;
            break;

        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_EXP_E:
            dxChRemarkTableType = CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_EXP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChPolicerEgressQosRemarkingEntryGet(devNum, dxChRemarkTableType,
            remarkParamValue, confLevel, &dxChQosParam);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEgressQosRemarkingEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChQosParam from device specific format */
    qosParamPtr->up   = dxChQosParam.up;
    qosParamPtr->dscp = dxChQosParam.dscp;
    qosParamPtr->exp  = dxChQosParam.exp;
    qosParamPtr->dp   = dxChQosParam.dp;

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(remarkTableType);
    TGF_PARAM_NOT_USED(remarkParamValue);
    TGF_PARAM_NOT_USED(confLevel);
    TGF_PARAM_NOT_USED(qosParamPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerStageMeterModeSet function
* @endinternal
*
* @brief   Sets Policer Global stage mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
*/
GT_STATUS prvTgfPolicerStageMeterModeSet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN  PRV_TGF_POLICER_STAGE_METER_MODE_ENT mode
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT       dxChStage;
    CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT dxChMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert mode into device specific format */
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E,
                                PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E,
                                PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerStageMeterModeSet(devNum, dxChStage, dxChMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerStageMeterModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_TRIGGER_METER_MODE_ENT      pmMode;
    CPSS_PP_FAMILY_TYPE_ENT             devFamily;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert mode into device specific format */
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(pmMode,
                                CPSS_EXMXPM_POLICER_TRIGGER_METER_MODE_PORT_E,
                                PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

        PRV_TGF_SWITCH_CASE_MAC(pmMode,
                                CPSS_EXMXPM_POLICER_TRIGGER_METER_MODE_FLOW_E,
                                PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = prvUtfDeviceFamilyGet(devNum, &devFamily);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ((devFamily == CPSS_PP_FAMILY_PUMA_E) &&
            (mode == PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E))
        {
            /* puma2 supports only flow mode */
            continue;
        }

        /* call device specific API */
        rc = cpssExMxPmPolicerMeterModeSet(devNum, direction, pmMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerMeterModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerStageMeterModeGet function
* @endinternal
*
* @brief   Gets Policer Global stage mode.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) Policer meter mode: FLOW or PORT.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerStageMeterModeGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    OUT PRV_TGF_POLICER_STAGE_METER_MODE_ENT *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT       dxChStage;
    CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT dxChMode;

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerStageMeterModeGet(devNum, dxChStage, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerStageMeterModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert dxChResolution into device specific format */
    switch (dxChMode)
    {
        case CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E:
            *modePtr = PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E;
            break;

        case CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E:
            *modePtr = PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_TRIGGER_METER_MODE_ENT      pmMode;
    CPSS_PP_FAMILY_TYPE_ENT             devFamily;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = prvUtfDeviceFamilyGet(devNum, &devFamily);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (devFamily == CPSS_PP_FAMILY_PUMA_E)
    {
        /* puma2 supports only flow mode */
        *modePtr = PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E;
        return GT_OK;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerMeterModeGet(devNum, direction, &pmMode);

    /* convert pmMode into device specific format */
    switch (pmMode)
    {
        case CPSS_EXMXPM_POLICER_TRIGGER_METER_MODE_PORT_E:
            *modePtr = PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E;
            break;

        case CPSS_EXMXPM_POLICER_TRIGGER_METER_MODE_FLOW_E:
            *modePtr = PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPortStormTypeIndexSet function
* @endinternal
*
* @brief   Associates policing profile with source/target port and storm rate type.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
* @param[in] index                    - policer  (0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType ot port.
* @retval GT_OUT_OF_RANGE          - on out of range of index.
*/
GT_STATUS prvTgfPolicerPortStormTypeIndexSet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN GT_U32                                portNum,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT       stormType,
    IN GT_U32                               index
)
{

#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT       dxChStage;
    CPSS_DXCH_POLICER_STORM_TYPE_ENT       dxChStormType;


    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert command into device specific format */
    switch (stormType)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChStormType,
                                PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E,
                                CPSS_DXCH_POLICER_STORM_TYPE_UC_KNOWN_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChStormType,
                                PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E,
                                CPSS_DXCH_POLICER_STORM_TYPE_UC_UNKNOWN_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChStormType,
                                PRV_TGF_POLICER_STORM_TYPE_MC_UNREGISTERED_E,
                                CPSS_DXCH_POLICER_STORM_TYPE_MC_UNREGISTERED_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChStormType,
                                PRV_TGF_POLICER_STORM_TYPE_MC_REGISTERED_E,
                                CPSS_DXCH_POLICER_STORM_TYPE_MC_REGISTERED_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChStormType,
                                PRV_TGF_POLICER_STORM_TYPE_BC_E,
                                CPSS_DXCH_POLICER_STORM_TYPE_BC_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChStormType,
                                PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E,
                                CPSS_DXCH_POLICER_STORM_TYPE_TCP_SYN_E);

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChPolicerPortStormTypeIndexSet(devNum, dxChStage, portNum,
                                              dxChStormType, index);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortStormTypeIndexSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(stormType);
    TGF_PARAM_NOT_USED(index);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerL2RemarkModelSet function
* @endinternal
*
* @brief   Sets Policer L2 packets remarking model.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] model                    - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS prvTgfPolicerL2RemarkModelSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT      stage,
    IN PRV_TGF_POLICER_L2_REMARK_MODEL_ENT model
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT dxChModel;

    if (stage != PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        return GT_BAD_PARAM;
    }

    /* convert model into device specific format */
    switch (model)
    {
        case PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E:
            dxChModel = CPSS_DXCH_POLICER_L2_REMARK_MODEL_TC_E;
            break;

        case PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E:
            dxChModel = CPSS_DXCH_POLICER_L2_REMARK_MODEL_UP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerEgressL2RemarkModelSet(devNum, dxChModel);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEgressL2RemarkModelSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;
    CPSS_EXMXPM_POLICER_L2_REMARK_MODEL_ENT  pmModel;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert model into device specific format */
    switch (model)
    {
        case PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E:
            pmModel = CPSS_EXMXPM_POLICER_L2_REMARK_TC_E;
            break;

        case PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E:
            pmModel = CPSS_EXMXPM_POLICER_L2_REMARK_UP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerL2RemarkModelSet(devNum, direction, pmModel);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerL2RemarkModelSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(model);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEgressL2RemarkModelGet function
* @endinternal
*
* @brief   Gets Egress Policer L2 packets remarking model.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modelPtr                 - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerEgressL2RemarkModelGet
(
    IN  GT_U8                                    devNum,
    OUT PRV_TGF_POLICER_L2_REMARK_MODEL_ENT    *modelPtr
);

/**
* @internal prvTgfPolicerQosUpdateEnableSet function
* @endinternal
*
* @brief   The function enables or disables QoS remarking of conforming packets.
*
* @param[in] enable                   - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPolicerQosUpdateEnableSet
(
    IN GT_BOOL                          enable
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerEgressQosUpdateEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEgressQosUpdateEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerQosUpdateEnableSet(devNum, direction, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerQosUpdateEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEgressQosUpdateEnableGet function
* @endinternal
*
* @brief   The function get QoS remarking status of conforming packets
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - Enable/disable Qos update for conforming packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerEgressQosUpdateEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfPolicerVlanCntrSet function
* @endinternal
*
* @brief   Sets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
* @param[in] cntrValue                - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
GT_STATUS prvTgfPolicerVlanCntrSet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U16                         vid,
    IN  GT_U32                         cntrValue
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerVlanCntrSet(devNum, dxChStage, vid, cntrValue);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerVlanCntrSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerVlanCntrSet(devNum, direction, vid, cntrValue);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(cntrValue);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerVlanCntrGet function
* @endinternal
*
* @brief   Gets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
*
* @param[out] cntrValuePtr             - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
GT_STATUS prvTgfPolicerVlanCntrGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U16                         vid,
    OUT GT_U32                         *cntrValuePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerVlanCntrGet(devNum, dxChStage, vid, cntrValuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerVlanCntrGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerVlanCntrGet(devNum, direction, vid, cntrValuePtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(cntrValuePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerVlanCountingPacketCmdTriggerSet function
* @endinternal
*
* @brief   Enables or disables VLAN counting triggering according to the
*         specified packet command.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
* @param[in] enable                   - Enable/Disable VLAN Counting according to the packet
*                                      command trigger
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_SUPPORTED         - on not supported egress direction
*/
GT_STATUS prvTgfPolicerVlanCountingPacketCmdTriggerSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_PACKET_CMD_ENT            cmdTrigger,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerVlanCountingPacketCmdTriggerSet(devNum, dxChStage,
            cmdTrigger, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerVlanCountingPacketCmdTriggerSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerVlanCountingPacketCmdTriggerSet(devNum, direction,
                                                          cmdTrigger, enable);
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(cmdTrigger);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerVlanCountingPacketCmdTriggerGet function
* @endinternal
*
* @brief   Gets VLAN counting triggering status (Enable/Disable) according to the
*         specified packet command.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*
* @param[out] enablePtr                - Pointer to Enable/Disable VLAN Counting according
*                                      to the packet command trigger:
*                                      GT_TRUE  - Enable VLAN counting on packet with
*                                      triggered packet command.
*                                      GT_FALSE - Disable VLAN counting on packet with
*                                      triggered packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerVlanCountingPacketCmdTriggerGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  CPSS_PACKET_CMD_ENT            cmdTrigger,
    OUT GT_BOOL                        *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerVlanCountingPacketCmdTriggerGet(devNum, dxChStage,
            cmdTrigger, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerVlanCountingPacketCmdTriggerGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerVlanCountingPacketCmdTriggerGet(devNum, direction,
                                                          cmdTrigger, enablePtr);
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(cmdTrigger);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfPolicerPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
* @param[in] cntrValue                - packets counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters).
*/
GT_STATUS prvTgfPolicerPolicyCntrSet
(
    IN GT_U8                           devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         index,
    IN  GT_U32                         cntrValue
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChPolicerPolicyCntrSet(devNum, dxChStage, index, cntrValue);
    }
    else
    {
        rc = cpssDxChPolicerPortGroupPolicyCntrSet(devNum, currPortGroupsBmp, dxChStage, index, cntrValue);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupPolicyCntrSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssExMxPmPolicerPolicyCntrSet(devNum, direction, index, cntrValue);
    }
    else
    {
        rc = cpssExMxPmPolicerPortGroupPolicyCntrSet(devNum, currPortGroupsBmp, direction, index, cntrValue);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(cntrValue);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
*/
GT_STATUS prvTgfPolicerPolicyCntrGet
(
    IN GT_U8                           devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         index,
    OUT GT_U32                         *cntrValuePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChPolicerPolicyCntrGet(devNum, dxChStage, index, cntrValuePtr);
    }
    else
    {
        rc = cpssDxChPolicerPortGroupPolicyCntrGet(devNum, currPortGroupsBmp, dxChStage, index, cntrValuePtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortGroupPolicyCntrGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssExMxPmPolicerPolicyCntrGet(devNum, direction, index, cntrValuePtr);
    }
    else
    {
        rc = cpssExMxPmPolicerPortGroupPolicyCntrGet(devNum, currPortGroupsBmp, direction, index, cntrValuePtr);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(cntrValuePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountersSet function
* @endinternal
*
* @brief   Set Policer Counters. To reset counters use zero values.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - policing counters set index
* @param[in] countersPtr              - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerCountersSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterSetIndex,
    IN  PRV_TGF_POLICER_COUNTERS_STC *countersPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_POLICER_COUNTERS_STC dxChCounters;

    /* convert countersPtr into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChCounters), countersPtr, outOfProfileBytesCnt);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChCounters), countersPtr, inProfileBytesCnt);

    /* call device specific API */
    return cpssDxChPolicerCountersSet(devNum, counterSetIndex, &dxChCounters);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssExMxPmPolicerPolicyCntrSet(devNum, CPSS_DIRECTION_INGRESS_E, counterSetIndex,
                                        countersPtr->inProfileBytesCnt);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerPolicyCntrSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(counterSetIndex);
    TGF_PARAM_NOT_USED(countersPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPolicerCountersGet function
* @endinternal
*
* @brief   Get Policer Counters
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - policing counters set index
*
* @param[out] countersPtr              - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - on max number of retries checking if PP ready
*/
GT_STATUS prvTgfPolicerCountersGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterSetIndex,
    OUT PRV_TGF_POLICER_COUNTERS_STC *countersPtr
)
{
    GT_STATUS rc;
#ifdef CHX_FAMILY
    CPSS_DXCH_POLICER_COUNTERS_STC dxChCounters;
    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    /* call device specific API */
    rc = cpssDxChPolicerCountersGet(devNum, counterSetIndex, &dxChCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerCountersGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert countersPtr from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChCounters), countersPtr, outOfProfileBytesCnt);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChCounters), countersPtr, inProfileBytesCnt);

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    /* call device specific API */
    rc = cpssExMxPmPolicerPolicyCntrGet(devNum, CPSS_DIRECTION_INGRESS_E, counterSetIndex,
                                        &(countersPtr->inProfileBytesCnt));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerPolicyCntrGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(counterSetIndex);
    TGF_PARAM_NOT_USED(countersPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPolicerMemorySizeModeSet function
* @endinternal
*
* @brief   Sets internal table sizes and the way they are shared between the Ingress
*         policers.
* @param[in] devNum                   - device number.
* @param[in] mode                     - The  in which internal tables are shared.
* @param[in] numEntriesIngressStage0  - The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
* @param[in] numEntriesIngressStage1  - The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      NOTE: the egress policer gets the 'rest' of the metering/billing counters
*                                      the that device support.
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*/
GT_STATUS prvTgfPolicerMemorySizeModeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT mode,
    IN  GT_U32                              numEntriesIngressStage0,
    IN  GT_U32                              numEntriesIngressStage1
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT dxChMode;
    GT_U32  numEntriesEgressStage;

    /* convert command into device specific format */
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_4_E,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_5_E,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChMode,
                                PRV_TGF_POLICER_MEMORY_FLEX_MODE_E,
                                CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E);
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChPolicerMemorySizeModeSet(devNum, dxChMode ,
        numEntriesIngressStage0 , numEntriesIngressStage1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerMemorySizeModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChPolicerMemorySizeModeGet(devNum, &dxChMode ,
        &numEntriesIngressStage0 , &numEntriesIngressStage1 , &numEntriesEgressStage);
    if (GT_OK == rc)
    {
        PRV_UTF_LOG4_MAC("[TGF]: Policer sizes changed: mode[%d] PLR0[%d] PLR1[%d] EPLR[%d] \n",
            dxChMode,
            numEntriesIngressStage0 ,
            numEntriesIngressStage1 ,
            numEntriesEgressStage);
    }



    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMemorySizeModeGet function
* @endinternal
*
* @brief   Gets internal table sizes and the way they are shared between the Ingress
*         policers.
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - The mode in which internal tables are shared.
* @param[out] numEntriesIngressStage0Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      NOTE: ignored if NULL
* @param[out] numEntriesIngressStage1Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      NOTE: ignored if NULL
* @param[out] numEntriesEgressStagePtr - (pointer to) The number of metering/billing counters for
*                                      egress policer stage
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerMemorySizeModeGet
(
    IN  GT_U8                                      devNum,
    OUT PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT     *modePtr,
    OUT GT_U32                                     *numEntriesIngressStage0Ptr,
    OUT GT_U32                                     *numEntriesIngressStage1Ptr,
    OUT GT_U32                                     *numEntriesEgressStagePtr
)
{

#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT dxChMode;

    /* call device specific API */
    rc = cpssDxChPolicerMemorySizeModeGet(devNum, &dxChMode,
        numEntriesIngressStage0Ptr,
        numEntriesIngressStage1Ptr,
        numEntriesEgressStagePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerMemorySizeModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert command into device specific format */
    switch (dxChMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E);

        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_4_E);

        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E,
                                PRV_TGF_POLICER_MEMORY_CTRL_MODE_5_E);

        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E,
                                PRV_TGF_POLICER_MEMORY_FLEX_MODE_E);
        default:
            return GT_BAD_STATE;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountingWriteBackCacheFlush function
* @endinternal
*
* @brief   Flush internal Write Back Cache (WBC) of counting entries.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_TIMEOUT               - on time out.
*/
GT_STATUS prvTgfPolicerCountingWriteBackCacheFlush
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, dxChStage);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerCountingWriteBackCacheFlush FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8     devNum;
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerCountingWriteBackCacheFlush(devNum, direction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerCountingWriteBackCacheFlush FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerConformanceLevelForce function
* @endinternal
*
* @brief   force the conformance level for the packets entering the policer
*         (traffic cond)
* @param[in] dp                       -  conformance level (drop precedence) - green/yellow/red
*
* @retval GT_OK                    - success, GT_FAIL otherwise
*/
GT_STATUS prvTgfPolicerConformanceLevelForce(
    IN  GT_U32      dp
)
{
#ifdef ASIC_SIMULATION
    GT_STATUS rc;

    /* set the conformance level */
    rc = skernelPolicerConformanceLevelForce(dp);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: skernelPolicerConformanceLevelForce FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif

#if !(defined ASIC_SIMULATION)
    TGF_PARAM_NOT_USED(dp);

    return GT_OK;
#endif /* !(defined CHX_FAMILY) */
}

#ifdef CHX_FAMILY
static GT_STATUS prvTgfDxChPlrStageCheck
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     dxChStage
)
{
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum,dxChStage);
    return GT_OK;
}
#endif /* CHX_FAMILY */

/**
* @internal prvTgfPolicerStageCheck function
* @endinternal
*
* @brief   check if the device supports this policer stage
*
* @param[in] stage                    -  policer  to check
*
* @retval GT_TRUE                  - the device supports the stage
* @retval GT_FALSE                 - the device not supports the stage
*/
GT_BOOL prvTgfPolicerStageCheck(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U8     devNum = prvTgfDevNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerStageCheck FAILED, rc = [%d]", rc);
        return GT_FALSE;
    }

    if(GT_OK != prvTgfDxChPlrStageCheck(devNum,dxChStage))
    {
        /* the device not support the state */
        return GT_FALSE;
    }

    return GT_TRUE;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerPortModeAddressSelectSet function
* @endinternal
*
* @brief   Configure Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] type                     - Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or type.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerPortModeAddressSelectSet
(
    IN  GT_U8                                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT                      stage,
    IN  PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT      type
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT      dxChType;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                      dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert command into device specific format */
    dxChType = (type != PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E) ?
        CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E :
        CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E;

    /* call device specific API */
    rc = cpssDxChPolicerPortModeAddressSelectSet(devNum, dxChStage, dxChType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortModeAddressSelectSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(type);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfPolicerPortModeAddressSelectGet function
* @endinternal
*
* @brief   Get Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] typePtr                  - (pointer to) Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerPortModeAddressSelectGet
(
    IN  GT_U8                                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT                      stage,
    OUT PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT      *typePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT      dxChType;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                      dxChStage;


    CPSS_NULL_PTR_CHECK_MAC(typePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerPortModeAddressSelectGet(devNum, dxChStage, &dxChType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerPortModeAddressSelectGet FAILED, rc = [%d]", rc);

        return rc;
    }


    /* convert command into device specific format */
    *typePtr = (dxChType == CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E) ?
        PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E :
        PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E;

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(typePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountingTriggerByPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Counting when port based metering is triggered for
*         the packet.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerCountingTriggerByPortEnableSet
(
    IN  GT_U8                                   devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN GT_BOOL                                  enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                      dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerCountingTriggerByPortEnableSet(devNum, dxChStage, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerCountingTriggerByPortEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerCountingTriggerByPortEnableGet function
* @endinternal
*
* @brief   Get Counting trigger by port status.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerCountingTriggerByPortEnableGet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    OUT GT_BOOL                                 *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                      dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerCountingTriggerByPortEnableGet(devNum, dxChStage, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerCountingTriggerByPortEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEAttributesMeteringModeSet function
* @endinternal
*
* @brief   Set ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerEAttributesMeteringModeSet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN PRV_TGF_POLICER_E_ATTR_METER_MODE_ENT    mode
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                    dxChStage;
    CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT             cpssMode;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(mode)
    {
        case PRV_TGF_POLICER_E_ATTR_METER_MODE_DISABLED_E:
            cpssMode = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_DISABLED_E;
            break;
        case PRV_TGF_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E:
            cpssMode = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E;
            break;
        case PRV_TGF_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E:
            cpssMode = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* call device specific API */
    rc = cpssDxChPolicerEAttributesMeteringModeSet(devNum, dxChStage, cpssMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEAttributesMeteringModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mode);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerEAttributesMeteringModeGet function
* @endinternal
*
* @brief   Get ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfPolicerEAttributesMeteringModeGet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    OUT PRV_TGF_POLICER_E_ATTR_METER_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                    dxChStage;
    CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT             dxChEAttrMode;

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerEAttributesMeteringModeGet(devNum, dxChStage, &dxChEAttrMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerEAttributesMeteringModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    *modePtr = PRV_TGF_POLICER_E_ATTR_METER_MODE_DISABLED_E;
    if(dxChEAttrMode == CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E)
    {
        /* ePort enable */
        *modePtr = PRV_TGF_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E;
    }
    else if(dxChEAttrMode == CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E)
    {
        /* eVlan enable */
        *modePtr = PRV_TGF_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerTriggerEntrySet function
* @endinternal
*
* @brief   Set Policer trigger entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index
* @param[in] entryPtr                 - (pointer to) Policer port trigger entry
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on table index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerTriggerEntrySet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    IN  GT_U32                                  entryIndex,
    IN  PRV_TGF_POLICER_TRIGGER_ENTRY_STC       *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                      dxChStage;
    CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC                   dxChTriggerEntry;

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    prvTgfConvertGenericToDxChTriggerEntry(entryPtr, &dxChTriggerEntry);

    /* call device specific API */
    rc = cpssDxChPolicerTriggerEntrySet(devNum, dxChStage, entryIndex, &dxChTriggerEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerTriggerEntrySet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerTriggerEntryGet function
* @endinternal
*
* @brief   Get Policer trigger entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index
*
* @param[out] entryPtr                 - (pointer to) Policer port trigger entry
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on table index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerTriggerEntryGet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    IN  GT_U32                                  entryIndex,
    OUT PRV_TGF_POLICER_TRIGGER_ENTRY_STC       *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT                      dxChStage;
    CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC                   dxChTriggerEntry;

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPolicerTriggerEntryGet(devNum, dxChStage, entryIndex, &dxChTriggerEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerTriggerEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    prvTgfConvertDxChToGenericTriggerEntry(&dxChTriggerEntry, entryPtr);


    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerQosParamsRemarkEntrySet function
* @endinternal
*
* @brief   Sets Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkProfileTableIndex,
*                                       remarkParamType or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC   | [0:7] |
*                                       | UP   | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPolicerQosParamsRemarkEntrySet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN GT_U32                                   remarkProfileTableIndex,
    IN PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT    remarkParamType,
    IN GT_U32                                   remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                        confLevel,
    IN PRV_TGF_POLICER_QOS_PARAM_STC            *qosParamPtr
)
{

#ifdef EXMXPM_FAMILY
    GT_U8                               devNum;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_DIRECTION_ENT                  direction;
    CPSS_EXMXPM_QOS_PARAM_ENT           pmRemarkParamType;
    CPSS_EXMXPM_QOS_PARAM_STC           pmQosParam;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    switch (remarkParamType)
    {
        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E:
            pmRemarkParamType = CPSS_EXMXPM_QOS_PARAM_TC_E;
            break;
        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_DSCP_E:
            pmRemarkParamType = CPSS_EXMXPM_QOS_PARAM_DSCP_E;
            break;
        case PRV_TGF_POLICER_REMARK_TABLE_TYPE_EXP_E:
            pmRemarkParamType = CPSS_EXMXPM_QOS_PARAM_EXP_E;
            break;
        default:return GT_BAD_PARAM;
    }

    pmQosParam.dp   = qosParamPtr->dp;
    pmQosParam.dscp = qosParamPtr->dscp;
    pmQosParam.exp  = qosParamPtr->exp;
    pmQosParam.tc   = qosParamPtr->tc;
    pmQosParam.up   = qosParamPtr->up;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerQosRemarkEntrySet(devNum, direction, remarkProfileTableIndex, pmRemarkParamType, remarkParamValue, confLevel, &pmQosParam);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPolicerQosRemarkEntrySet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if  !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(remarkProfileTableIndex);
    TGF_PARAM_NOT_USED(remarkParamType);
    TGF_PARAM_NOT_USED(remarkParamValue);
    TGF_PARAM_NOT_USED(confLevel);
    TGF_PARAM_NOT_USED(qosParamPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPolicerTrappedPacketsBillingEnableSet function
* @endinternal
*
* @brief   Enables the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerTrappedPacketsBillingEnableSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT   stage,
    IN GT_BOOL                          enable
)
{
    GT_U8     devNum = 0;
#ifdef CHX_FAMILY
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPolicerTrappedPacketsBillingEnableSet(devNum, dxChStage, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerMeteringEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */
#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfPolicerLossMeasurementCounterCaptureEnableSet function
* @endinternal
*
* @brief   Enables or disables Loss Measurement capture.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable Loss Measurement capture.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerLossMeasurementCounterCaptureEnableSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN GT_BOOL                              enable
)
{
    GT_U8       devNum  = 0;
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Enable/disable LM counter capture */
        rc = cpssDxChPolicerLossMeasurementCounterCaptureEnableSet(
                                                    devNum, dxChStage, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamLossMeasurementCounterCaptureEnableSet FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfPolicerFlowIdCountingCfgSet function
* @endinternal
*
* @brief   Set Flow Id based counting configuration.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] billingIndexCfgPtr       - (pointer to) billing index configuration structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfPolicerFlowIdCountingCfgSet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN  PRV_TGF_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
)
{
    GT_U8       devNum  = 0;
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         dxChStage;
    CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  dxChBillingIndexCfg;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    cpssOsMemSet(&dxChBillingIndexCfg, 0, sizeof(dxChBillingIndexCfg));

    switch (billingIndexCfgPtr->billingIndexMode)
    {
        case PRV_TGF_POLICER_BILLING_INDEX_MODE_STANDARD_E:
            dxChBillingIndexCfg.billingIndexMode =
                CPSS_DXCH_POLICER_BILLING_INDEX_MODE_STANDARD_E;
            break;
        case PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E:
            dxChBillingIndexCfg.billingIndexMode =
                CPSS_DXCH_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;
            break;
        default: return GT_BAD_PARAM;
    }

    dxChBillingIndexCfg.billingFlowIdIndexBase = billingIndexCfgPtr->billingFlowIdIndexBase;
    dxChBillingIndexCfg.billingMinFlowId       = billingIndexCfgPtr->billingMinFlowId;
    dxChBillingIndexCfg.billingMaxFlowId       = billingIndexCfgPtr->billingMaxFlowId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Enable/disable LM counter capture */
        rc = cpssDxChPolicerFlowIdCountingCfgSet(
            devNum, dxChStage, &dxChBillingIndexCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerFlowIdCountingCfgSet FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(billingIndexCfgPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}
/**
* @internal prvTgfPolicerMruSet function
* @endinternal
*
* @brief   Sets the Policer Maximum Receive Unit size.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mruSize                  - Policer MRU value in bytes, (APPLICABLE RANGES: 0..0xFFFF).
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_OUT_OF_RANGE          - on mruSize out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerMruSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN GT_U32                               mruSize
)
{

#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxCh3PolicerMruSet(devNum, dxChStage, mruSize);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMruSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmPolicerMruSet(devNum, direction, mruSize);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mruSize);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerMruGet function
* @endinternal
*
* @brief   Gets the Policer Maximum Receive Unit size.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] mruSizePtr               - pointer to the Policer MRU value in bytes.
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMruGet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    OUT GT_U32                            *mruSizePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT dxChStage;

    CPSS_NULL_PTR_CHECK_MAC(mruSizePtr);

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChPolicerStage(stage, &dxChStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChPolicerStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxCh3PolicerMruGet(devNum, dxChStage, mruSizePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxCh3PolicerMruGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc;
    CPSS_DIRECTION_ENT direction;

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToExMxPmPolicerDirection(stage, &direction);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPolicerMruGet(devNum, direction, mruSizePtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(mruSizePtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}
/**
* @internal prvTgfPolicerHierarchicalTableEntrySet function
* @endinternal
*
* @brief   Set Hierarchical Table Entry configuration.
*
* @param[in] index                    - Hierarchical Table Entry index.
* @param[in] entryPtr                 - (pointer to) Hierarchical Table Entry structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
GT_STATUS prvTgfPolicerHierarchicalTableEntrySet
(
    IN  GT_U32                                          index,
    IN  PRV_TGF_POLICER_HIERARCHICAL_TABLE_ENTRY_STC    *entryPtr
)
{
    GT_U8       devNum  = 0;
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  dxChEntry;

    cpssOsMemSet(&dxChEntry, 0, sizeof(dxChEntry));
    dxChEntry.countingEnable = entryPtr->countingEnable;
    dxChEntry.meteringEnable = entryPtr->meteringEnable;
    dxChEntry.policerPointer = entryPtr->policerPointer;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Enable/disable LM counter capture */
        rc = cpssDxChPolicerHierarchicalTableEntrySet(
            devNum, index, &dxChEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPolicerHierarchicalTableEntrySet FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(entryPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfPolicerManagementCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management Counters
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
*                                       None
*/
GT_VOID prvTgfPolicerManagementCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT mngSet
)
{
    GT_STATUS                           rc;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntr;

    cpssOsMemSet(&prvTgfMngCntr, 0, sizeof(prvTgfMngCntr));

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerManagementCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*/
static GT_VOID prvTgfPolicerManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       policerStage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT    mngCntrSet,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr,
    IN GT_U32 callIndex
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            mngCntrSet,
                                            mngCntrType,
                                            &prvTgfMngCntr);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "%d: prvTgf3PolicerManagementCountersGet: %d", callIndex, prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL5_STRING_MAC(
        duLowCntr, prvTgfMngCntr.duMngCntr.l[0],
        "%d: prvTgfMngCntr.duMngCntr.l[0] = %d, policerStage = %d, "
             "mngCntrSet = %d, mngCntrType = %d\n",
        callIndex, prvTgfMngCntr.duMngCntr.l[0], policerStage, mngCntrSet,
        mngCntrType);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        duHiCntr, prvTgfMngCntr.duMngCntr.l[1],
        "%d: prvTgfMngCntr.duMngCntr.l[1] = %d, policerStage = %d, "
             "mngCntrSet = %d, mngCntrType = %d\n",
        callIndex, prvTgfMngCntr.duMngCntr.l[1], policerStage, mngCntrSet,
        mngCntrType);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        packetMngCntr, prvTgfMngCntr.packetMngCntr,
        "%d: prvTgfMngCntr.packetMngCntr = %d, policerStage = %d, "
             "mngCntrSet = %d, mngCntrType = %d\n",
        callIndex, prvTgfMngCntr.packetMngCntr, policerStage, mngCntrSet,
        mngCntrType);
}

/**
* @internal prvTgfPolicerFullManagementCountersCheck function
* @endinternal
*
* @brief   Check Management and Billing counters value
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
* @param[in] expMngCounters[4]        - expected Management counters value
* @param[in] callIndex                - reference to this check sequence call
*                                       None
*/
GT_VOID prvTgfPolicerFullManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT     mngSet,
    IN GT_U32                               expMngCounters[4][3],
    IN GT_U32                               callIndex
)
{
    prvTgfPolicerManagementCountersCheck(stage, mngSet,
                            PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][0],
                            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][1],
                            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][2],
                            callIndex);

    prvTgfPolicerManagementCountersCheck(stage, mngSet,
                           PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                           expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][0],
                           expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][1],
                           expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][2],
                           callIndex);

    prvTgfPolicerManagementCountersCheck(stage, mngSet,
                              PRV_TGF_POLICER_MNG_CNTR_RED_E,
                              expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0],
                              expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1],
                              expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2],
                              callIndex);

    prvTgfPolicerManagementCountersCheck(stage, mngSet,
                             PRV_TGF_POLICER_MNG_CNTR_DROP_E,
                             expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][0],
                             expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][1],
                             expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][2],
                             callIndex);
}

/**
* @internal prvTgfPolicerEgressCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            - sending port index
*                                      recPortindx     - receiving port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
* @param[in] packetInfoPtr            - the packet format to compare the captured packet with.
* @param[in] prvTgfBurstCount         - number of packets to send.
*                                       None
*/
GT_VOID prvTgfPolicerEgressCheckCounters
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           recPortIndex,
    IN GT_U32           expectedValue,
    IN GT_U32           callIdentifier,
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32            prvTgfBurstCount
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32  actualCapturedNumOfPackets;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[recPortIndex];

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (sendPortIndex == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL4_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "Rx another goodPktsRcv counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(expectedValue, portCntrs.goodPktsSent.l[0],
                                         "Tx another goodPktsSent counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsSent.l[0], expectedValue,
                                         prvTgfPortsArray[portIter], callIdentifier);

            if(recPortIndex == portIter)
            {
                /* print captured packets and check TriggerCounters */
                rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                        &portInterface,
                        packetInfoPtr,
                        prvTgfBurstCount,/*numOfPackets*/
                        0/*vfdNum*/,
                        NULL /*vfdArray*/,
                        NULL, /*byteNumMaskList[]*/
                        0, /*byteNumMaskListLen*/
                        &actualCapturedNumOfPackets,
                        NULL/*onFirstPacketNumTriggersBmpPtr*/);

                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     "port = %d, rc = 0x%02X, %d\n",
                                     portInterface.devPort.portNum, rc, callIdentifier);
            }
        }
    }
}

/**
* @internal prvTgfPolicerEgressCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management Counters
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
*                                       None
*/
GT_VOID prvTgfPolicerEgressCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT mngSet
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrGreen;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrRed;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrYellow;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrDrop;

    cpssOsMemSet(&prvTgfMngCntrGreen, 0, sizeof(prvTgfMngCntrGreen));
    cpssOsMemSet(&prvTgfMngCntrRed, 0, sizeof(prvTgfMngCntrRed));
    cpssOsMemSet(&prvTgfMngCntrYellow, 0, sizeof(prvTgfMngCntrYellow));
    cpssOsMemSet(&prvTgfMngCntrDrop, 0, sizeof(prvTgfMngCntrDrop));

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerEgressInitEntrySet function
* @endinternal
*
* @brief   Initial metering entry setting
*
* @param[in] stage                    - Policer stage
* @param[in] portNum                  - egress port number
*                                       None
*/
GT_VOID prvTgfPolicerEgressInitEntrySet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    GT_U32                         portNum
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_ENTRY_STC   meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;

    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.policerEnable = GT_TRUE;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET1_E;

    /* AUTODOC: Metering according to L2 */
    meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

    meterEntry.modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyTc = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    /* AUTODOC: Setting the initial metering entry */
    rc = prvTgfPolicerEntrySet(stage,
                               prvTgfPortsArray[portNum],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                 stage,
                                 prvTgfPortsArray[portNum]);
}

/**
* @internal prvTgfPolicerEgressTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] sendPortIndex            - sending port indexing place in test array ports.
* @param[in] recPortIndex             - receiving port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] prvTgfBurstCount         - number of packets to send.
*                                       None
*/
GT_VOID prvTgfPolicerEgressTestPacketSend
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           recPortIndex,
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32           prvTgfBurstCount
)
{
    GT_STATUS   rc;
    GT_U32      portIter;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[recPortIndex];

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);


    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}



/**
* @internal prvTgfPolicerEgressIsByteCountCrcExclude function
* @endinternal
*
* @brief   Checks if need exclude 4 bytes CRC from byte count calculation for
*         egress policer. Bobcat2 excludes CRC from byte count.
*
* @retval GT_FALSE                 - do not exclude CRC.
* @retval GT_TRUE                  - exclude CRC from byte count.
*/
GT_BOOL prvTgfPolicerEgressIsByteCountCrcExclude
(
    void
)
{
    GT_BOOL  result = GT_FALSE; /* do not exclude */

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
#ifdef CHX_FAMILY
        if(0 == PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
            /* Due to Error excludes CRC from byte count calculation */
            /* JIRA : POLICER-1304 : Egress Policer byte count does not include CRC */
            result = GT_TRUE;
        }
#endif /*CHX_FAMILY*/
    }
    return result;
}

/**
* @internal prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/

GT_STATUS prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet
(
    IN  GT_BOOL                       enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U8     devNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet(
            devNum, enable);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet FAILED, rc = [%d]",
                rc);
            return rc;
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enable);

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableGet
(
    OUT GT_BOOL                       *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U8     devNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet(
            devNum, enablePtr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet FAILED, rc = [%d]",
                rc);
            return rc;
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet
(
    IN  GT_BOOL                       enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U8     devNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet(
            devNum, enable);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet FAILED, rc = [%d]",
                rc);
            return rc;
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enable);

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableGet
(
    OUT GT_BOOL                       *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U8     devNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet(
            devNum, enablePtr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet FAILED, rc = [%d]",
                rc);
            return rc;
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

