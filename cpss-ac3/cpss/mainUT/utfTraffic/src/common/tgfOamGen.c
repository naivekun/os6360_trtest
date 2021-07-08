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
* @file tgfOamGen.c
*
* @brief Generic API for OAM Engine API.
*
* @version   14
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
#include <common/tgfOamGen.h>


#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#endif /*CHX_FAMILY*/

#ifdef CHX_FAMILY
static GT_U32 prvTgfConvertGenericToDxChStageTypeArr[PRV_TGF_OAM_STAGE_TYPE_LAST_E] =
{
    CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E,
    CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E
};

static GT_U32 prvTgfConvertGenericToDxChExceptionTypeArr[PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E] =
{
    CPSS_DXCH_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
    CPSS_DXCH_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
    CPSS_DXCH_OAM_EXCEPTION_TYPE_RDI_STATUS_CHANGED_E,
    CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_AGING_E,
    CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
    CPSS_DXCH_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
    CPSS_DXCH_OAM_EXCEPTION_TYPE_TX_PERIOD_E,
    CPSS_DXCH_OAM_EXCEPTION_TYPE_SUMMARY_E
};

static GT_U32 prvTgfConvertGenericToDxChOpcodeTypeArr[PRV_TGF_OAM_OPCODE_TYPE_LAST_E] =
{
    CPSS_DXCH_OAM_OPCODE_TYPE_LM_COUNTED_E,
    CPSS_DXCH_OAM_OPCODE_TYPE_LM_SINGLE_ENDED_E,
    CPSS_DXCH_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E,
    CPSS_DXCH_OAM_OPCODE_TYPE_DM_E,
    CPSS_DXCH_OAM_OPCODE_TYPE_KEEPALIVE_E
};

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/**
* @internal prvTgfConvertGenericToDxChOpcodeType function
* @endinternal
*
* @brief   Convert generic into device specific OAM opcode type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChOpcodeType
(
    IN  PRV_TGF_OAM_OPCODE_TYPE_ENT              opcodeType,
    OUT CPSS_DXCH_OAM_OPCODE_TYPE_ENT            *dxChOpcodeTypePtr
)
{
    if(opcodeType >= PRV_TGF_OAM_OPCODE_TYPE_LAST_E)
    {
        return GT_BAD_PARAM;
    }

    /* convert opcode type into device specific format */
    *dxChOpcodeTypePtr = prvTgfConvertGenericToDxChOpcodeTypeArr[opcodeType];

    return GT_OK;
}
/**
* @internal prvTgfConvertGenericToDxChOamStage function
* @endinternal
*
* @brief   Convert generic into device specific OAM stage
*
* @param[out] dxChStagePtr             - (pointer to) DxCh OAM stage
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChOamStage
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT      policerStage,
    OUT CPSS_DXCH_OAM_STAGE_TYPE_ENT    *dxChStagePtr
)
{
    if(policerStage >= PRV_TGF_OAM_STAGE_TYPE_LAST_E)
    {
        return GT_BAD_PARAM;
    }
    /* convert stage into device specific format */
    *dxChStagePtr = prvTgfConvertGenericToDxChStageTypeArr[policerStage];

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChExceptionType function
* @endinternal
*
* @brief   Convert generic into device specific OAM Exception Type
*
* @param[in] exceptionType            - (pointer to) OAM Exception Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChExceptionType
(
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    OUT CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        *dxChExceptionTypePtr
)
{
    if(exceptionType >= PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E)
    {
        return GT_BAD_PARAM;
    }

    /* convert exception type into device specific format */
    *dxChExceptionTypePtr = prvTgfConvertGenericToDxChExceptionTypeArr[exceptionType];

    return GT_OK;
}


