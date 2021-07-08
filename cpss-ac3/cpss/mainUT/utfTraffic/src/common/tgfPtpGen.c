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
* @file tgfPtpGen.c
*
* @brief Generic API for Presision Time Protocol (PTP) API.
*
* @version   2
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
#include <common/tgfPtpGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
#endif /* EXMXPM_FAMILY */

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

#ifdef CHX_FAMILY

static GT_STATUS tgf2dxchPTP_TAI_ID_STC
(
    IN  PRV_TGF_PTP_TAI_ID_STC   *tgfTaiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_ID_STC *dxchTaiIdPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(tgfTaiIdPtr);

    cpssOsMemSet(dxchTaiIdPtr, 0, sizeof(*dxchTaiIdPtr));

    switch (tgfTaiIdPtr->taiInstance)
    {
        case PRV_TGF_PTP_TAI_INSTANCE_GLOBAL_E:
            dxchTaiIdPtr->taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E;
            break;
        case PRV_TGF_PTP_TAI_INSTANCE_PORT_E:
            dxchTaiIdPtr->taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_PORT_E;
            break;
        case PRV_TGF_PTP_TAI_INSTANCE_ALL_E:
            dxchTaiIdPtr->taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E;
            break;
        default: return GT_BAD_PARAM;
    }

    switch (tgfTaiIdPtr->taiNumber)
    {
        case PRV_TGF_PTP_TAI_NUMBER_0_E:
            dxchTaiIdPtr->taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
            break;
        case PRV_TGF_PTP_TAI_NUMBER_1_E:
            dxchTaiIdPtr->taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_1_E;
            break;
        case PRV_TGF_PTP_TAI_NUMBER_ALL_E:
            dxchTaiIdPtr->taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
            break;
        default: return GT_BAD_PARAM;
    }

    dxchTaiIdPtr->portNum = tgfTaiIdPtr->portNum;

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TOD_COUNTER_FUNC_ENT
(
    IN  PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT   tgfFunction,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT *dxchFunctionPtr
)
{
    switch (tgfFunction)
    {
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_UPDATE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_INCREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_CAPTURE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_GENERATE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GENERATE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_DECREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E;
            break;
        case PRV_TGF_PTP_TOD_COUNTER_FUNC_NOP_E:
            *dxchFunctionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS dxch2tgfPTP_TOD_COUNTER_FUNC_ENT
(
    IN  CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT dxchFunction,
    OUT PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT   *tgfFunctionPtr
)
{
    switch (dxchFunction)
    {
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_UPDATE_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_INCREMENT_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_CAPTURE_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GENERATE_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_GENERATE_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_DECREMENT_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E:
            *tgfFunctionPtr = PRV_TGF_PTP_TOD_COUNTER_FUNC_NOP_E;
            break;
        default: return GT_BAD_STATE;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TAI_TOD_TYPE_ENT
(
    IN  PRV_TGF_PTP_TAI_TOD_TYPE_ENT    tgfTaiTodType,
    OUT CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT  *dxchTaiTodTypePtr
)
{
    switch (tgfTaiTodType)
    {
        case PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E:
            *dxchTaiTodTypePtr = CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E;
            break;
        case PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E:
            *dxchTaiTodTypePtr = CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E;
            break;
        case PRV_TGF_PTP_TAI_TOD_TYPE_LOAD_VALUE_E:
            *dxchTaiTodTypePtr = CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E;
            break;
        case PRV_TGF_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E:
            *dxchTaiTodTypePtr = CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E;
            break;
        case PRV_TGF_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E:
            *dxchTaiTodTypePtr = CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TS_ACTION_ENT
(
    IN  PRV_TGF_PTP_TS_ACTION_ENT    tgfTsAction,
    OUT CPSS_DXCH_PTP_TS_ACTION_ENT  *dxchTsActionPtr
)
{
    switch (tgfTsAction)
    {
        case PRV_TGF_PTP_TS_ACTION_NONE_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_NONE_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_DROP_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_DROP_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E:
            *dxchTsActionPtr = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS dxch2tgfPTP_TS_ACTION_ENT
(
    IN  CPSS_DXCH_PTP_TS_ACTION_ENT  dxchTsAction,
    OUT PRV_TGF_PTP_TS_ACTION_ENT    *tgfTsActionPtr
)
{
    switch (dxchTsAction)
    {
        case CPSS_DXCH_PTP_TS_ACTION_NONE_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_NONE_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_DROP_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_DROP_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_CAPTURE_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_TIME_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_TIME_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E:
            *tgfTsActionPtr = PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
            break;
        default: return GT_BAD_STATE;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TS_PACKET_TYPE_ENT
(
    IN  PRV_TGF_PTP_TS_PACKET_TYPE_ENT    tgfPacketType,
    OUT CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT  *dxchPacketTypePtr
)
{
    switch (tgfPacketType)
    {
        case PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V1_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TS_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_NTP_RX_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TX_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E;
            break;
        case PRV_TGF_PTP_TS_PACKET_TYPE_WAMP_E:
            *dxchPacketTypePtr = CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS dxch2tgfPTP_TS_PACKET_TYPE_ENT
(
    IN  CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT  dxchPacketType,
    OUT PRV_TGF_PTP_TS_PACKET_TYPE_ENT    *tgfPacketTypePtr
)
{
    switch (dxchPacketType)
    {
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V1_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TS_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_NTP_RX_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TX_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_WAMP_E;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_RESERVED_E:
            *tgfPacketTypePtr = PRV_TGF_PTP_TS_PACKET_TYPE_RESERVED_E;
            break;
        default: return GT_BAD_STATE;
    }

    return GT_OK;
}

static GT_STATUS tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT
(
    IN  PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT    tgfCsUpdMode,
    OUT CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT  *dxchCsUpdModePtr
)
{
    switch (tgfCsUpdMode)
    {
        case PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E:
            *dxchCsUpdModePtr = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E;
            break;
        case PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E:
            *dxchCsUpdModePtr = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E;
            break;
        case PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E:
            *dxchCsUpdModePtr = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

static GT_STATUS dxch2tgfPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT
(
    IN  CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT    dxchCsUpdMode,
    OUT PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT      *tgfCsUpdModePtr
)
{
    switch (dxchCsUpdMode)
    {
        case CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E:
            *tgfCsUpdModePtr = PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E;
            break;
        case CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E:
            *tgfCsUpdModePtr = PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E;
            break;
        case CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E:
            *tgfCsUpdModePtr = PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E;
            break;
        default: return GT_BAD_STATE;
    }

    return GT_OK;
}

#endif /* CHX_FAMILY */

/**
* @internal prvTgfPtpEtherTypeSet function
* @endinternal
*
* @brief   Configure ethertype0/ethertype1 of PTP over Ethernet packets.
*         A packet is identified as PTP over Ethernet if its EtherType matches
*         one of the configured values.
* @param[in] etherType                - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] etherType                - PTP EtherType0 or EtherType1 according to the index.
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_OUT_OF_RANGE          - on wrong etherType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEtherTypeSet
(
    IN GT_U32   etherTypeIndex,
    IN GT_U32   etherType
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpEtherTypeSet(
            devNum, etherTypeIndex, etherType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpEtherTypeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(etherTypeIndex);
    TGF_PARAM_NOT_USED(etherType);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpEtherTypeGet function
* @endinternal
*
* @brief   Get ethertypes of PTP over Ethernet packets.
*
* @param[out] etherTypePtr             - (pointer to) PTP EtherType0 or EtherType1 according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEtherTypeGet
(
    IN  GT_U32   etherTypeIndex,
    OUT GT_U32   *etherTypePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpEtherTypeGet(
        devNum, etherTypeIndex, etherTypePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpEtherTypeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(etherTypeIndex);
    TGF_PARAM_NOT_USED(etherTypePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpUdpDestPortsSet function
* @endinternal
*
* @brief   Configure UDP destination port0/port1 of PTP over UDP packets.
*         A packet is identified as PTP over UDP if it is a UDP packet, whose
*         destination port matches one of the configured ports.
* @param[in] udpPortIndex             - UDP port index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] udpPortNum               - UDP port1/port0 number according to the index,
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_OUT_OF_RANGE          - on wrong udpPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpUdpDestPortsSet
(
    IN GT_U32   udpPortIndex,
    IN GT_U32   udpPortNum
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpUdpDestPortsSet(
            devNum, udpPortIndex, udpPortNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpUdpDestPortsSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(udpPortIndex);
    TGF_PARAM_NOT_USED(udpPortNum);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpUdpDestPortsGet function
* @endinternal
*
* @brief   Get UDP destination port0/port1 of PTP over UDP packets.
*
* @param[in] udpPortIndex             - UDP port index
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] udpPortNumPtr            - (pointer to) UDP port0/port1 number, according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpUdpDestPortsGet
(
    IN  GT_U32   udpPortIndex,
    OUT GT_U32   *udpPortNumPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpUdpDestPortsGet(
        devNum, udpPortIndex, udpPortNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpUdpDestPortsGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(udpPortIndex);
    TGF_PARAM_NOT_USED(udpPortNumPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpMessageTypeCmdSet function
* @endinternal
*
* @brief   Configure packet command per PTP message type.
*         The message type is extracted from the PTP header.
* @param[in] portNum                  - port number.
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] command                  - assigned  to the packet.
*                                      Acceptable values: FORWARD, MIRROR, TRAP, HARD_DROP and
*                                      SOFT_DROP.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpMessageTypeCmdSet
(
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   domainNum,
    IN GT_U32                   messageType,
    IN CPSS_PACKET_CMD_ENT      command
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpMessageTypeCmdSet(
            devNum, portNum, domainNum, messageType, command);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpMessageTypeCmdSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(command);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpMessageTypeCmdGet function
* @endinternal
*
* @brief   Get packet command per PTP message type.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] commandPtr               - (pointer to) assigned command to the packet.
*                                      Acceptable values: FORWARD, MIRROR, TRAP, HARD_DROP and
*                                      SOFT_DROP.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpMessageTypeCmdGet
(
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   domainNum,
    IN  GT_U32                   messageType,
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpMessageTypeCmdGet(
        devNum, portNum, domainNum, messageType, commandPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpMessageTypeCmdGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(commandPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets mirrored
*                                      or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpCpuCodeBaseSet
(
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpCpuCodeBaseSet(
            devNum, cpuCode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpCpuCodeBaseSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cpuCode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @param[out] cpuCodePtr               - (pointer to) The base of CPU code assigned to PTP
*                                      packets mirrored or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpCpuCodeBaseGet
(
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpCpuCodeBaseGet(
        devNum, cpuCodePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpCpuCodeBaseGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cpuCodePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by prvTgfPtpTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
* @param[in] direction                - Tx, Rx or both directions
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfFunction              - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionSet
(
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN PRV_TGF_PTP_TAI_ID_STC                   *tgfTaiIdPtr,
    IN PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT         tgfFunction
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT         dxchFunction;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TOD_COUNTER_FUNC_ENT(tgfFunction, &dxchFunction);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTodCounterFunctionSet(
            devNum, direction, &dxchTaiId, dxchFunction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTodCounterFunctionSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfFunction);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         prvTgfPtpTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
* @param[in] direction                - Tx, Rx or both directions
*                                      taiIdPtr        - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionGet
(
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT        *tgfFunctionPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT         dxchFunction;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTodCounterFunctionGet(
        devNum, direction, &dxchTaiId, &dxchFunction);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTodCounterFunctionGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    rc = dxch2tgfPTP_TOD_COUNTER_FUNC_ENT(dxchFunction, tgfFunctionPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfFunctionPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by prvTgfPtpTodCounterFunctionSet().
* @param[in] direction                - Tx, Rx or both directions
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionTriggerSet
(
    IN CPSS_PORT_DIRECTION_ENT     direction,
    IN PRV_TGF_PTP_TAI_ID_STC      *tgfTaiIdPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(
            devNum, direction, &dxchTaiId);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTodCounterFunctionTriggerSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodCounterFunctionAllTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for all TAI units.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodCounterFunctionAllTriggerSet
(
    IN GT_VOID
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiTodCounterFunctionAllTriggerSet(devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodCounterFunctionAllTriggerSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTodCounterFunctionTriggerGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @param[in] direction                - Tx, Rx
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionTriggerGet
(
    IN CPSS_PORT_DIRECTION_ENT     direction,
    IN  PRV_TGF_PTP_TAI_ID_STC     *tgfTaiIdPtr,
    OUT GT_BOOL                    *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTodCounterFunctionTriggerGet(
        devNum, direction, &dxchTaiId, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTodCounterFunctionTriggerGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfClockMode             - clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockModeSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_CLOCK_MODE_ENT          tgfClockMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT           dxChClockMode;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }
    switch (tgfClockMode)
    {
        case PRV_TGF_PTP_TAI_CLOCK_MODE_DISABLED_E:
            dxChClockMode = CPSS_DXCH_PTP_TAI_CLOCK_MODE_DISABLED_E;
            break;
        case PRV_TGF_PTP_TAI_CLOCK_MODE_OUTPUT_E:
            dxChClockMode = CPSS_DXCH_PTP_TAI_CLOCK_MODE_OUTPUT_E;
            break;
        case PRV_TGF_PTP_TAI_CLOCK_MODE_INPUT_E:
            dxChClockMode = CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_E;
            break;
        case PRV_TGF_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E:
            dxChClockMode = CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiClockModeSet(
            devNum, &dxchTaiId, dxChClockMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiClockModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfClockMode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] tgfClockModePtr          - (pointer to) clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockModeGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TAI_CLOCK_MODE_ENT          *tgfClockModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT           dxChClockMode;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiClockModeGet(
        devNum, &dxchTaiId, &dxChClockMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiClockModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    switch (dxChClockMode)
    {
        case CPSS_DXCH_PTP_TAI_CLOCK_MODE_DISABLED_E:
            *tgfClockModePtr = PRV_TGF_PTP_TAI_CLOCK_MODE_DISABLED_E;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_MODE_OUTPUT_E:
            *tgfClockModePtr = PRV_TGF_PTP_TAI_CLOCK_MODE_OUTPUT_E;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_E:
            *tgfClockModePtr = PRV_TGF_PTP_TAI_CLOCK_MODE_INPUT_E;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E:
            *tgfClockModePtr = PRV_TGF_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E;
            break;
        default: return GT_BAD_STATE;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfClockModePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiInternalClockGenerateEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Internal Clock Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock signal
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInternalClockGenerateEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiInternalClockGenerateEnableSet(
            devNum, &dxchTaiId, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiInternalClockGenerateEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiInternalClockGenerateEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Internal Clock Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInternalClockGenerateEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiInternalClockGenerateEnableGet(
        devNum, &dxchTaiId, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiInternalClockGenerateEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiPtpPClockDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPtpPClockDriftAdjustEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiPtpPClockDriftAdjustEnableSet(
            devNum, &dxchTaiId, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiPtpPClockDriftAdjustEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiPtpPClockDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiPtpPClockDriftAdjustEnableGet(
        devNum, &dxchTaiId, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiPtpPClockDriftAdjustEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiCaptureOverrideEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Override Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiCaptureOverrideEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiCaptureOverrideEnableSet(
            devNum, &dxchTaiId, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiCaptureOverrideEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiCaptureOverrideEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Override Enable
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiCaptureOverrideEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiCaptureOverrideEnableGet(
        devNum, &dxchTaiId, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiCaptureOverrideEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see prvTgfPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInputTriggersCountEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiInputTriggersCountEnableSet(
            devNum, &dxchTaiId, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiInputTriggersCountEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see prvTgfPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInputTriggersCountEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiInputTriggersCountEnableGet(
        devNum, &dxchTaiId, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiInputTriggersCountEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiExternalPulseWidthSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_U32                                  extPulseWidth
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiExternalPulseWidthSet(
            devNum, &dxchTaiId, extPulseWidth);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiExternalPulseWidthSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(extPulseWidth);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiExternalPulseWidthGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_U32                                  *extPulseWidthPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiExternalPulseWidthGet(
        devNum, &dxchTaiId, extPulseWidthPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiExternalPulseWidthGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(extPulseWidthPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfTodValueType          - type of TOD value.
*                                      Valid types are
*                                      PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      PRV_TGF_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] tgfTodValuePtr           - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
GT_STATUS prvTgfPtpTaiTodSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_TOD_TYPE_ENT        tgfTodValueType,
    IN  PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT             dxchTodValueType;
    CPSS_DXCH_PTP_TOD_COUNT_STC                dxchTodValue;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TAI_TOD_TYPE_ENT(tgfTodValueType, &dxchTodValueType);
    if (rc != GT_OK)
    {
        return rc;
    }

    dxchTodValue.seconds         = tgfTodValuePtr->seconds;
    dxchTodValue.nanoSeconds     = tgfTodValuePtr->nanoSeconds;
    dxchTodValue.fracNanoSeconds = tgfTodValuePtr->fracNanoSeconds;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiTodSet(
            devNum, &dxchTaiId, dxchTodValueType, &dxchTodValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfTodValueType);
    TGF_PARAM_NOT_USED(tgfTodValuePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
* @param[in] tgfTodValueType          - type of TOD value.
*
* @param[out] tgfTodValuePtr           - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_TOD_TYPE_ENT        tgfTodValueType,
    OUT PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT             dxchTodValueType;
    CPSS_DXCH_PTP_TOD_COUNT_STC                dxchTodValue;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TAI_TOD_TYPE_ENT(tgfTodValueType, &dxchTodValueType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiTodGet(
        devNum, &dxchTaiId, dxchTodValueType, &dxchTodValue);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfTodValuePtr->seconds         = dxchTodValue.seconds;
    tgfTodValuePtr->nanoSeconds     = dxchTodValue.nanoSeconds;
    tgfTodValuePtr->fracNanoSeconds = dxchTodValue.fracNanoSeconds;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfTodValueType);
    TGF_PARAM_NOT_USED(tgfTodValuePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use prvTgfPtpTaiTodSet with
*       PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
GT_STATUS prvTgfPtpTaiOutputTriggerEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  GT_BOOL                             enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiOutputTriggerEnableSet(
            devNum, &dxchTaiId, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiOutputTriggerEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiOutputTriggerEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    OUT GT_BOOL                             *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiOutputTriggerEnableGet(
        devNum, &dxchTaiId, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiOutputTriggerEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfTodStepPtr            - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodStepSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_TOD_STEP_STC    *tgfTodStepPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC             dxchTodStep;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    dxchTodStep.nanoSeconds     = tgfTodStepPtr->nanoSeconds;
    dxchTodStep.fracNanoSeconds = tgfTodStepPtr->fracNanoSeconds;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiTodStepSet(
            devNum, &dxchTaiId, &dxchTodStep);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodStepSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfTodStepPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] tgfTodStepPtr            - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodStepGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TAI_TOD_STEP_STC    *tgfTodStepPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC             dxchTodStep;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiTodStepGet(
        devNum, &dxchTaiId, &dxchTodStep);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodStepGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfTodStepPtr->nanoSeconds     = dxchTodStep.nanoSeconds;
    tgfTodStepPtr->fracNanoSeconds = dxchTodStep.fracNanoSeconds;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(tgfTodStepPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiPulseDelaySet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Trigger Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] nanoSeconds              - nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPulseDelaySet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          nanoSeconds
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiPulseDelaySet(
            devNum, &dxchTaiId, nanoSeconds);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiPulseDelaySet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(nanoSeconds);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiPulseDelayGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Trigger Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPulseDelayGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *nanoSecondsPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiPulseDelayGet(
        devNum, &dxchTaiId, nanoSecondsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiPulseDelayGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(nanoSecondsPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiClockDelaySet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Clock Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] nanoSeconds              - nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockDelaySet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          nanoSeconds
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiClockDelaySet(
            devNum, &dxchTaiId, nanoSeconds);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiClockDelaySet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(nanoSeconds);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiClockDelayGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Clock Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockDelayGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *nanoSecondsPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiClockDelayGet(
        devNum, &dxchTaiId, nanoSecondsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiClockDelayGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(nanoSecondsPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] fracNanoSecond           - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiFractionalNanosecondDriftSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_32                           fracNanoSecond
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiFractionalNanosecondDriftSet(
            devNum, &dxchTaiId, fracNanoSecond);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiFractionalNanosecondDriftSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(fracNanoSecond);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @param[out] fracNanoSecondPtr        - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiFractionalNanosecondDriftGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_32                           *fracNanoSecondPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiFractionalNanosecondDriftGet(
        devNum, &dxchTaiId, fracNanoSecondPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiFractionalNanosecondDriftGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(fracNanoSecondPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiPClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPClockCycleSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          nanoSeconds
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiPClockCycleSet(
            devNum, &dxchTaiId, nanoSeconds);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiPClockCycleSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(nanoSeconds);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiPClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPClockCycleGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *nanoSecondsPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiPClockCycleGet(
        devNum, &dxchTaiId, nanoSecondsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiPClockCycleGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(nanoSecondsPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockCycleSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          seconds,
    IN  GT_U32                          nanoSeconds
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiClockCycleSet(
            devNum, &dxchTaiId, seconds, nanoSeconds);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiClockCycleSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(seconds);
    TGF_PARAM_NOT_USED(nanoSeconds);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockCycleGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *secondsPtr,
    OUT GT_U32                          *nanoSecondsPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiClockCycleGet(
        devNum, &dxchTaiId, secondsPtr, nanoSecondsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiClockCycleGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(secondsPtr);
    TGF_PARAM_NOT_USED(nanoSecondsPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodCaptureStatusSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Capture Status.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] valid                    - TOD Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodCaptureStatusSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          captureIndex,
    IN  GT_BOOL                         valid
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiTodCaptureStatusSet(
            devNum, &dxchTaiId, captureIndex, valid);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodCaptureStatusSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(captureIndex);
    TGF_PARAM_NOT_USED(valid);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodCaptureStatusGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          captureIndex,
    OUT GT_BOOL                         *validPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiTodCaptureStatusGet(
        devNum, &dxchTaiId, captureIndex, validPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodCaptureStatusGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(captureIndex);
    TGF_PARAM_NOT_USED(validPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
GT_STATUS prvTgfPtpTaiTodUpdateCounterGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *valuePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiTodUpdateCounterGet(
        devNum, &dxchTaiId, valuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiTodUpdateCounterGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(valuePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiIncomingTriggerCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Trigger Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] value                    -  to set.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiIncomingTriggerCounterSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          value
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiIncomingTriggerCounterSet(
            devNum, &dxchTaiId, value);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiIncomingTriggerCounterSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(value);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiIncomingTriggerCounterGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *valuePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiIncomingTriggerCounterGet(
        devNum, &dxchTaiId, valuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiIncomingTriggerCounterGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(valuePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiIncomingClockCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Clock Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[in] value                    -  to set.
*                                      (APPLICABLE RANGES: 0..2^30-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiIncomingClockCounterSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_BOOL                         enable,
    IN  GT_U32                          value
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiIncomingClockCounterSet(
            devNum, &dxchTaiId, enable, value);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiIncomingClockCounterSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(value);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiIncomingClockCounterGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_BOOL                         *enablePtr,
    OUT GT_U32                          *valuePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiIncomingClockCounterGet(
        devNum, &dxchTaiId, enablePtr, valuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiIncomingClockCounterGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(valuePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiFrequencyDriftThesholdsSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Frequency Drift Thesholds.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] interruptThreshold       - the drift value threshold causing interrupt.
*                                      Notifies about the clock frequency out of sync.
*                                      (APPLICABLE RANGES: 0..2^24-1.)
* @param[in] adjustThreshold          - the drift value threshold that triggers
*                                      the adjustment logic. Prevents minor jitters
*                                      from affecting the drift adjustment.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiFrequencyDriftThesholdsSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          interruptThreshold,
    IN  GT_U32                          adjustThreshold
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTaiFrequencyDriftThesholdsSet(
            devNum, &dxchTaiId, interruptThreshold, adjustThreshold);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiFrequencyDriftThesholdsSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(interruptThreshold);
    TGF_PARAM_NOT_USED(adjustThreshold);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTaiFrequencyDriftThesholdsGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Frequency Drift Thesholds.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] interruptThresholdPtr    - (pointer to)the drift value threshold causing interrupt.
*                                      Notifies about the clock frequency out of sync.
*                                      (APPLICABLE RANGES: 0..2^24-1.)
* @param[out] adjustThresholdPtr       - (pointer to)the drift value threshold that triggers
*                                      the adjustment logic. Prevents minor jitters
*                                      from affecting the drift adjustment.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiFrequencyDriftThesholdsGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *interruptThresholdPtr,
    OUT GT_U32                          *adjustThresholdPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TAI_ID_STC                   dxchTaiId;

    rc = tgf2dxchPTP_TAI_ID_STC(tgfTaiIdPtr, &dxchTaiId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPtpTaiFrequencyDriftThesholdsGet(
        devNum, &dxchTaiId, interruptThresholdPtr, adjustThresholdPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTaiFrequencyDriftThesholdsGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTaiIdPtr);
    TGF_PARAM_NOT_USED(interruptThresholdPtr);
    TGF_PARAM_NOT_USED(adjustThresholdPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @param[in] portNum                  - port number.
* @param[in] tgfControlPtr            - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuControlSet
(
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  PRV_TGF_PTP_TSU_CONTROL_STC     *tgfControlPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TSU_CONTROL_STC              dxchControl;

    dxchControl.rxTaiSelect         = tgfControlPtr->rxTaiSelect;
    dxchControl.tsQueOverrideEnable = tgfControlPtr->tsQueOverrideEnable;
    dxchControl.unitEnable          = tgfControlPtr->unitEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsuControlSet(
            devNum, portNum, &dxchControl);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsuControlSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tgfControlPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @param[in] portNum                  - port number.
*
* @param[out] tgfControlPtr            - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuControlGet
(
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT PRV_TGF_PTP_TSU_CONTROL_STC       *tgfControlPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TSU_CONTROL_STC              dxchControl;

    /* call device specific API */
    rc = cpssDxChPtpTsuControlGet(
        devNum, portNum, &dxchControl);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsuControlGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfControlPtr->rxTaiSelect         = dxchControl.rxTaiSelect;
    tgfControlPtr->tsQueOverrideEnable = dxchControl.tsQueOverrideEnable;
    tgfControlPtr->unitEnable          = dxchControl.unitEnable;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tgfControlPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsuTxTimestampQueueRead function
* @endinternal
*
* @brief   Read TSU (Time Stamp Unit) TX Timestamp Queue Entry.
*
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Entry deleted from the Queue by reading it.
*
*/
GT_STATUS prvTgfPtpTsuTxTimestampQueueRead
(
    IN  GT_PHYSICAL_PORT_NUM                               portNum,
    IN  GT_U32                                             queueNum,
    OUT PRV_TGF_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC       *tgfEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                           devNum  = 0;
    GT_STATUS                                       rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC  dxchEntry;

    cpssOsMemSet(&dxchEntry, 0, sizeof(dxchEntry));

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsuTxTimestampQueueRead(
            devNum, portNum, queueNum, &dxchEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsuTxTimestampQueueRead FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        if (dxchEntry.entryValid != GT_FALSE)
        {
            break;
        }
    }

    tgfEntryPtr->entryValid    = dxchEntry.entryValid;
    tgfEntryPtr->entryId       = dxchEntry.entryId;
    tgfEntryPtr->taiSelect     = dxchEntry.taiSelect;
    tgfEntryPtr->todUpdateFlag = dxchEntry.todUpdateFlag;
    tgfEntryPtr->timestamp     = dxchEntry.timestamp;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(queueNum);
    TGF_PARAM_NOT_USED(tgfEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuCountersClear
(
    IN  GT_PHYSICAL_PORT_NUM              portNum
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsuCountersClear(
            devNum, portNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsuCountersClear FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsuPacketCouterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @param[in] portNum                  - port number.
* @param[in] tgfCounterType           - counter type (see enum)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuPacketCouterGet
(
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ENT   tgfCounterType,
    OUT GT_U32                                    *valuePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = 0;
    GT_STATUS                                 rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT dxchCounterType;
    GT_U32                                    counter;

    *valuePtr = 0; /* initial value */

    switch (tgfCounterType)
    {
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_Y1731_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_Y1731_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TS_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TS_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NTP_RX_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_RX_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TX_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TX_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_WAMP_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_WAMP_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NONE_ACTION_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NONE_ACTION_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_FORWARD_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_FORWARD_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_DROP_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_DROP_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ADD_TIME_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_TIME_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_TIME_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_TIME_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_CORR_TIME_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_CORR_TIME_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ADD_INGRESS_TIME_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_INGRESS_TIME_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_INGRESS_TIME_E;
            break;
        case PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_INGRESS_TIME_E:
            dxchCounterType = CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_INGRESS_TIME_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* summing counter value over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsuPacketCouterGet(
            devNum, portNum, dxchCounterType, &counter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsuPacketCouterGet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        *valuePtr += counter;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tgfCounterType);
    TGF_PARAM_NOT_USED(valuePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsuNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) NTP Time Offset.
*
* @param[in] portNum                  - port number.
* @param[in] ntpTimeOffset            - NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuNtpTimeOffsetSet
(
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_U32                            ntpTimeOffset
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsuNtpTimeOffsetSet(
            devNum, portNum, ntpTimeOffset);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsuNtpTimeOffsetSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ntpTimeOffset);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsuNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) NTP Time Offset.
*
* @param[in] portNum                  - port number.
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuNtpTimeOffsetGet
(
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsuNtpTimeOffsetGet(
        devNum, portNum, ntpTimeOffsetPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsuNtpTimeOffsetGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ntpTimeOffsetPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsTagGlobalCfgSet function
* @endinternal
*
* @brief   Set Timestamp Global Configuration.
*
* @param[in] tgfTsTagGlobalCfgPtr     - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagGlobalCfgSet
(
    IN  PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   *tgfTsTagGlobalCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC        dxchTsTagGlobalCfg;

    dxchTsTagGlobalCfg.tsTagParseEnable       = tgfTsTagGlobalCfgPtr->tsTagParseEnable;
    dxchTsTagGlobalCfg.hybridTsTagParseEnable = tgfTsTagGlobalCfgPtr->hybridTsTagParseEnable;
    dxchTsTagGlobalCfg.tsTagEtherType         = tgfTsTagGlobalCfgPtr->tsTagEtherType;
    dxchTsTagGlobalCfg.hybridTsTagEtherType   = tgfTsTagGlobalCfgPtr->hybridTsTagEtherType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsTagGlobalCfgSet(
            devNum, &dxchTsTagGlobalCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsTagGlobalCfgSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTsTagGlobalCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsTagGlobalCfgGet function
* @endinternal
*
* @brief   Get Timestamp Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagGlobalCfgGet
(
    OUT PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   *tgfTsTagGlobalCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC       dxchTsTagGlobalCfg;

    /* call device specific API */
    rc = cpssDxChPtpTsTagGlobalCfgGet(
        devNum, &dxchTsTagGlobalCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsTagGlobalCfgGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfTsTagGlobalCfgPtr->tsTagParseEnable       = dxchTsTagGlobalCfg.tsTagParseEnable;
    tgfTsTagGlobalCfgPtr->hybridTsTagParseEnable = dxchTsTagGlobalCfg.hybridTsTagParseEnable;
    tgfTsTagGlobalCfgPtr->tsTagEtherType         = dxchTsTagGlobalCfg.tsTagEtherType;
    tgfTsTagGlobalCfgPtr->hybridTsTagEtherType   = dxchTsTagGlobalCfg.hybridTsTagEtherType;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfTsTagGlobalCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsTagPortCfgSet function
* @endinternal
*
* @brief   Set Timestamp Port Configuration.
*
* @param[in] portNum                  - port number.
* @param[in] tgfTsTagPortCfgPtr       - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagPortCfgSet
(
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     *tgfTsTagPortCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC          dxchTagPortCfg;

    dxchTagPortCfg.tsReceptionEnable = tgfTsTagPortCfgPtr->tsReceptionEnable;
    dxchTagPortCfg.tsPiggyBackEnable = tgfTsTagPortCfgPtr->tsPiggyBackEnable;

    switch (tgfTsTagPortCfgPtr->tsTagMode)
    {
        case PRV_TGF_PTP_TS_TAG_MODE_NONE_E:
            dxchTagPortCfg.tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_NONE_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E:
            dxchTagPortCfg.tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E:
            dxchTagPortCfg.tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_ALL_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_NON_EXTENDED_E:
            dxchTagPortCfg.tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_NON_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_EXTENDED_E:
            dxchTagPortCfg.tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_EXTENDED_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E:
            dxchTagPortCfg.tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_PIGGYBACK_E;
            break;
        case PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E:
            dxchTagPortCfg.tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_HYBRID_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsTagPortCfgSet(
            devNum, portNum, &dxchTagPortCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsTagPortCfgSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tgfTsTagPortCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsTagPortCfgGet function
* @endinternal
*
* @brief   Get Timestamp Port Configuration.
*
* @param[in] portNum                  - port number.
*
* @param[out] tgfTsTagPortCfgPtr       - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagPortCfgGet
(
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     *tgfTsTagPortCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC         dxchTagPortCfg;

    /* call device specific API */
    rc = cpssDxChPtpTsTagPortCfgGet(
        devNum, portNum, &dxchTagPortCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsTagPortCfgGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfTsTagPortCfgPtr->tsReceptionEnable = dxchTagPortCfg.tsReceptionEnable;
    tgfTsTagPortCfgPtr->tsPiggyBackEnable = dxchTagPortCfg.tsPiggyBackEnable;

    switch (dxchTagPortCfg.tsTagMode)
    {
        case CPSS_DXCH_PTP_TS_TAG_MODE_NONE_E:
            tgfTsTagPortCfgPtr->tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_NONE_E;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E:
            tgfTsTagPortCfgPtr->tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_ALL_EXTENDED_E:
            tgfTsTagPortCfgPtr->tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_NON_EXTENDED_E:
            tgfTsTagPortCfgPtr->tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_NON_EXTENDED_E;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_EXTENDED_E:
            tgfTsTagPortCfgPtr->tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_EXTENDED_E;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_PIGGYBACK_E:
            tgfTsTagPortCfgPtr->tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_HYBRID_E:
            tgfTsTagPortCfgPtr->tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E;
            break;
        default: return GT_BAD_STATE;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tgfTsTagPortCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayIngressPortDelaySet function
* @endinternal
*
* @brief   Set Ingress Port Delay Correction.
*
* @param[in] portNum                  - port number.
* @param[in] ingressDelayCorr         - Ingress Port Delay Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressPortDelaySet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_32                   ingressDelayCorr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDelayIngressPortDelaySet(
            devNum, portNum, ingressDelayCorr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayIngressPortDelaySet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ingressDelayCorr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayIngressPortDelayGet function
* @endinternal
*
* @brief   Get Port Delay Correction Configuration.
*
* @param[in] portNum                  - port number.
*
* @param[out] ingressDelayCorrPtr      - (pointer to) Ingress Port Delay Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressPortDelayGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *ingressDelayCorrPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsDelayIngressPortDelayGet(
        devNum, portNum, ingressDelayCorrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayIngressPortDelayGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ingressDelayCorrPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressPipeDelaySet function
* @endinternal
*
* @brief   Set Egress Pipe Delay Correction.
*
* @param[in] portNum                  - port number.
* @param[in] egrPipeDelayCorr         - Egress Pipe Delay Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressPipeDelaySet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_32                   egrPipeDelayCorr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDelayEgressPipeDelaySet(
            devNum, portNum, egrPipeDelayCorr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressPipeDelaySet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egrPipeDelayCorr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressPipeDelayGet function
* @endinternal
*
* @brief   Get Egress Pipe Delay Correction.
*
* @param[in] portNum                  - port number.
*
* @param[out] egrPipeDelayCorrPtr      - (pointer to) Egress Pipe Delay Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressPipeDelayGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *egrPipeDelayCorrPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsDelayEgressPipeDelayGet(
        devNum, portNum, egrPipeDelayCorrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressPipeDelayGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egrPipeDelayCorrPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressAsymmetryCorrectionSet function
* @endinternal
*
* @brief   Set Egress Asymmetry Correction.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] egrAsymmetryCorr         - Egress Asymmetry Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressAsymmetryCorrectionSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainNum,
    IN  GT_32                   egrAsymmetryCorr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet(
            devNum, portNum, domainNum, egrAsymmetryCorr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(egrAsymmetryCorr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressAsymmetryCorrectionGet function
* @endinternal
*
* @brief   Get Egress Asymmetry Correction.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] egrAsymmetryCorrPtr      - (pointer to) Egress Asymmetry Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressAsymmetryCorrectionGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainNum,
    OUT GT_32                   *egrAsymmetryCorrPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet(
        devNum, portNum, domainNum, egrAsymmetryCorrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(egrAsymmetryCorrPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Ingress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
* @param[in] ingrCorrFldPBEnable      - Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 ingrCorrFldPBEnable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet(
            devNum, portNum, ingrCorrFldPBEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ingrCorrFldPBEnable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Ingress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
*
* @param[out] ingrCorrFldPBEnablePtr   - (pointer to) Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *ingrCorrFldPBEnablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet(
        devNum, portNum, ingrCorrFldPBEnablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ingrCorrFldPBEnablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Egress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
* @param[in] egrCorrFldPBEnable       - Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 egrCorrFldPBEnable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet(
            devNum, portNum, egrCorrFldPBEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egrCorrFldPBEnable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Egress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
*
* @param[out] egrCorrFldPBEnablePtr    - (pointer to) Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *egrCorrFldPBEnablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet(
        devNum, portNum, egrCorrFldPBEnablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egrCorrFldPBEnablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeSet function
* @endinternal
*
* @brief   Set Egress Time Correction TAI Select Mode.
*
* @param[in] portNum                  - port number.
* @param[in] tgfEgrTimeCorrTaiSelMode - Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeSet
(
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT     tgfEgrTimeCorrTaiSelMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT    dxchEgrTimeCorrTaiSelMode;

    switch (tgfEgrTimeCorrTaiSelMode)
    {
        case PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E:
            dxchEgrTimeCorrTaiSelMode = CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E;
            break;
        case PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E:
            dxchEgrTimeCorrTaiSelMode = CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet(
            devNum, portNum, dxchEgrTimeCorrTaiSelMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tgfEgrTimeCorrTaiSelMode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeGet function
* @endinternal
*
* @brief   Get Egress Time Correction TAI Select Mode.
*
* @param[in] portNum                  - port number.
*
* @param[out] tgfEgrTimeCorrTaiSelModePtr - (pointer to)Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeGet
(
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    OUT PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT     *tgfEgrTimeCorrTaiSelModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   dxchEgrTimeCorrTaiSelMode;

    /* call device specific API */
    rc = cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet(
        devNum, portNum, &dxchEgrTimeCorrTaiSelMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    switch (dxchEgrTimeCorrTaiSelMode)
    {
        case CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E:
            *tgfEgrTimeCorrTaiSelModePtr = PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E;
            break;
        case CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E:
            *tgfEgrTimeCorrTaiSelModePtr = PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E;
            break;
        default: return GT_BAD_STATE;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tgfEgrTimeCorrTaiSelModePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressTimestampTaiSelectSet function
* @endinternal
*
* @brief   Set Egress Timestamp TAI Select.
*
* @param[in] portNum                  - port number.
* @param[in] egrTsTaiNum              - Egress Timestamp TAI Number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimestampTaiSelectSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  egrTsTaiNum
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDelayEgressTimestampTaiSelectSet(
            devNum, portNum, egrTsTaiNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressTimestampTaiSelectSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egrTsTaiNum);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDelayEgressTimestampTaiSelectGet function
* @endinternal
*
* @brief   Get Egress Timestamp TAI Select.
*
* @param[in] portNum                  - port number.
*
* @param[out] egrTsTaiNumPtr           - (pointer to)Egress Timestamp TAI Number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimestampTaiSelectGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *egrTsTaiNumPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsDelayEgressTimestampTaiSelectGet(
        devNum, portNum, egrTsTaiNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDelayEgressTimestampTaiSelectGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egrTsTaiNumPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsCfgTableSet function
* @endinternal
*
* @brief   Set Timestamp Configuration Table Entry.
*
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
* @param[in] tgfEntryPtr              - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsCfgTableSet
(
    IN  GT_U32                         entryIndex,
    IN  PRV_TGF_PTP_TS_CFG_ENTRY_STC   *tgfEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_CFG_ENTRY_STC             dxchEntry;

    switch (tgfEntryPtr->tsMode)
    {
        case PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E:
            dxchEntry.tsMode = CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
            break;
        case PRV_TGF_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E:
            dxchEntry.tsMode = CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E;
            break;
        default: return GT_BAD_PARAM;
    }


    dxchEntry.offsetProfile = tgfEntryPtr->offsetProfile;
    dxchEntry.OE            = tgfEntryPtr->OE;

    rc = tgf2dxchPTP_TS_ACTION_ENT(
        tgfEntryPtr->tsAction, &(dxchEntry.tsAction));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TS_PACKET_TYPE_ENT(
        tgfEntryPtr->packetFormat, &(dxchEntry.packetFormat));
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (tgfEntryPtr->ptpTransport)
    {
        case PRV_TGF_PTP_TRANSPORT_TYPE_ETHERNET_E:
            dxchEntry.ptpTransport = CPSS_DXCH_PTP_TRANSPORT_TYPE_ETHERNET_E;
            break;
        case PRV_TGF_PTP_TRANSPORT_TYPE_UDP_IPV4_E:
            dxchEntry.ptpTransport = CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV4_E;
            break;
        case PRV_TGF_PTP_TRANSPORT_TYPE_UDP_IPV6_E:
            dxchEntry.ptpTransport = CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV6_E;
            break;
        default: return GT_BAD_PARAM;
    }

    dxchEntry.offset                  = tgfEntryPtr->offset;
    dxchEntry.ptpMessageType          = tgfEntryPtr->ptpMessageType;
    dxchEntry.domain                  = tgfEntryPtr->domain;
    dxchEntry.ingrLinkDelayEnable     = tgfEntryPtr->ingrLinkDelayEnable;
    dxchEntry.packetDispatchingEnable = tgfEntryPtr->packetDispatchingEnable;
    dxchEntry.offset2 = 0; /* TBD */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsCfgTableSet(
            devNum, entryIndex, &dxchEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsCfgTableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(tgfEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsCfgTableGet function
* @endinternal
*
* @brief   Get Timestamp Configuration Table Entry.
*
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
*
* @param[out] tgfEntryPtr              - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsCfgTableGet
(
    IN  GT_U32                         entryIndex,
    OUT PRV_TGF_PTP_TS_CFG_ENTRY_STC   *tgfEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_CFG_ENTRY_STC            dxchEntry;

    /* call device specific API */
    rc = cpssDxChPtpTsCfgTableGet(
        devNum, entryIndex, &(dxchEntry));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsCfgTableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    switch (dxchEntry.tsMode)
    {
        case CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E:
            tgfEntryPtr->tsMode = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
            break;
        case CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E:
            tgfEntryPtr->tsMode = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E;
            break;
        default: return GT_BAD_STATE;
    }

    tgfEntryPtr->offsetProfile = dxchEntry.offsetProfile;
    tgfEntryPtr->OE            = dxchEntry.OE;

    rc = dxch2tgfPTP_TS_ACTION_ENT(
        dxchEntry.tsAction, &(tgfEntryPtr->tsAction));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = dxch2tgfPTP_TS_PACKET_TYPE_ENT(
        dxchEntry.packetFormat, &(tgfEntryPtr->packetFormat));
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (dxchEntry.ptpTransport)
    {
        case CPSS_DXCH_PTP_TRANSPORT_TYPE_ETHERNET_E:
            tgfEntryPtr->ptpTransport = PRV_TGF_PTP_TRANSPORT_TYPE_ETHERNET_E;
            break;
        case CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV4_E:
            tgfEntryPtr->ptpTransport = PRV_TGF_PTP_TRANSPORT_TYPE_UDP_IPV4_E;
            break;
        case CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV6_E:
            tgfEntryPtr->ptpTransport = PRV_TGF_PTP_TRANSPORT_TYPE_UDP_IPV6_E;
            break;
        default: return GT_BAD_STATE;
    }


    tgfEntryPtr->offset                  = dxchEntry.offset;
    tgfEntryPtr->ptpMessageType          = dxchEntry.ptpMessageType;
    tgfEntryPtr->domain                  = dxchEntry.domain;
    tgfEntryPtr->ingrLinkDelayEnable     = dxchEntry.ingrLinkDelayEnable;
    tgfEntryPtr->packetDispatchingEnable = dxchEntry.packetDispatchingEnable;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(tgfEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsLocalActionTableSet function
* @endinternal
*
* @brief   Set Timestamp Local Action Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] tgfEntryPtr              - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsLocalActionTableSet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    IN  GT_U32                                  messageType,
    IN  PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC   *tgfEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC    dxchEntry;

    rc = tgf2dxchPTP_TS_ACTION_ENT(
        tgfEntryPtr->tsAction, &(dxchEntry.tsAction));
    if (rc != GT_OK)
    {
        return rc;
    }

    dxchEntry.ingrLinkDelayEnable     = tgfEntryPtr->ingrLinkDelayEnable;
    dxchEntry.packetDispatchingEnable = tgfEntryPtr->packetDispatchingEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsLocalActionTableSet(
            devNum, portNum, domainNum, messageType, &dxchEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsLocalActionTableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(tgfEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsLocalActionTableGet function
* @endinternal
*
* @brief   Get Timestamp Local Action Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] tgfEntryPtr              - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsLocalActionTableGet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    IN  GT_U32                                  messageType,
    OUT PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC   *tgfEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                     devNum  = prvTgfDevNum;
    GT_STATUS                                 rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC   dxchEntry;

    /* call device specific API */
    rc = cpssDxChPtpTsLocalActionTableGet(
        devNum, portNum, domainNum, messageType, &(dxchEntry));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsLocalActionTableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    rc = dxch2tgfPTP_TS_ACTION_ENT(
        dxchEntry.tsAction, &(tgfEntryPtr->tsAction));
    if (rc != GT_OK)
    {
        return rc;
    }

    tgfEntryPtr->ingrLinkDelayEnable     = dxchEntry.ingrLinkDelayEnable;
    tgfEntryPtr->packetDispatchingEnable = dxchEntry.packetDispatchingEnable;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(tgfEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsUdpChecksumUpdateModeSet function
* @endinternal
*
* @brief   Set Timestamp UDP Checksum Update Mode.
*
* @param[in] tgfUdpCsUpdModePtr       - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsUdpChecksumUpdateModeSet
(
    IN  PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *tgfUdpCsUpdModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC dxchUdpCsUpdMode;

    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        tgfUdpCsUpdModePtr->ptpIpv4Mode, &(dxchUdpCsUpdMode.ptpIpv4Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        tgfUdpCsUpdModePtr->ptpIpv6Mode, &(dxchUdpCsUpdMode.ptpIpv6Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        tgfUdpCsUpdModePtr->ntpIpv4Mode, &(dxchUdpCsUpdMode.ntpIpv4Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        tgfUdpCsUpdModePtr->ntpIpv6Mode, &(dxchUdpCsUpdMode.ntpIpv6Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        tgfUdpCsUpdModePtr->wampIpv4Mode, &(dxchUdpCsUpdMode.wampIpv4Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = tgf2dxchPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        tgfUdpCsUpdModePtr->wampIpv6Mode, &(dxchUdpCsUpdMode.wampIpv6Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsUdpChecksumUpdateModeSet(
            devNum, &dxchUdpCsUpdMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsUdpChecksumUpdateModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfUdpCsUpdModePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsUdpChecksumUpdateModeGet function
* @endinternal
*
* @brief   Get Timestamp UDP Checksum Update Mode.
*
* @param[out] tgfUdpCsUpdModePtr       - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsUdpChecksumUpdateModeGet
(
    OUT PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *tgfUdpCsUpdModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC dxchUdpCsUpdMode;

    /* call device specific API */
    rc = cpssDxChPtpTsUdpChecksumUpdateModeGet(
        devNum, &dxchUdpCsUpdMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsUdpChecksumUpdateModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    rc = dxch2tgfPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        dxchUdpCsUpdMode.ptpIpv4Mode, &(tgfUdpCsUpdModePtr->ptpIpv4Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = dxch2tgfPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        dxchUdpCsUpdMode.ptpIpv6Mode, &(tgfUdpCsUpdModePtr->ptpIpv6Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = dxch2tgfPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        dxchUdpCsUpdMode.ntpIpv4Mode, &(tgfUdpCsUpdModePtr->ntpIpv4Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = dxch2tgfPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        dxchUdpCsUpdMode.ntpIpv6Mode, &(tgfUdpCsUpdModePtr->ntpIpv6Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = dxch2tgfPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        dxchUdpCsUpdMode.wampIpv4Mode, &(tgfUdpCsUpdModePtr->wampIpv4Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = dxch2tgfPTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT(
        dxchUdpCsUpdMode.wampIpv6Mode, &(tgfUdpCsUpdModePtr->wampIpv6Mode));
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfUdpCsUpdModePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpOverEthernetEnableSet function
* @endinternal
*
* @brief   Set enable PTP over Ethernet packet.
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverEthernetEnableSet
(
    IN  GT_BOOL   enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpOverEthernetEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpOverEthernetEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpOverEthernetEnableGet function
* @endinternal
*
* @brief   Get enable PTP over Ethernet packet.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverEthernetEnableGet
(
    OUT GT_BOOL  *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpOverEthernetEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpOverEthernetEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpOverUdpEnableSet function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverUdpEnableSet
(
    IN  GT_BOOL   enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpOverUdpEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpOverUdpEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpOverUdpEnableGet function
* @endinternal
*
* @brief   Get enable PTP over UDP packet.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverUdpEnableGet
(
    OUT GT_BOOL  *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpOverUdpEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpOverUdpEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpDomainModeSet function
* @endinternal
*
* @brief   Set PTP domain mode.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] tgfDomainMode            - domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpDomainModeSet
(
    IN  GT_U32                                 domainNum,
    IN  PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT    tgfDomainMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT         dxchDomainMode;

    switch (tgfDomainMode)
    {
        case PRV_TGF_PTP_INGRESS_DOMAIN_MODE_DISABLE_E:
            dxchDomainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E;
            break;
        case PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E:
            dxchDomainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E;
            break;
        case PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E:
            dxchDomainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpDomainModeSet(
            devNum, domainNum, dxchDomainMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpDomainModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(tgfDomainMode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpDomainModeGet function
* @endinternal
*
* @brief   Get PTP domain mode.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] tgfDomainModePtr         - (pointer to) domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpDomainModeGet
(
    IN  GT_U32                                 domainNum,
    OUT PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT    *tgfDomainModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT         dxchDomainMode;

    /* call device specific API */
    rc = cpssDxChPtpDomainModeGet(
        devNum, domainNum, &dxchDomainMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpDomainModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    switch (dxchDomainMode)
    {
        case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E:
            *tgfDomainModePtr = PRV_TGF_PTP_INGRESS_DOMAIN_MODE_DISABLE_E;
            break;
        case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E:
            *tgfDomainModePtr = PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E;
            break;
        case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E:
            *tgfDomainModePtr = PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
            break;
        default: return GT_BAD_STATE;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(tgfDomainModePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpDomainV1IdSet function
* @endinternal
*
* @brief   Set PTP V1 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV1IdSet
(
    IN  GT_U32       domainNum,
    IN  GT_U32       domainIdArr[4]
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpDomainV1IdSet(
            devNum, domainNum, domainIdArr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpDomainV1IdSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(domainIdArr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpDomainV1IdGet function
* @endinternal
*
* @brief   Get PTP V1 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV1IdGet
(
    IN  GT_U32       domainNum,
    OUT GT_U32       domainIdArr[4]
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpDomainV1IdGet(
        devNum, domainNum, domainIdArr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpDomainV1IdGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(domainIdArr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpDomainV2IdSet function
* @endinternal
*
* @brief   Set PTP V2 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainId                 - domain Id
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV2IdSet
(
    IN  GT_U32       domainNum,
    IN  GT_U32       domainId
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpDomainV2IdSet(
            devNum, domainNum, domainId);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpDomainV2IdSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(domainId);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpDomainV2IdGet function
* @endinternal
*
* @brief   Get PTP V2 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdPtr              - (pointer to) domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV2IdGet
(
    IN  GT_U32       domainNum,
    OUT GT_U32       *domainIdPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpDomainV2IdGet(
        devNum, domainNum, domainIdPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpDomainV2IdGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(domainIdPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpEgressDomainTableSet function
* @endinternal
*
* @brief   Set Egress Domain Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] tgfEntryPtr              - (pointer to) Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressDomainTableSet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    IN  PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC     *tgfEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC         dxchEntry;

    dxchEntry.ptpOverEhernetTsEnable       = tgfEntryPtr->ptpOverEhernetTsEnable;
    dxchEntry.ptpOverUdpIpv4TsEnable       = tgfEntryPtr->ptpOverUdpIpv4TsEnable;
    dxchEntry.ptpOverUdpIpv6TsEnable       = tgfEntryPtr->ptpOverUdpIpv6TsEnable;
    dxchEntry.messageTypeTsEnableBmp       = tgfEntryPtr->messageTypeTsEnableBmp;
    dxchEntry.transportSpecificTsEnableBmp = tgfEntryPtr->transportSpecificTsEnableBmp;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpEgressDomainTableSet(
            devNum, portNum, domainNum, &dxchEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpEgressDomainTableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(tgfEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpEgressDomainTableGet function
* @endinternal
*
* @brief   Get Egress Domain Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..4.)
*
* @param[out] tgfEntryPtr              - (pointer to) Egress Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressDomainTableGet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    OUT PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC     *tgfEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC         dxchEntry;

    /* call device specific API */
    rc = cpssDxChPtpEgressDomainTableGet(
        devNum, portNum, domainNum, &dxchEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpEgressDomainTableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfEntryPtr->ptpOverEhernetTsEnable       = dxchEntry.ptpOverEhernetTsEnable;
    tgfEntryPtr->ptpOverUdpIpv4TsEnable       = dxchEntry.ptpOverUdpIpv4TsEnable;
    tgfEntryPtr->ptpOverUdpIpv6TsEnable       = dxchEntry.ptpOverUdpIpv6TsEnable;
    tgfEntryPtr->messageTypeTsEnableBmp       = dxchEntry.messageTypeTsEnableBmp;
    tgfEntryPtr->transportSpecificTsEnableBmp = dxchEntry.transportSpecificTsEnableBmp;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(tgfEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpIngressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Exception Configuration.
*
* @param[in] tgfIngrExceptionCfgPtr   - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressExceptionCfgSet
(
    IN  PRV_TGF_PTP_INGRESS_EXCEPTION_CFG_STC   *tgfIngrExceptionCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC       dxchIngrExceptionCfg;

    dxchIngrExceptionCfg.ptpExceptionCommandEnable = tgfIngrExceptionCfgPtr->ptpExceptionCommandEnable;
    dxchIngrExceptionCfg.ptpExceptionCommand       = tgfIngrExceptionCfgPtr->ptpExceptionCommand;
    dxchIngrExceptionCfg.ptpExceptionCpuCode       = tgfIngrExceptionCfgPtr->ptpExceptionCpuCode;
    dxchIngrExceptionCfg.ptpVersionCheckEnable     = tgfIngrExceptionCfgPtr->ptpVersionCheckEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpIngressExceptionCfgSet(
            devNum, &dxchIngrExceptionCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpIngressExceptionCfgSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfIngrExceptionCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpIngressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Exception Configuration.
*
* @param[out] tgfIngrExceptionCfgPtr   - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressExceptionCfgGet
(
    OUT PRV_TGF_PTP_INGRESS_EXCEPTION_CFG_STC   *tgfIngrExceptionCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC       dxchIngrExceptionCfg;

    /* call device specific API */
    rc = cpssDxChPtpIngressExceptionCfgGet(
        devNum, &dxchIngrExceptionCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpIngressExceptionCfgGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfIngrExceptionCfgPtr->ptpExceptionCommandEnable = dxchIngrExceptionCfg.ptpExceptionCommandEnable;
    tgfIngrExceptionCfgPtr->ptpExceptionCommand       = dxchIngrExceptionCfg.ptpExceptionCommand;
    tgfIngrExceptionCfgPtr->ptpExceptionCpuCode       = dxchIngrExceptionCfg.ptpExceptionCpuCode;
    tgfIngrExceptionCfgPtr->ptpVersionCheckEnable     = dxchIngrExceptionCfg.ptpVersionCheckEnable;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfIngrExceptionCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpIngressPacketCheckingModeSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Checking Mode.
*
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] tgfCheckingMode          - PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressPacketCheckingModeSet
(
    IN  GT_U32                                   domainNum,
    IN  GT_U32                                   messageType,
    IN  PRV_TGF_PTP_INGRESS_CHECKING_MODE_ENT    tgfCheckingMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT       dxchCheckingMode;

    switch (tgfCheckingMode)
    {
        case PRV_TGF_PTP_INGRESS_CHECKING_MODE_BASIC_E:
            dxchCheckingMode = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BASIC_E;
            break;
        case PRV_TGF_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E:
            dxchCheckingMode = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E;
            break;
        case PRV_TGF_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E:
            dxchCheckingMode = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E;
            break;
        case PRV_TGF_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E:
            dxchCheckingMode = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E;
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpIngressPacketCheckingModeSet(
            devNum, domainNum, messageType, dxchCheckingMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpIngressPacketCheckingModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(tgfCheckingMode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpIngressPacketCheckingModeGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Checking Mode.
*
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] tgfCheckingModePtr       - (pointer to)PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressPacketCheckingModeGet
(
    IN  GT_U32                                   domainNum,
    IN  GT_U32                                   messageType,
    OUT PRV_TGF_PTP_INGRESS_CHECKING_MODE_ENT    *tgfCheckingModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT       dxchCheckingMode;

    /* call device specific API */
    rc = cpssDxChPtpIngressPacketCheckingModeGet(
        devNum, domainNum, messageType, &dxchCheckingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpIngressPacketCheckingModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    switch (dxchCheckingMode)
    {
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BASIC_E:
            *tgfCheckingModePtr = PRV_TGF_PTP_INGRESS_CHECKING_MODE_BASIC_E;
            break;
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E:
            *tgfCheckingModePtr = PRV_TGF_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E;
            break;
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E:
            *tgfCheckingModePtr = PRV_TGF_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E;
            break;
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E:
            *tgfCheckingModePtr = PRV_TGF_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E;
            break;
        default: return GT_BAD_STATE;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(domainNum);
    TGF_PARAM_NOT_USED(messageType);
    TGF_PARAM_NOT_USED(tgfCheckingModePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpIngressExceptionCounterGet function
* @endinternal
*
* @brief   Get PTP Ingress Exception packet Counter.
*
* @param[out] counterPtr               - (pointer to) PTP Ingress Exception packet Counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is Clear On Read.
*       Returns the summary from all devices.
*
*/
GT_STATUS prvTgfPtpIngressExceptionCounterGet
(
    OUT GT_U32   *counterPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    GT_U32                                        counter;

    *counterPtr = 0; /* initial value */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpIngressExceptionCounterGet(
            devNum, &counter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpIngressExceptionCounterGet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        *counterPtr += counter;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(counterPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpEgressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Egress Exception Configuration.
*
* @param[in] tgfEgrExceptionCfgPtr    - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressExceptionCfgSet
(
    IN  PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC      *tgfEgrExceptionCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC        dxchEgrExceptionCfg;

    dxchEgrExceptionCfg.invalidPtpPktCmd          = tgfEgrExceptionCfgPtr->invalidPtpPktCmd;
    dxchEgrExceptionCfg.invalidOutPiggybackPktCmd = tgfEgrExceptionCfgPtr->invalidOutPiggybackPktCmd;
    dxchEgrExceptionCfg.invalidInPiggybackPktCmd  = tgfEgrExceptionCfgPtr->invalidInPiggybackPktCmd;
    dxchEgrExceptionCfg.invalidTsPktCmd           = tgfEgrExceptionCfgPtr->invalidTsPktCmd;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpEgressExceptionCfgSet(
            devNum, &dxchEgrExceptionCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpEgressExceptionCfgSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfEgrExceptionCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpEgressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Configuration.
*
* @param[out] tgfEgrExceptionCfgPtr    - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressExceptionCfgGet
(
    OUT PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC     *tgfEgrExceptionCfgPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC        dxchEgrExceptionCfg;

    /* call device specific API */
    rc = cpssDxChPtpEgressExceptionCfgGet(
        devNum, &dxchEgrExceptionCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpEgressExceptionCfgGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    tgfEgrExceptionCfgPtr->invalidPtpPktCmd          = dxchEgrExceptionCfg.invalidPtpPktCmd;
    tgfEgrExceptionCfgPtr->invalidOutPiggybackPktCmd = dxchEgrExceptionCfg.invalidOutPiggybackPktCmd;
    tgfEgrExceptionCfgPtr->invalidInPiggybackPktCmd  = dxchEgrExceptionCfg.invalidInPiggybackPktCmd;
    tgfEgrExceptionCfgPtr->invalidTsPktCmd           = dxchEgrExceptionCfg.invalidTsPktCmd;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfEgrExceptionCfgPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpEgressExceptionCountersGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Counters.
*
* @param[out] tgfEgrExceptionCntPtr    - (pointer to) PTP packet Egress Exception Counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
GT_STATUS prvTgfPtpEgressExceptionCountersGet
(
    OUT PRV_TGF_PTP_EGRESS_EXCEPTION_COUNTERS_STC     *tgfEgrExceptionCntPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC   dxchEgrExceptionCnt;

    /* initial value */
    cpssOsMemSet(tgfEgrExceptionCntPtr, 0, sizeof(*tgfEgrExceptionCntPtr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* summing over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpEgressExceptionCountersGet(
            devNum, &dxchEgrExceptionCnt);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpEgressExceptionCountersGet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        tgfEgrExceptionCntPtr->invalidPtpPktCnt          += dxchEgrExceptionCnt.invalidPtpPktCnt;
        tgfEgrExceptionCntPtr->invalidOutPiggybackPktCnt += dxchEgrExceptionCnt.invalidOutPiggybackPktCnt;
        tgfEgrExceptionCntPtr->invalidInPiggybackPktCnt  += dxchEgrExceptionCnt.invalidInPiggybackPktCnt;
        tgfEgrExceptionCntPtr->invalidTsPktCnt           += dxchEgrExceptionCnt.invalidTsPktCnt;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(tgfEgrExceptionCntPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsIngressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Queue Entry.
*
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tgfTsQueueEntryPtr       - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS prvTgfPtpTsIngressTimestampQueueEntryRead
(
    IN  GT_U32                                             queueNum,
    OUT PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *tgfTsQueueEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                               devNum  = 0;
    GT_STATUS                                           rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC  dxchTsQueueEntry;

    /* initial value */
    cpssOsMemSet(tgfTsQueueEntryPtr, 0, sizeof(*tgfTsQueueEntryPtr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsIngressTimestampQueueEntryRead(
            devNum, queueNum, &dxchTsQueueEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsIngressTimestampQueueEntryRead FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        if (dxchTsQueueEntry.entryValid != GT_FALSE)
        {
            break;
        }
    }

    /* reset the device iterator : to avoid "Error , reached ITERATION_MAX_DEPTH_CNS ... please check it" */
    utfExtraTestEnded();

    tgfTsQueueEntryPtr->entryValid    = dxchTsQueueEntry.entryValid;
    tgfTsQueueEntryPtr->isPtpExeption = dxchTsQueueEntry.isPtpExeption;

    rc = dxch2tgfPTP_TS_PACKET_TYPE_ENT(
        dxchTsQueueEntry.packetFormat, &(tgfTsQueueEntryPtr->packetFormat));
    if (rc != GT_OK)
    {

        PRV_UTF_LOG1_MAC("[TGF]: dxch2tgfPTP_TS_PACKET_TYPE_ENT FAILED, rc = [%d]\n", rc);
        return rc;
    }

    tgfTsQueueEntryPtr->taiSelect     = dxchTsQueueEntry.taiSelect;
    tgfTsQueueEntryPtr->todUpdateFlag = dxchTsQueueEntry.todUpdateFlag;
    tgfTsQueueEntryPtr->messageType   = dxchTsQueueEntry.messageType;
    tgfTsQueueEntryPtr->domainNum     = dxchTsQueueEntry.domainNum;
    tgfTsQueueEntryPtr->sequenceId    = dxchTsQueueEntry.sequenceId;
    tgfTsQueueEntryPtr->timestamp     = dxchTsQueueEntry.timestamp;
    tgfTsQueueEntryPtr->portNum       = dxchTsQueueEntry.portNum;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(queueNum);
    TGF_PARAM_NOT_USED(tgfTsQueueEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsEgressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry.
*
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tgfTsQueueEntryPtr       - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS prvTgfPtpTsEgressTimestampQueueEntryRead
(
    IN  GT_U32                                             queueNum,
    OUT PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *tgfTsQueueEntryPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                               devNum  = 0;
    GT_STATUS                                           rc, rc1 = GT_OK;
    CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC   dxchTsQueueEntry;

    /* initial value */
    cpssOsMemSet(tgfTsQueueEntryPtr, 0, sizeof(*tgfTsQueueEntryPtr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsEgressTimestampQueueEntryRead(
            devNum, queueNum, &dxchTsQueueEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsEgressTimestampQueueEntryRead FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        if (dxchTsQueueEntry.entryValid != GT_FALSE)
        {
            break;
        }
    }

    /* reset the device iterator : to avoid "Error , reached ITERATION_MAX_DEPTH_CNS ... please check it" */
    utfExtraTestEnded();

    tgfTsQueueEntryPtr->entryValid    = dxchTsQueueEntry.entryValid;
    tgfTsQueueEntryPtr->isPtpExeption = dxchTsQueueEntry.isPtpExeption;

    rc = dxch2tgfPTP_TS_PACKET_TYPE_ENT(
        dxchTsQueueEntry.packetFormat, &(tgfTsQueueEntryPtr->packetFormat));
    if (rc != GT_OK)
    {
        return rc;
    }

    tgfTsQueueEntryPtr->messageType   = dxchTsQueueEntry.messageType;
    tgfTsQueueEntryPtr->domainNum     = dxchTsQueueEntry.domainNum;
    tgfTsQueueEntryPtr->sequenceId    = dxchTsQueueEntry.sequenceId;
    tgfTsQueueEntryPtr->queueEntryId  = dxchTsQueueEntry.queueEntryId;
    tgfTsQueueEntryPtr->portNum       = dxchTsQueueEntry.portNum;

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(queueNum);
    TGF_PARAM_NOT_USED(tgfTsQueueEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsMessageTypeToQueueIdMapSet function
* @endinternal
*
* @brief   Set PTP Message Type To Queue Id Map.
*
* @param[in] idMapBmp                 - PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsMessageTypeToQueueIdMapSet
(
    IN  GT_U32       idMapBmp
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsMessageTypeToQueueIdMapSet(
            devNum, idMapBmp);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsMessageTypeToQueueIdMapSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(idMapBmp);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsMessageTypeToQueueIdMapGet function
* @endinternal
*
* @brief   Get PTP Message Type To Queue Id Map.
*
* @param[out] idMapBmpPtr              - (pointer to) PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsMessageTypeToQueueIdMapGet
(
    OUT GT_U32       *idMapBmpPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsMessageTypeToQueueIdMapGet(
        devNum, idMapBmpPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsMessageTypeToQueueIdMapGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(idMapBmpPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsQueuesEntryOverrideEnableSet function
* @endinternal
*
* @brief   Set Timestamp Queues Override Enable.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
* @param[in] enable                   - when the TS Queue is full the new timestamp
*                                      GT_TRUE  - overrides an oldest Queue entry.
*                                      GT_FALSE - not queued.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note prvTgfPtpTsuControlSet should configure the same value per port
*
*/
GT_STATUS prvTgfPtpTsQueuesEntryOverrideEnableSet
(
    IN  GT_BOOL     enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsQueuesEntryOverrideEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsQueuesEntryOverrideEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsQueuesEntryOverrideEnableGet function
* @endinternal
*
* @brief   Get Timestamp Queues Override Enable.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @param[out] enablePtr                - (pointer to)when the TS Queue is full the new timestamp
*                                      GT_TRUE  - overrides an oldest Queue entry.
*                                      GT_FALSE - not queued.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsQueuesEntryOverrideEnableGet
(
    OUT GT_BOOL     *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsQueuesEntryOverrideEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsQueuesEntryOverrideEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsQueuesSizeSet function
* @endinternal
*
* @brief   Set Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
* @param[in] queueSize                - The size of each PTP Queue.
*                                      (APPLICABLE RANGES: 0..256.)
*                                      If Timestamp Queues Override is Enable,
*                                      the size should be (maximal - 2) == 254.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsQueuesSizeSet
(
    IN  GT_U32      queueSize
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsQueuesSizeSet(
            devNum, queueSize);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsQueuesSizeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(queueSize);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsQueuesSizeGet function
* @endinternal
*
* @brief   Get Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @param[out] queueSizePtr             - (pointer to)The size of each PTP Queue.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsQueuesSizeGet
(
    OUT GT_U32      *queueSizePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsQueuesSizeGet(
        devNum, queueSizePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsQueuesSizeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(queueSizePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDebugQueuesEntryIdsClear function
* @endinternal
*
* @brief   Set Timestamp Queues Current Entry Ids Clear.
*         For Debug only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDebugQueuesEntryIdsClear
(
    GT_VOID
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsDebugQueuesEntryIdsClear(
            devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDebugQueuesEntryIdsClear FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsDebugQueuesEntryIdsGet function
* @endinternal
*
* @brief   Get Timestamp Queues Current Entry IDs.
*         For Debug only.
*
* @param[out] queueEntryId0Ptr         - (pointer to)The PTP Queue0 current entry Id.
* @param[out] queueEntryId1Ptr         - (pointer to)The PTP Queue1 current entry Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDebugQueuesEntryIdsGet
(
    OUT GT_U32      *queueEntryId0Ptr,
    OUT GT_U32      *queueEntryId1Ptr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsDebugQueuesEntryIdsGet(
        devNum, queueEntryId0Ptr, queueEntryId1Ptr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsDebugQueuesEntryIdsGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(queueEntryId0Ptr);
    TGF_PARAM_NOT_USED(queueEntryId1Ptr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TS NTP Time Offset.
*
* @param[in] ntpTimeOffset            - NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The same value should be configured per port.
*       See prvTgfPtpTsuNtpTimeOffsetSet.
*
*/
GT_STATUS prvTgfPtpTsNtpTimeOffsetSet
(
    IN  GT_U32                            ntpTimeOffset
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = 0;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPtpTsNtpTimeOffsetSet(
            devNum, ntpTimeOffset);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsNtpTimeOffsetSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(ntpTimeOffset);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPtpTsNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TS NTP Time Offset.
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsNtpTimeOffsetGet
(
    OUT GT_U32                            *ntpTimeOffsetPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
/* place for common local variables */
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    GT_U8                                         devNum  = prvTgfDevNum;
    GT_STATUS                                     rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPtpTsNtpTimeOffsetGet(
        devNum, ntpTimeOffsetPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPtpTsNtpTimeOffsetGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(ntpTimeOffsetPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}