/**
* @internal prvTgfConvertGenericToDxChOamEntry function
* @endinternal
*
* @brief   Convert generic into device specific OAM entry
*
* @param[in] oamEntryPtr              - (pointer to) OAM entry
*
* @param[out] dxChOamEntryPtr          - (pointer to) DxCh OAM entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChOamEntry
(
    IN  PRV_TGF_OAM_ENTRY_STC    *oamEntryPtr,
    OUT CPSS_DXCH_OAM_ENTRY_STC  *dxChOamEntryPtr
)
{
    GT_STATUS rc;

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, opcodeParsingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, megLevelCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, megLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, packetCommandProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, cpuCodeOffset);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, sourceInterfaceCheckEnable);

    /* convert entry source interface check mode into device specific format */
    switch(oamEntryPtr->sourceInterfaceCheckMode)
    {
        case PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E:
            dxChOamEntryPtr->sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E;
            break;
        case PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E:
            dxChOamEntryPtr->sourceInterfaceCheckMode = CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert interface info into device specific format */
    dxChOamEntryPtr->sourceInterface.type = oamEntryPtr->sourceInterface.type;
    switch (dxChOamEntryPtr->sourceInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

            rc = prvUtfHwDeviceNumberGet((GT_U8)oamEntryPtr->sourceInterface.devPort.hwDevNum,
                                         &dxChOamEntryPtr->sourceInterface.devPort.hwDevNum);
            if(GT_OK != rc)
                return rc;
            dxChOamEntryPtr->sourceInterface.devPort.portNum = oamEntryPtr->sourceInterface.devPort.portNum;
            break;

        case CPSS_INTERFACE_TRUNK_E:
            dxChOamEntryPtr->sourceInterface.trunkId = oamEntryPtr->sourceInterface.trunkId;
            break;

        case CPSS_INTERFACE_VIDX_E:
            dxChOamEntryPtr->sourceInterface.vidx = oamEntryPtr->sourceInterface.vidx;
            break;

        case CPSS_INTERFACE_VID_E:
            dxChOamEntryPtr->sourceInterface.vlanId = oamEntryPtr->sourceInterface.vlanId;
            break;

        case CPSS_INTERFACE_DEVICE_E:
            dxChOamEntryPtr->sourceInterface.hwDevNum = oamEntryPtr->sourceInterface.hwDevNum;
            break;

        case CPSS_INTERFACE_FABRIC_VIDX_E:
            dxChOamEntryPtr->sourceInterface.fabricVidx = oamEntryPtr->sourceInterface.fabricVidx;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, lmCounterCaptureEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, dualEndedLmEnable);

    /* convert entry LM counting mode into device specific format */
    switch(oamEntryPtr->lmCountingMode)
    {
        case PRV_TGF_OAM_LM_COUNTING_MODE_RETAIN_E:
            dxChOamEntryPtr->lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_RETAIN_E;
            break;
        case PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E:
            dxChOamEntryPtr->lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_ENABLE_E;
            break;
        case PRV_TGF_OAM_LM_COUNTING_MODE_DISABLE_E:
            dxChOamEntryPtr->lmCountingMode = CPSS_DXCH_OAM_LM_COUNTING_MODE_DISABLE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, oamPtpOffsetIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, timestampEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, keepaliveAgingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, ageState);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, agingPeriodIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, agingThreshold);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, hashVerifyEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, lockHashValueEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, protectionLocUpdateEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, flowHash);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepaliveDetectionEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepalivePeriodCounter);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepalivePeriodThreshold);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepaliveMessageCounter);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepaliveMessageThreshold);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, rdiCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, rdiStatus);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, periodCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, keepaliveTxPeriod);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericOamEntry function
* @endinternal
*
* @brief   Convert generic from device specific OAM entry
*
* @param[in] dxChOamEntryPtr          - (pointer to) DxCh OAM entry parameters
*
* @param[out] oamEntryPtr              - (pointer to) OAM entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericOamEntry
(
    IN  CPSS_DXCH_OAM_ENTRY_STC  *dxChOamEntryPtr,
    OUT PRV_TGF_OAM_ENTRY_STC    *oamEntryPtr

)
{
    GT_STATUS rc;
    /* convert entry info from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, opcodeParsingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, megLevelCheckEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, megLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, packetCommandProfile);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, cpuCodeOffset);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, sourceInterfaceCheckEnable);

    /* convert entry source interface check mode into device specific format */
    switch(dxChOamEntryPtr->sourceInterfaceCheckMode)
    {
        case CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E:
            oamEntryPtr->sourceInterfaceCheckMode = PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E;
            break;
        case CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E:
            oamEntryPtr->sourceInterfaceCheckMode = PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert interface info from device specific format */
    oamEntryPtr->sourceInterface.type = dxChOamEntryPtr->sourceInterface.type;
    switch (dxChOamEntryPtr->sourceInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

            rc = prvUtfSwFromHwDeviceNumberGet(dxChOamEntryPtr->sourceInterface.devPort.hwDevNum,
                                               &(oamEntryPtr->sourceInterface.devPort.hwDevNum));
            if(GT_OK != rc)
                return rc;
            oamEntryPtr->sourceInterface.devPort.portNum = dxChOamEntryPtr->sourceInterface.devPort.portNum;
            break;

        case CPSS_INTERFACE_TRUNK_E:
            oamEntryPtr->sourceInterface.trunkId = dxChOamEntryPtr->sourceInterface.trunkId;
            break;

        case CPSS_INTERFACE_VIDX_E:
            oamEntryPtr->sourceInterface.vidx = dxChOamEntryPtr->sourceInterface.vidx;
            break;

        case CPSS_INTERFACE_VID_E:
            oamEntryPtr->sourceInterface.vlanId = dxChOamEntryPtr->sourceInterface.vlanId;
            break;

        case CPSS_INTERFACE_DEVICE_E:
            oamEntryPtr->sourceInterface.hwDevNum = dxChOamEntryPtr->sourceInterface.hwDevNum;
            break;

        case CPSS_INTERFACE_FABRIC_VIDX_E:
            oamEntryPtr->sourceInterface.fabricVidx = dxChOamEntryPtr->sourceInterface.fabricVidx;
            break;

        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, lmCounterCaptureEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, dualEndedLmEnable);

    /* convert LM counting mode from device specific format */
    switch(dxChOamEntryPtr->lmCountingMode)
    {
        case CPSS_DXCH_OAM_LM_COUNTING_MODE_RETAIN_E:
            oamEntryPtr->lmCountingMode = PRV_TGF_OAM_LM_COUNTING_MODE_RETAIN_E;
            break;
        case CPSS_DXCH_OAM_LM_COUNTING_MODE_ENABLE_E:
            oamEntryPtr->lmCountingMode = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
            break;
        case CPSS_DXCH_OAM_LM_COUNTING_MODE_DISABLE_E:
            oamEntryPtr->lmCountingMode = PRV_TGF_OAM_LM_COUNTING_MODE_DISABLE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, oamPtpOffsetIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, timestampEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, keepaliveAgingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, ageState);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, agingPeriodIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, agingThreshold);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, hashVerifyEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, lockHashValueEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, protectionLocUpdateEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, flowHash);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepaliveDetectionEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepalivePeriodCounter);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepalivePeriodThreshold);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepaliveMessageCounter);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, excessKeepaliveMessageThreshold);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, rdiCheckEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, rdiStatus);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, periodCheckEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChOamEntryPtr, oamEntryPtr, keepaliveTxPeriod);

    return GT_OK;
}
#endif /* !(defined CHX_FAMILY) */

/**
* @internal prvTgfOamEntrySet function
* @endinternal
*
* @brief   Set OAM Entry configuration
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - OAM entry index.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to OAM entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex or entry parameter.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_OUT_OF_RANGE          - one of the OAM entry fields are out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEntrySet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              entryIndex,
    IN  PRV_TGF_OAM_ENTRY_STC               *entryPtr
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_ENTRY_STC                 dxChOamEntry;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert OAM entry into device specific format */
    rc = prvTgfConvertGenericToDxChOamEntry(entryPtr, &dxChOamEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamEntry FAILED, rc = [%d]", rc);

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
            rc = cpssDxChOamEntrySet(devNum, dxChOamStage, entryIndex,
                                     &dxChOamEntry);
        }
        else
        {
            rc = cpssDxChOamPortGroupEntrySet(devNum, currPortGroupsBmp,
                                     dxChOamStage, entryIndex, &dxChOamEntry);
        }
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamEntrySet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif /* CHX_FAMILY */
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamEntryGet function
* @endinternal
*
* @brief   Get OAM Entry
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - OAM entry index.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to OAM entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEntryGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              entryIndex,
    OUT PRV_TGF_OAM_ENTRY_STC               *entryPtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_ENTRY_STC                 dxChOamEntry;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    cpssOsMemSet(&dxChOamEntry, 0, sizeof(dxChOamEntry));

    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChOamEntryGet(prvTgfDevNum, dxChOamStage, entryIndex,
                                 &dxChOamEntry);
    }
    else
    {
        rc = cpssDxChOamPortGroupEntryGet(prvTgfDevNum, currPortGroupsBmp,
                                 dxChOamStage, entryIndex, &dxChOamEntry);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert OAM entry into device specific format */
    return prvTgfConvertDxChToGenericOamEntry(&dxChOamEntry, entryPtr);
#endif /* CHX_FAMILY */
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamEnableSet function
* @endinternal
*
* @brief   Enable OAM processing
*
* @param[in] stage                    - OAM  type.
* @param[in] enable                   - enable/disable OAM processing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEnableSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_BOOL                             enable
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamEnableSet(devNum, dxChOamStage, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamEnableSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enable);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamEnableGet function
* @endinternal
*
* @brief   Get OAM processing status
*
* @param[in] stage                    - OAM  type.
*
* @param[out] enablePtr                - pointer to enable/disable OAM processing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEnableGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    OUT  GT_BOOL                            *enablePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamEnableGet(prvTgfDevNum, dxChOamStage, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enablePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamTableBaseFlowIdSet function
* @endinternal
*
* @brief   Set the base first Flow ID index used for OAM.
*
* @param[in] stage                    - OAM  type.
* @param[in] baseFlowId               - base first Flow ID index used for OAM.
*                                      (APPLICABLE RANGES: 0..65534)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_OUT_OF_RANGE          - base FlowId is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note OAM indices are calculated by <Flow ID>-<OAM Table Base Flow ID>.
*
*/
GT_STATUS prvTgfOamTableBaseFlowIdSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              baseFlowId
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamTableBaseFlowIdSet(devNum, dxChOamStage, baseFlowId);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamTableBaseFlowIdSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(baseFlowId);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamTableBaseFlowIdGet function
* @endinternal
*
* @brief   Get the base first Flow ID index used for OAM.
*
* @param[in] stage                    - OAM  type
*
* @param[out] baseFlowIdPtr            - pointer to first Flow ID index used for OAM.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note OAM indices are calculated by <Flow ID>-<OAM Table Base Flow ID>.
*
*/
GT_STATUS prvTgfOamTableBaseFlowIdGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    OUT  GT_U32                            *baseFlowIdPtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamTableBaseFlowIdGet(prvTgfDevNum, dxChOamStage, baseFlowIdPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamTableBaseFlowIdGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(baseFlowIdPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamExceptionConfigSet function
* @endinternal
*
* @brief   Set OAM exception configuration
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
* @param[in] exceptionConfigPtr       - OAM exception configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, exceptionType or
*                                       CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC fields.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamExceptionConfigSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    IN  PRV_TGF_OAM_EXCEPTION_CONFIG_STC        *exceptionConfigPtr
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        dxChOamExceptionType;
    CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC      dxChOamexceptionConfig;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    rc = prvTgfConvertGenericToDxChExceptionType(exceptionType, &dxChOamExceptionType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExceptionType FAILED, rc = [%d]", rc);

        return rc;
    }
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        dxChOamexceptionConfig.command = exceptionConfigPtr->command;
        dxChOamexceptionConfig.cpuCode = exceptionConfigPtr->cpuCode;
        dxChOamexceptionConfig.summaryBitEnable = exceptionConfigPtr->summaryBitEnable;

        rc = cpssDxChOamExceptionConfigSet(devNum, dxChOamStage,
                                           dxChOamExceptionType,
                                           &dxChOamexceptionConfig);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamExceptionConfigSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(exceptionConfigPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamExceptionConfigGet function
* @endinternal
*
* @brief   Get OAM exception configuration
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
*
* @param[out] exceptionConfigPtr       - pointer to OAM exception configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or exceptionType.
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamExceptionConfigGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    OUT PRV_TGF_OAM_EXCEPTION_CONFIG_STC        *exceptionConfigPtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        dxChOamExceptionType;
    CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC      dxChOamexceptionConfig;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    rc = prvTgfConvertGenericToDxChExceptionType(exceptionType, &dxChOamExceptionType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExceptionType FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamExceptionConfigGet(prvTgfDevNum, dxChOamStage,
                                       dxChOamExceptionType,
                                       &dxChOamexceptionConfig);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamExceptionConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    exceptionConfigPtr->command = dxChOamexceptionConfig.command;
    exceptionConfigPtr->cpuCode = dxChOamexceptionConfig.cpuCode;
    exceptionConfigPtr->summaryBitEnable = dxChOamexceptionConfig.summaryBitEnable;
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(exceptionConfigPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamExceptionCounterGet function
* @endinternal
*
* @brief   Get OAM exception counter value
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
*
* @param[out] counterValuePtr          - pointer to OAM exception counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or exceptionType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamExceptionCounterGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    OUT GT_U32                                  *counterValuePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        dxChOamExceptionType;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    rc = prvTgfConvertGenericToDxChExceptionType(exceptionType, &dxChOamExceptionType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExceptionType FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamExceptionCounterGet(prvTgfDevNum, dxChOamStage,
                                       dxChOamExceptionType,
                                       counterValuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamExceptionCounterGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(counterValuePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamExceptionGroupStatusGet function
* @endinternal
*
* @brief   Get OAM exception group status
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
*
* @param[out] groupStatusArr[PRV_TGF_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS] - array of OAM exception groups status.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or
*                                       PRV_TGF_OAM_EXCEPTION_TYPE_ENT value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Each bit in group status notifies exception in one of 32 OAM table entries
*       according to formula: <<1024n+32m>> - <<1024n+32m+31>>.
*       Where n - word in group status (0 < n < PRV_TGF_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS),
*       m - bit in group status (0 < m < 32)
*
*/
GT_STATUS prvTgfOamExceptionGroupStatusGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    OUT GT_U32                                  groupStatusArr[PRV_TGF_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS]
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        dxChOamExceptionType;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    rc = prvTgfConvertGenericToDxChExceptionType(exceptionType, &dxChOamExceptionType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExceptionType FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamExceptionGroupStatusGet(prvTgfDevNum, dxChOamStage,
                                            dxChOamExceptionType,
                                            groupStatusArr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamExceptionGroupStatusGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(groupStatusArr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamOpcodeSet function
* @endinternal
*
* @brief   Set OAM opcode value
*
* @param[in] stage                    - OAM  type.
* @param[in] opcodeType               - OAM opcode type.
* @param[in] opcodeIndex              - OAM opcode index: (APPLICABLE RANGES: 0..3)
* @param[in] opcodeValue              - OAM opcode value: (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, opcodeType or opcodeIndex.
* @retval GT_OUT_OF_RANGE          - opcode value is out of range.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeSet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT               stage,
    IN PRV_TGF_OAM_OPCODE_TYPE_ENT              opcodeType,
    IN GT_U32                                   opcodeIndex,
    IN GT_U32                                   opcodeValue
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    GT_U8     devNum = 0;
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_OPCODE_TYPE_ENT           dxChOpcodeType;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    rc = prvTgfConvertGenericToDxChOpcodeType(opcodeType, &dxChOpcodeType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOpcodeType FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamOpcodeSet(devNum, dxChOamStage, dxChOpcodeType,
                                  opcodeIndex, opcodeValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamOpcodeSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(opcodeType);
    TGF_PARAM_NOT_USED(opcodeIndex);
    TGF_PARAM_NOT_USED(opcodeValue);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamOpcodeGet function
* @endinternal
*
* @brief   Get OAM opcode value
*
* @param[in] stage                    - OAM  type.
* @param[in] opcodeType               - OAM opcode type.
* @param[in] opcodeIndex              - OAM opcode index: (APPLICABLE RANGES: 0..3).
*
* @param[out] opcodeValuePtr           - pointer to OAM opcode value
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, opcodeType or opcodeIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_OPCODE_TYPE_ENT             opcodeType,
    IN  GT_U32                                  opcodeIndex,
    OUT GT_U32                                  *opcodeValuePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_OPCODE_TYPE_ENT           dxChOpcodeType;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    rc = prvTgfConvertGenericToDxChOpcodeType(opcodeType, &dxChOpcodeType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOpcodeType FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamOpcodeGet(prvTgfDevNum, dxChOamStage,
                              dxChOpcodeType, opcodeIndex, opcodeValuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamOpcodeGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(opcodeType);
    TGF_PARAM_NOT_USED(opcodeIndex);
    TGF_PARAM_NOT_USED(opcodeValuePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamAgingPeriodEntrySet function
* @endinternal
*
* @brief   Set OAM aging period entry
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - aging period entry index:
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] agingPeriodValue         - expected time interval between
*                                      two received keepalive
*                                      messages from a given source in nanoseconds.
*                                      The granularity of the parameter is 40 ns.
*                                      Thus, the 34 bit field allows a period of
*                                      up to ~10 minutes
*                                      The  value is rounded down to nearest HW value.
*                                      (APPLICABLE RANGES: 0..0x3FFFFFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - aging period value is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingPeriodEntrySet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT       stage,
    IN GT_U32                           entryIndex,
    IN GT_U64                           agingPeriodValue
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamAgingPeriodEntrySet(devNum, dxChOamStage, entryIndex, agingPeriodValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamAgingPeriodEntrySet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(agingPeriodValue);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamAgingPeriodEntryGet function
* @endinternal
*
* @brief   Get OAM aging period entry
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - aging period entry index
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] agingPeriodValuePtr      - pointer to expected time interval between
*                                      two received keepalive messages from a given source.
*                                      The period is measured using a 25 MHz clock,
*                                      i.e., in a granularity of 40 ns.
*                                      Thus, the 34 bit field allows a period of
*                                      up to ~10 minutes
*                                      The  value is rounded down to nearest HW value.
*                                      (APPLICABLE RANGES: 0..0x3FFFFFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingPeriodEntryGet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT           stage,
    IN GT_U32                               entryIndex,
    OUT GT_U64                             *agingPeriodValuePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = prvTgfOamAgingPeriodEntryGet(prvTgfDevNum, entryIndex, agingPeriodValuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfOamAgingPeriodEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(agingPeriodValuePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamOpcodeProfilePacketCommandEntrySet function
* @endinternal
*
* @brief   Set OAM Opcode Packet Command Entry
*
* @param[in] stage                    - OAM  type.
* @param[in] opcode                   -  value: (APPLICABLE RANGES: 0..255).
* @param[in] profile                  -  index: (APPLICABLE RANGES: 0..7).
* @param[in] command                  - OAM packet command.
* @param[in] cpuCodeOffset            - two least significant bits of OAM CPU code:
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, opcode, profile or command.
* @retval GT_OUT_OF_RANGE          - CPU offset is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeProfilePacketCommandEntrySet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              opcode,
    IN  GT_U32                              profile,
    IN  CPSS_PACKET_CMD_ENT                 command,
    IN  GT_U32                              cpuCodeOffset
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamOpcodeProfilePacketCommandEntrySet(devNum, dxChOamStage,
                                                           opcode, profile,
                                                           command, cpuCodeOffset);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamOpcodeProfilePacketCommandEntrySet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(opcode);
    TGF_PARAM_NOT_USED(profile);
    TGF_PARAM_NOT_USED(command);
    TGF_PARAM_NOT_USED(cpuCodeOffset);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamOpcodeProfilePacketCommandEntryGet function
* @endinternal
*
* @brief   Get OAM Opcode Packet Command Entry
*
* @param[in] stage                    - OAM  type.
* @param[in] opcode                   -  value: (APPLICABLE RANGES: 0..255).
* @param[in] profile                  -  index: (APPLICABLE RANGES: 0..7).
*
* @param[out] commandPtr               - pointer to OAM packet command.
* @param[out] cpuCodeOffsetPtr         - pointer to two least significant bits
*                                      of OAM CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex or entry parameter.
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeProfilePacketCommandEntryGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              opcode,
    IN  GT_U32                              profile,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr,
    OUT GT_U32                              *cpuCodeOffsetPtr
)
{
    GT_STATUS rc = GT_BAD_STATE;
#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamOpcodeProfilePacketCommandEntryGet(prvTgfDevNum, dxChOamStage,
                                                       opcode, profile,
                                                       commandPtr, cpuCodeOffsetPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamOpcodeProfilePacketCommandEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(opcode);
    TGF_PARAM_NOT_USED(profile);
    TGF_PARAM_NOT_USED(commandPtr);
    TGF_PARAM_NOT_USED(cpuCodeOffsetPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamTimeStampEtherTypeSet function
* @endinternal
*
* @brief   Set the ethertype which is used to identify Timestamp Tag
*
* @param[in] etherType                - the Ethertype of the Timestamp Tag.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or etherType.
* @retval GT_OUT_OF_RANGE          - Ethertype is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampEtherTypeSet
(
    IN  GT_U32      etherType
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamTimeStampEtherTypeSet(devNum, etherType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamTimeStampEtherTypeSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(etherType);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamTimeStampEtherTypeGet function
* @endinternal
*
* @brief   Get the ethertype used to identify Timestamp Tag
*
* @param[out] etherTypePtr             - pointer to Ethertype of the Timestamp Tag.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampEtherTypeGet
(
    OUT  GT_U32      *etherTypePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    rc = cpssDxChOamTimeStampEtherTypeGet(prvTgfDevNum, etherTypePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamTimeStampEtherTypeGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(etherTypePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamTimeStampParsingEnableSet function
* @endinternal
*
* @brief   Enable timestamp parsing
*
* @param[in] enable                   - enable/disable timestamp parsing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampParsingEnableSet
(
    IN  GT_BOOL      enable
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamTimeStampParsingEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamTimeStampParsingEnableSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamTimeStampParsingEnableGet function
* @endinternal
*
* @brief   Get timestamp parsing status
*
* @param[out] enablePtr                - pointer to enable/disable timestamp parsing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampParsingEnableGet
(
    OUT  GT_BOOL     *enablePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    rc = cpssDxChOamTimeStampParsingEnableGet(prvTgfDevNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamTimeStampParsingEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enablePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamLmOffsetTableSet function
* @endinternal
*
* @brief   Set Loss Measurement Offset Table Entry
*
* @param[in] entryIndex               - entry index: (APPLICABLE RANGES: 0..15).
* @param[in] offset                   - LM  in bytes: (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or offset.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_OUT_OF_RANGE          - on offset out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamLmOffsetTableSet
(
    IN  GT_U32    entryIndex,
    IN  GT_U32    offset
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamLmOffsetTableSet(devNum, entryIndex, offset);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamLmOffsetTableSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(offset);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamLmOffsetTableGet function
* @endinternal
*
* @brief   Get Loss Measurement Offset Table Entry
*
* @param[in] entryIndex               - entry index: (APPLICABLE RANGES: 0..15).
*
* @param[out] offsetPtr                - pointer to LM offset in bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamLmOffsetTableGet
(
    IN  GT_U32                                      entryIndex,
    OUT GT_U32                                     *offsetPtr
)
{
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    rc = cpssDxChOamLmOffsetTableGet(prvTgfDevNum, entryIndex, offsetPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamLmOffsetTableGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(offsetPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code of an OAM packet that is trapped or mirrored to the CPU.
*
* @param[in] stage                    - OAM  type.
* @param[in] baseCpuCode              - CPU code of an OAM packet that is trapped
*                                      or mirrored to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or baseCpuCode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamCpuCodeBaseSet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT                   stage,
    IN CPSS_NET_RX_CPU_CODE_ENT                     baseCpuCode
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamCpuCodeBaseSet(devNum, dxChOamStage, baseCpuCode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamCpuCodeBaseSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(baseCpuCode);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code of an OAM packet that is trapped or mirrored to the CPU.
*
* @param[in] stage                    - OAM  type.
*
* @param[out] baseCpuCodePtr           - pointer to CPU code of an OAM packet that is trapped
*                                      or mirrored to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamCpuCodeBaseGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT                  stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT                    *baseCpuCodePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamCpuCodeBaseGet(prvTgfDevNum, dxChOamStage, baseCpuCodePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamCpuCodeBaseGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(baseCpuCodePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamDualEndedLmPacketCommandSet function
* @endinternal
*
* @brief   Set Packet Command for Dual-Ended Loss Measurement packets.
*
* @param[in] stage                    - OAM  type.
* @param[in] command                  - Packet Command for Dual-Ended Loss Measurement packets.
* @param[in] cpuCodeLsBits            - Two least significant bits of the CPU code
*                                      (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, command.
* @retval GT_OUT_OF_RANGE          - Packet command or two least significant bits
*                                       of CPU code are out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamDualEndedLmPacketCommandSet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT                   stage,
    IN CPSS_PACKET_CMD_ENT                          command,
    IN GT_U32                                       cpuCodeLsBits
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamDualEndedLmPacketCommandSet(devNum, dxChOamStage, command, cpuCodeLsBits);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamDualEndedLmPacketCommandSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(command);
    TGF_PARAM_NOT_USED(cpuCodeLsBits);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamDualEndedLmPacketCommandGet function
* @endinternal
*
* @brief   Get Packet Command for Dual-Ended Loss Measurement packets.
*
* @param[in] stage                    - OAM  type.
*
* @param[out] commandPtr               - pointer to Packet Command for
*                                      Dual-Ended Loss Measurement packets.
* @param[out] cpuCodeLsBitsPtr         - pointer to two least significant bits
*                                      of the CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamDualEndedLmPacketCommandGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT                  stage,
    OUT CPSS_PACKET_CMD_ENT                         *commandPtr,
    OUT GT_U32                                      *cpuCodeLsBitsPtr
)
{
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamDualEndedLmPacketCommandGet(prvTgfDevNum, dxChOamStage, commandPtr, cpuCodeLsBitsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamDualEndedLmPacketCommandGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(commandPtr);
    TGF_PARAM_NOT_USED(cpuCodeLsBitsPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamExceptionStatusGet function
* @endinternal
*
* @brief   Get exception status entry.
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
* @param[in] entryIndex               - index of OAM exception table entry
*
* @param[out] entryBmpPtr              - pointer to OAM exception table entry
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, exceptionType or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Each bit in the entry notifies exception per OAM flow according to formula:
*       <<32m+n>>; where m - table entry index (0 < m < 64). n - exception bit (0 < n < 32)
*
*/
GT_STATUS prvTgfOamExceptionStatusGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT                  stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT              exceptionType,
    IN  GT_U32                                      entryIndex,
    OUT GT_U32                                      *entryBmpPtr
)
{
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
    CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT        dxChOamExceptionType;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }
    rc = prvTgfConvertGenericToDxChExceptionType(exceptionType, &dxChOamExceptionType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExceptionType FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamExceptionStatusGet(prvTgfDevNum, dxChOamStage, dxChOamExceptionType, entryIndex, entryBmpPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamExceptionStatusGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(exceptionType);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryBmpPtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamAgingDaemonEnableSet function
* @endinternal
*
* @brief   Enable or disable OAM aging daemon.
*
* @param[in] stage                    - OAM  type.
* @param[in] enable                   - enable/disable OAM aging daemon:
*                                      GT_TRUE  - Aging Daemon periodically access
*                                      keepalive table and check the age state.
*                                      GT_FALSE - Aging Daemon doesn't request access
*                                      to keepalive table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingDaemonEnableSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_BOOL                             enable
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamAgingDaemonEnableSet(devNum, dxChOamStage, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamAgingDaemonEnableSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enable);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamAgingDaemonEnableGet function
* @endinternal
*
* @brief   Get OAM aging daemon status.
*
* @param[in] stage                    - OAM  type.
*
* @param[out] enablePtr                - pointer to OAM aging daemon status:
*                                      GT_TRUE  - Aging Daemon periodically access
*                                      keepalive table  and check the age state.
*                                      GT_FALSE - Aging Daemon doesn't request access
*                                      to keepalive table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingDaemonEnableGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChOamAgingDaemonEnableGet(prvTgfDevNum, dxChOamStage, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamAgingDaemonEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(enablePtr);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfOamMplsCwChannelTypeProfileSet function
* @endinternal
*
* @brief   Set an MPLS Control Word Channel Type to Channel Type Profile
*
* @param[in] profileIndex             - channel type profile index (APPLICABLE RANGES: 1..15)
* @param[in] channelTypeId            - channel type id      (APPLICABLE RANGES: 0..65535)
* @param[in] profileEnable            - enable/disable the channel id:
*                                      GT_TRUE: the packet Control Word Channel Type is
*                                      compared with the channel type profile value
*                                      GT_FALSE: the packet Control Word Channel Type is not
*                                      compared with the channel type profile value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - out-of-range in channelTypeId parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The MPLS packet is assumed to have a Control Word if the first nibble of
*       the MPLS payload is 0x1.
*       The Channel Type ID placed in 16 LSBs of a Control Word.
*
*/
GT_STATUS prvTgfOamMplsCwChannelTypeProfileSet
(
    IN  GT_U32    profileIndex,
    IN  GT_U32    channelTypeId,
    IN  GT_BOOL   profileEnable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamMplsCwChannelTypeProfileSet(
            devNum, profileIndex, channelTypeId, profileEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]:cpssDxChOamMplsCwChannelTypeProfileSet  FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(profileIndex);
    TGF_PARAM_NOT_USED(channelTypeId);
    TGF_PARAM_NOT_USED(profileEnable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfOamHashBitSelectionSet function
* @endinternal
*
* @brief   Set indexes of the first and last bits of the 32 bits hash.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] stage                    - OAM  type.
* @param[in] hashFirstBit             - hash first index
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] hashLastBit              - hash last index
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on bad stage, bad first or last index.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on hash index is out of range.
*/
GT_STATUS prvTgfOamHashBitSelectionSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT      stage,
    IN  GT_U32                          hashFirstBit,
    IN  GT_U32                          hashLastBit
)
{
    GT_U8     devNum = 0;
    GT_STATUS rc = GT_BAD_STATE;

#ifdef CHX_FAMILY
    CPSS_DXCH_OAM_STAGE_TYPE_ENT            dxChOamStage;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    /* convert stage into device specific format */
    rc = prvTgfConvertGenericToDxChOamStage(stage, &dxChOamStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChOamStage FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChOamHashBitSelectionSet(devNum, dxChOamStage,
                                            hashFirstBit, hashLastBit);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChOamHashBitSelectionSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    return GT_OK;
#endif
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(stage);
    TGF_PARAM_NOT_USED(hashFirstBit);
    TGF_PARAM_NOT_USED(hashLastBit);

    return rc;
#endif /* !(defined CHX_FAMILY) */
}


