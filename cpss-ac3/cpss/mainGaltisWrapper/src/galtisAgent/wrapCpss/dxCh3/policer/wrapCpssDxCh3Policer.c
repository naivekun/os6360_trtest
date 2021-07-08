/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file wrapCpssDxCh3Policer.c
*
* @brief wrappers for cpssDxCh3Policer.c
*
* @version   35
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

/* Policer stage number: Ingress #0, Ingress #1 or Egress */
static CPSS_DXCH_POLICER_STAGE_TYPE_ENT plrStage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

/* Save configured metering resolution to be used in eArch metering entry set */
/* while using legacy APIs - used for Ingress #0, Ingress #1 and Egress.      */
static CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT plrMeterResolution[3] =
                                  {CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E,
                                   CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E,
                                   CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E};

/* support for multi port groups */

/**
* @internal policerMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs
*         with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*                                       NONE
*/
static void policerMultiPortGroupsBmpGet
(
    IN   GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    /* default */
    *enablePtr  = GT_FALSE;
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return;
    }

    utilMultiPortGroupsBmpGet(devNum, enablePtr, portGroupsBmpPtr);
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerMeteringEntryRefresh
(
    IN GT_U8                          devNum,
    IN GT_U32                         entryIndex
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerMeteringEntryRefresh(devNum, plrStage, entryIndex);
    }
    else
    {
        return  cpssDxChPolicerPortGroupMeteringEntryRefresh(devNum, pgBmp,
                                                              plrStage,
                                                              entryIndex);
    }
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerErrorGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerErrorGet(devNum, plrStage, entryTypePtr,
                                         entryAddrPtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupErrorGet(devNum, pgBmp, plrStage,
                                                  entryTypePtr, entryAddrPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerErrorCounterGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerErrorCounterGet(devNum, plrStage, cntrValuePtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupErrorCounterGet(devNum, pgBmp, plrStage,
                                                         cntrValuePtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerManagementCountersSet(devNum, plrStage,
                                                      mngCntrSet,
                                                      mngCntrType, mngCntrPtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupManagementCountersSet(devNum, pgBmp,
                                                               plrStage,
                                                               mngCntrSet,
                                                               mngCntrType,
                                                               mngCntrPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerManagementCountersGet(devNum, plrStage,
                                                      mngCntrSet,
                                                      mngCntrType, mngCntrPtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupManagementCountersGet(devNum, pgBmp,
                                                               plrStage,
                                                               mngCntrSet,
                                                               mngCntrType,
                                                               mngCntrPtr);
    }
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerMeteringEntrySet(devNum, plrStage, entryIndex,
                                                 entryPtr, tbParamsPtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupMeteringEntrySet(devNum, pgBmp,
                                                          plrStage, entryIndex,
                                                          entryPtr, tbParamsPtr);
    }
}

/* new eArch fields setting to avoid failures */
static GT_STATUS cpssDxCh3PolicerMeteringEntryLegacyUpdates
(
    IN  GT_U8                                   devNum,
    INOUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryPtr
)
{
    entryPtr->byteOrPacketCountingMode = plrMeterResolution[plrStage];

    if(plrStage )
    {
        if(entryPtr->modifyExp == CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E)
        {
            entryPtr->modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        }

        if(entryPtr->modifyTc == CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E)
        {
            entryPtr->modifyTc = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        }
    }

    return cpssDxCh3PolicerPacketSizeModeGet(devNum,
                                             plrStage,
                                             &entryPtr->packetSizeMode);
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerMeteringEntryGet(devNum, plrStage, entryIndex,
                                                 entryPtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupMeteringEntryGet(devNum, pgBmp,
                                                          plrStage, entryIndex,
                                                          entryPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerBillingEntrySet(devNum, plrStage, entryIndex,
                                                billingCntrPtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupBillingEntrySet(devNum, pgBmp,
                                                         plrStage, entryIndex,
                                                         billingCntrPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxCh3PolicerBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxCh3PolicerBillingEntryGet(devNum, plrStage, entryIndex,
                                                reset, billingCntrPtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupBillingEntryGet(devNum, pgBmp,
                                                         plrStage, entryIndex,
                                                         reset, billingCntrPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPolicerPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPolicerPolicyCntrSet(devNum, plrStage, index,
                                             cntrValue);
    }
    else
    {
        return  cpssDxChPolicerPortGroupPolicyCntrSet(devNum, pgBmp, plrStage,
                                                      index, cntrValue);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPolicerPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPolicerPolicyCntrGet(devNum, plrStage, index,
                                             cntrValuePtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupPolicyCntrGet(devNum, pgBmp, plrStage,
                                                      index, cntrValuePtr);
    }
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChPolicerBucketsCurrentStateGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      policerIndex,
    OUT  CPSS_DXCH_POLICER_BUCKETS_STATE_STC        *bucketsStatePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    policerMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPolicerBucketsCurrentStateGet(
            devNum, stage, policerIndex, bucketsStatePtr);
    }
    else
    {
        return  cpssDxChPolicerPortGroupBucketsCurrentStateGet(
            devNum, pgBmp, stage, policerIndex, bucketsStatePtr);
    }
}


/**
* @internal wrCpssDxCh3PolicerMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables metering per device.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringEnableSet(devNum, plrStage, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringEnableGet function
* @endinternal
*
* @brief   Gets device metering status (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringEnableGet(devNum, plrStage, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerBillingCountingEnableSet function
* @endinternal
*
* @brief   Enables or disables Billing Counting.
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingCountingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerBillingCountingEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerBillingCountingEnableGet function
* @endinternal
*
* @brief   Gets device Billing Counting status (Enable/Disable).
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingCountingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerBillingCountingEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerBillingCountersModeSet function
* @endinternal
*
* @brief   Sets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrMode.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingCountersModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT cntrMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    cntrMode = (CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerBillingCountersModeSet(devNum, cntrMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerBillingCountersModeGet function
* @endinternal
*
* @brief   Gets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingCountersModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT cntrMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerBillingCountersModeGet(devNum, &cntrMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cntrMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Sets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on on wrong devNum or packetSize.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerPacketSizeModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSize;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    packetSize = (CPSS_POLICER_PACKET_SIZE_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerPacketSizeModeSet(devNum, plrStage, packetSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerPacketSizeModeGet function
* @endinternal
*
* @brief   Gets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerPacketSizeModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSize;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerPacketSizeModeGet(devNum, plrStage, &packetSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", packetSize);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeterResolutionSet function
* @endinternal
*
* @brief   Sets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or resolution.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeterResolutionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT resolution;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    resolution = (CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeterResolutionSet(devNum, plrStage, resolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    plrMeterResolution[plrStage] = resolution;

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeterResolutionGet function
* @endinternal
*
* @brief   Gets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeterResolutionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT resolution;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeterResolutionGet(devNum, plrStage, &resolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", resolution);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerDropTypeSet function
* @endinternal
*
* @brief   Sets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or dropType.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerDropTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DROP_MODE_TYPE_ENT dropType;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    dropType = (CPSS_DROP_MODE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerDropTypeSet(devNum, plrStage, dropType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerDropTypeGet function
* @endinternal
*
* @brief   Gets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerDropTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DROP_MODE_TYPE_ENT dropType;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerDropTypeGet(devNum, plrStage, &dropType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropType);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerCountingColorModeSet function
* @endinternal
*
* @brief   Sets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerCountingColorModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerCountingColorModeSet(devNum, plrStage, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerCountingColorModeGet function
* @endinternal
*
* @brief   Gets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerCountingColorModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerCountingColorModeGet(devNum, plrStage, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerManagementCntrsResolutionSet function
* @endinternal
*
* @brief   Sets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, Management Counters Set or
*                                       cntrResolution.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerManagementCntrsResolutionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT cntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT cntrResolution;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    cntrSet = (CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inArgs[1];
    cntrResolution = (CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxCh3PolicerManagementCntrsResolutionSet(devNum, plrStage,
                                                          cntrSet,
                                                          cntrResolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerManagementCntrsResolutionGet function
* @endinternal
*
* @brief   Gets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or Management Counters Set
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerManagementCntrsResolutionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT cntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT cntrResolution;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    cntrSet = (CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerManagementCntrsResolutionGet(devNum, plrStage,
                                                          cntrSet,
                                                          &cntrResolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cntrResolution);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Sets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or ttPacketSizeMode.
*/
static CMD_STATUS wrCpssDxCh3PolicerPacketSizeModeForTunnelTermSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT ttPacketSizeMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    ttPacketSizeMode = (CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerPacketSizeModeForTunnelTermSet(devNum, plrStage,
                                                            ttPacketSizeMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerPacketSizeModeForTunnelTermGet function
* @endinternal
*
* @brief   Gets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerPacketSizeModeForTunnelTermGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT ttPacketSizeMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerPacketSizeModeForTunnelTermGet(devNum, plrStage,
                                                            &ttPacketSizeMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ttPacketSizeMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringAutoRefreshScanEnableSet function
* @endinternal
*
* @brief   Enables or disables the metering Auto Refresh Scan mechanism.
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringAutoRefreshScanEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringAutoRefreshScanEnableGet function
* @endinternal
*
* @brief   Gets metering Auto Refresh Scan mechanism status (Enabled/Disabled).
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringAutoRefreshScanEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringAutoRefreshRangeSet function
* @endinternal
*
* @brief   Sets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_OUT_OF_RANGE          - on out of range of startAddress or stopAddress.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringAutoRefreshRangeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 startAddress;
    GT_U32 stopAddress;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    startAddress = (GT_U32)inArgs[1];
    stopAddress = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringAutoRefreshRangeSet(devNum, startAddress, stopAddress);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringAutoRefreshRangeGet function
* @endinternal
*
* @brief   Gets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointers.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringAutoRefreshRangeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 startAddress;
    GT_U32 stopAddress;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringAutoRefreshRangeGet(devNum, &startAddress, &stopAddress);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", startAddress, stopAddress);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringAutoRefreshIntervalSet function
* @endinternal
*
* @brief   Sets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_OUT_OF_RANGE          - on out of range of interval value.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringAutoRefreshIntervalSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 interval;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    interval = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringAutoRefreshIntervalSet(devNum, interval);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringAutoRefreshIntervalGet function
* @endinternal
*
* @brief   Gets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringAutoRefreshIntervalGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 interval;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerMeteringAutoRefreshIntervalGet(devNum, &interval);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", interval);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*         Meter Bucket's state parameters refreshing are needed in order to
*         prevent mis-behavior due to wrap around of timers.
*         The wrap around problem can occur when there are long periods of
*         'silence' on the flow and the bucket's state parameters do not get
*         refreshed (meter is in the idle state). This causes a problem once the
*         flow traffic is re-started since the timers might have already wrapped
*         around which can cause a situation in which the bucket's state is
*         mis-interpreted and the incoming packet is marked as non-conforming
*         even though the bucket was actually supposed to be full.
*         To prevent this from happening the CPU needs to trigger a meter
*         refresh transaction at least once every 10 minutes per meter.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_TIMEOUT               - on time out of Policer Tables non-direct access.
*
* @note Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryRefresh
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 entryIndex;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerMeteringEntryRefresh(devNum, entryIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerPortMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables a port metering trigger for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerPortMeteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxCh3PolicerPortMeteringEnableSet(devNum, plrStage, portNum,
                                                   enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerPortMeteringEnableGet function
* @endinternal
*
* @brief   Gets port status (Enable/Disable) of metering for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerPortMeteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxCh3PolicerPortMeteringEnableGet(devNum, plrStage, portNum,
                                                   &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMruSet function
* @endinternal
*
* @brief   Sets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on mruSize out of range of [0..65535 Bytes].
* @retval GT_HW_ERROR              - on Hardware error.
*/
static CMD_STATUS wrCpssDxCh3PolicerMruSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 mruSize;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mruSize = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxCh3PolicerMruSet(devNum, plrStage, mruSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMruGet function
* @endinternal
*
* @brief   Gets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerMruGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 mruSize;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerMruGet(devNum, plrStage, &mruSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mruSize);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*         This information available if error is happened and not read till now.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - on missing error information.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerErrorGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT entryType;
    GT_U32 entryAddr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerErrorGet(devNum, &entryType, &entryAddr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", entryType, entryAddr);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*         The Error Counter is a free-running non-sticky 8-bit read-only
*         counter.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerErrorCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 cntrValue;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerErrorCounterGet(devNum, &cntrValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cntrValue);

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or mngCntrType or Management
*                                       Counters Set number.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerManagementCountersSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT mngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT mngCntrType;
    CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC mngCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

   devNum = (GT_U8)inArgs[0];
   mngCntrSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inArgs[1];
   mngCntrType=(CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT)inArgs[2];
   mngCntr.duMngCntr.l[0]=(GT_U32)inArgs[3];
   mngCntr.packetMngCntr=(GT_U32)inArgs[4];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerManagementCountersSet(devNum, mngCntrSet,
                                                           mngCntrType,
                                                           &mngCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or mngCntrType or Management
*                                       Counters Set number.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerManagementCountersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT mngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT mngCntrType;
    CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC mngCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

     devNum = (GT_U8)inArgs[0];
     mngCntrSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inArgs[1];
     mngCntrType=(CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerManagementCountersGet(devNum, mngCntrSet,
                                                           mngCntrType, &mngCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",mngCntr.duMngCntr.l[0],mngCntr.packetMngCntr);

    return CMD_OK;
}
 /**************Table:cpssDxCh3PolicerMeteringEntry********************************/
static  GT_U32 minMeteringEntryIndex;
static  GT_U32 maxMeteringEntryIndex;
static  GT_U32 meteringEntryIndex;
/**
* @internal wrCpssDxCh3PolicerMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or entry parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of Policer Tables non-direct access.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 entryIndex;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC entry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT tbParams;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&entry, 0, sizeof(entry));

    /* map input arguments to locals */

    devNum =(GT_U8)inArgs[0];

    entryIndex=(GT_U32)inFields[0];
    entry.countingEntryIndex=(GT_U32)inFields[1];
    entry.mngCounterSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inFields[2];
    entry.meterColorMode=(CPSS_POLICER_COLOR_MODE_ENT)inFields[3];
    entry.meterMode=(CPSS_DXCH3_POLICER_METER_MODE_ENT)inFields[4];

    if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
          entry.tokenBucketParams.srTcmParams.cir=(GT_U32)inFields[5];
          entry.tokenBucketParams.srTcmParams.cbs=(GT_U32)inFields[6];
          entry.tokenBucketParams.srTcmParams.ebs=(GT_U32)inFields[7];
        }
    else
        if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
        {
          entry.tokenBucketParams.trTcmParams.cir=(GT_U32)inFields[8];
          entry.tokenBucketParams.trTcmParams.cbs=(GT_U32)inFields[9];
          entry.tokenBucketParams.trTcmParams.pir=(GT_U32)inFields[10];
          entry.tokenBucketParams.trTcmParams.pbs=(GT_U32)inFields[11];
        }

    entry.modifyUp=(CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[12];
    entry.modifyDscp=(CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[13];
    entry.yellowPcktCmd=(CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[14];
    entry.redPcktCmd=(CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[15];

    result = cpssDxCh3PolicerMeteringEntryLegacyUpdates(devNum, &entry);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerMeteringEntrySet(devNum, entryIndex,
                                                      &entry, &tbParams);

    /* pack output arguments to galtis string */


    if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
          galtisOutput(outArgs, result, "%d%d%d",tbParams.srTcmParams.cir,tbParams.srTcmParams.cbs,tbParams.srTcmParams.ebs);
        }
    else
        if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
        {
          galtisOutput(outArgs, result, "%d%d%d%d",tbParams.trTcmParams.cir,tbParams.trTcmParams.cbs,
                                               tbParams.trTcmParams.pir,tbParams.trTcmParams.pbs);
        }

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringEntryGetEntry function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;

    CPSS_DXCH3_POLICER_METERING_ENTRY_STC entry;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

     if (meteringEntryIndex>maxMeteringEntryIndex||meteringEntryIndex>1023)
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */

    devNum =(GT_U8)inArgs[0];


    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerMeteringEntryGet(devNum, meteringEntryIndex,
                                                      &entry);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    cmdOsMemSet(inFields,0,CMD_MAX_FIELDS);

    inFields[0]=meteringEntryIndex;
    inFields[1]=entry.countingEntryIndex;
    inFields[2]=entry.mngCounterSet;
    inFields[3]=entry.meterColorMode;
    inFields[4]=entry.meterMode;

    if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
          inFields[5]=entry.tokenBucketParams.srTcmParams.cir;
          inFields[6]=entry.tokenBucketParams.srTcmParams.cbs;
          inFields[7]=entry.tokenBucketParams.srTcmParams.ebs;
        }
    else
        if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
        {
          inFields[8]=entry.tokenBucketParams.trTcmParams.cir;
          inFields[9]=entry.tokenBucketParams.trTcmParams.cbs;
          inFields[10]=entry.tokenBucketParams.trTcmParams.pir;
          inFields[11]=entry.tokenBucketParams.trTcmParams.pbs;
        }

    inFields[12]=entry.modifyUp;
    inFields[13]=entry.modifyDscp;
    inFields[14]=entry.yellowPcktCmd;
    inFields[15]=entry.redPcktCmd;


    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0],inFields[1],inFields[2],inFields[3],
                                                   inFields[4],inFields[5],inFields[6],inFields[7],
                                                   inFields[8],inFields[9],inFields[10],inFields[11],
                                                  inFields[12],inFields[13],inFields[14],inFields[15]);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*******************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    minMeteringEntryIndex=(GT_U32)inArgs[1];
    maxMeteringEntryIndex=(GT_U32)inArgs[2];

    meteringEntryIndex=minMeteringEntryIndex;

    return wrCpssDxCh3PolicerMeteringEntryGetEntry(inArgs,inFields,numFields,outArgs);

}
/*******************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    meteringEntryIndex++;

    return wrCpssDxCh3PolicerMeteringEntryGetEntry(inArgs,inFields,numFields,outArgs);
}
/**
* @internal wrCpssDxCh3PolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculates Token Bucket parameters in the Application format without
*         HW update.
*         The token bucket parameters are returned as output values. This is
*         due to the hardware rate resolution, the exact rate or burst size
*         requested may not be honored. The returned value gives the user the
*         actual parameters configured in the hardware.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong DevicedevNum.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerEntryMeterParamsCalculate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_METER_MODE_ENT meterMode;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT tbInParams;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT tbOutParams;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

     devNum =(GT_U8)inArgs[0];
     meterMode=(CPSS_DXCH3_POLICER_METER_MODE_ENT)inArgs[1];

     if(meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
     {
         tbInParams.srTcmParams.cir=(GT_U32)inArgs[2];
         tbInParams.srTcmParams.cbs=(GT_U32)inArgs[3];
         tbInParams.srTcmParams.ebs=(GT_U32)inArgs[4];
    }
    else if(meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
    {
        tbInParams.trTcmParams.cir=(GT_U32)inArgs[5];
        tbInParams.trTcmParams.cbs=(GT_U32)inArgs[6];
        tbInParams.trTcmParams.pir=(GT_U32)inArgs[7];
        tbInParams.trTcmParams.pbs=(GT_U32)inArgs[8];
    }

    /* call cpss api function */
    result = cpssDxCh3PolicerEntryMeterParamsCalculate(devNum, plrStage, meterMode, &tbInParams, &tbOutParams);

    if(meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "%d%d%d%d%d%d%d",
                     tbOutParams.srTcmParams.cir,tbOutParams.srTcmParams.cbs,
                     tbOutParams.srTcmParams.ebs,
                     0,0,0,0);
    }
    else if(meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "%d%d%d%d%d%d%d",
                     0,0,0,
                     tbOutParams.trTcmParams.cir,tbOutParams.trTcmParams.cbs,
                     tbOutParams.trTcmParams.pir,tbOutParams.trTcmParams.pbs);
    }

    return CMD_OK;
}
/**************Table:cpssDxCh3PolicerBillingEntry********************************/
static  GT_U32 minBillingEntryIndex;
static  GT_U32 maxBillingEntryIndex;
static  GT_U32 billingEntryIndex;
/**
* @internal wrCpssDxCh3PolicerBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 entryIndex;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    entryIndex=(GT_U32)inFields[0];
    billingCntr.greenCntr.l[0]=(GT_U32)inFields[1];
    billingCntr.yellowCntr.l[0]=(GT_U32)inFields[2];
    billingCntr.redCntr.l[0]=(GT_U32)inFields[3];
    /* default values */
    billingCntr.greenCntr.l[1] = 0;
    billingCntr.yellowCntr.l[1] = 0;
    billingCntr.redCntr.l[1] = 0;
    billingCntr.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E;
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        billingCntr.billingCntrAllEnable = GT_FALSE;
        billingCntr.lmCntrCaptureMode =
            CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_COPY_E;
        billingCntr.greenCntrSnapshot.l[0] = 0;
        billingCntr.greenCntrSnapshot.l[1] = 0;
        billingCntr.greenCntrSnapshotValid = GT_FALSE;
        billingCntr.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        billingCntr.tunnelTerminationPacketSizeMode =
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E;
        billingCntr.dsaTagCountingMode =
            CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E;
        billingCntr.timeStampCountingMode =
            CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E;
    }
    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerBillingEntrySet(devNum, entryIndex,
                                                     &billingCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerBillingEntryGetEntry function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of Policer Tables non-direct access.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL reset;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (billingEntryIndex>maxBillingEntryIndex||billingEntryIndex>1023)
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    reset=(GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerBillingEntryGet(devNum, billingEntryIndex,
                                                     reset, &billingCntr);

   if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0]=billingEntryIndex;
    inFields[1]=billingCntr.greenCntr.l[0];
    inFields[2]=billingCntr.yellowCntr.l[0];
    inFields[3]=billingCntr.redCntr.l[0];

    fieldOutput("%d%d%d%d", inFields[0],inFields[1],inFields[2],inFields[3]);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*********************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    minBillingEntryIndex=(GT_U32)inArgs[2];
    maxBillingEntryIndex=(GT_U32)inArgs[3];
    billingEntryIndex=minBillingEntryIndex;

    return wrCpssDxCh3PolicerBillingEntryGetEntry(inArgs,inFields,numFields,outArgs);
}
/*********************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
     billingEntryIndex++;

     return wrCpssDxCh3PolicerBillingEntryGetEntry(inArgs,inFields,numFields,outArgs);

}

/**************Table:cpssDxCh3PolicerQosRemarkingEntry********************************/
static  GT_U8 minQosRemarkingEntryIndex;
static  GT_U8 maxQosRemarkingEntryIndex;
static  GT_U8 qosRemarkingEntryIndex;
/**
* @internal wrCpssDxCh3PolicerQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colors.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 Qos Remarking Entry contains three indexes [0..1023] of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
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
static CMD_STATUS wrCpssDxCh3PolicerQosRemarkingEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 qosProfileIndex;
    GT_U32 greenQosTableRemarkIndex;
    GT_U32 yellowQosTableRemarkIndex;
    GT_U32 redQosTableRemarkIndex;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    qosProfileIndex = (GT_U32)inFields[0];
    greenQosTableRemarkIndex = (GT_U32)inFields[1];
    yellowQosTableRemarkIndex = (GT_U32)inFields[2];
    redQosTableRemarkIndex = (GT_U32)inFields[3];

    /* call cpss api function */
    result = cpssDxCh3PolicerQosRemarkingEntrySet(
        devNum, plrStage, qosProfileIndex,
        greenQosTableRemarkIndex,
        yellowQosTableRemarkIndex,
        redQosTableRemarkIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerQosRemarkingEntryGetEntry function
* @endinternal
*
* @brief   Gets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colours.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 Qos Remarking Entry contains three indexes [0..1023] of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
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
static CMD_STATUS wrCpssDxCh3PolicerQosRemarkingEntryGetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 greenQosTableRemarkIndex;
    GT_U32 yellowQosTableRemarkIndex;
    GT_U32 redQosTableRemarkIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (qosRemarkingEntryIndex > maxQosRemarkingEntryIndex)
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxCh3PolicerQosRemarkingEntryGet(
        devNum, plrStage, qosRemarkingEntryIndex,
        &greenQosTableRemarkIndex,
        &yellowQosTableRemarkIndex,
        &redQosTableRemarkIndex);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput(
        "%d%d%d%d", qosRemarkingEntryIndex,
        greenQosTableRemarkIndex,
        yellowQosTableRemarkIndex,
        redQosTableRemarkIndex);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/***************************************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerQosRemarkingEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    minQosRemarkingEntryIndex=(GT_U8)inArgs[1];
    maxQosRemarkingEntryIndex=(GT_U8)inArgs[2];
    qosRemarkingEntryIndex=minQosRemarkingEntryIndex;

    return wrCpssDxCh3PolicerQosRemarkingEntryGetEntry(inArgs,inFields,numFields,outArgs);

}
/***************************************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerQosRemarkingEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
   qosRemarkingEntryIndex++;
   return wrCpssDxCh3PolicerQosRemarkingEntryGetEntry(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChPolicerCountingModeSet function
* @endinternal
*
* @brief   Configures counting mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerCountingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_COUNTING_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_POLICER_COUNTING_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerCountingModeSet(devNum, plrStage, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerCountingModeGet function
* @endinternal
*
* @brief   Gets the couning mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerCountingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    /* CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage; */
    CPSS_DXCH_POLICER_COUNTING_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerCountingModeGet(devNum, plrStage, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);


    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEgressQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerEgressQosRemarkingEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType;
    GT_U32                                      remarkParamValue;
    CPSS_DP_LEVEL_ENT                           confLevel;
    CPSS_DXCH_POLICER_QOS_PARAM_STC             qosParam;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    remarkTableType = (CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT)inArgs[1];
    remarkParamValue = (GT_U32)inArgs[2];
    confLevel = (CPSS_DP_LEVEL_ENT)inArgs[3];

    qosParam.up = (GT_U32)inFields[0];
    qosParam.dscp = (GT_U32)inFields[1];
    qosParam.exp = (GT_U32)inFields[2];
    qosParam.dp  = (CPSS_DP_LEVEL_ENT)inFields[3];

    /* call cpss api function */
    result = cpssDxChPolicerEgressQosRemarkingEntrySet(devNum,
                                                       remarkTableType,
                                                       remarkParamValue,
                                                       confLevel,
                                                       &qosParam);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEgressQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerEgressQosRemarkingEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType;
    GT_U32                                      remarkParamValue;
    CPSS_DP_LEVEL_ENT                           confLevel;
    CPSS_DXCH_POLICER_QOS_PARAM_STC             qosParam;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    remarkTableType = (CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT)inArgs[1];
    remarkParamValue = (GT_U32)inArgs[2];
    confLevel = (CPSS_DP_LEVEL_ENT)inArgs[3];


    /* call cpss api function */
    result = cpssDxChPolicerEgressQosRemarkingEntryGet(devNum,
                                                       remarkTableType,
                                                       remarkParamValue,
                                                       confLevel,
                                                       &qosParam);

    inFields[0] = qosParam.up;
    inFields[1] = qosParam.dscp;
    inFields[2] = qosParam.exp;
    inFields[3] = qosParam.dp;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1],
                              inFields[2], inFields[3]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerStageMeterModeSet function
* @endinternal
*
* @brief   Sets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
static CMD_STATUS wrCpssDxChPolicerStageMeterModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerStageMeterModeSet(devNum, plrStage, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerStageMeterModeGet function
* @endinternal
*
* @brief   Gets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
static CMD_STATUS wrCpssDxChPolicerStageMeterModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]

)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerStageMeterModeGet(devNum, plrStage, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMeteringOnTrappedPktsEnableSet function
* @endinternal
*
* @brief   Enable/Disable Ingress metering for Trapped packets.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
static CMD_STATUS wrCpssDxChPolicerMeteringOnTrappedPktsEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerMeteringOnTrappedPktsEnableSet(devNum, plrStage,
                                                           enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMeteringOnTrappedPktsEnableGet function
* @endinternal
*
* @brief   Get Ingress metering status (Enabled/Disabled) for Trapped packets.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
static CMD_STATUS wrCpssDxChPolicerMeteringOnTrappedPktsEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerMeteringOnTrappedPktsEnableGet(devNum, plrStage,
                                                           &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPortStormTypeIndexSet function
* @endinternal
*
* @brief   Associates policing profile with source/target port and storm rate type.
*         There are up to four policers per port and every storm type can be
*         flexibly associated with any of the four policers.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType ot port.
* @retval GT_OUT_OF_RANGE          - on out of range of index.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerPortStormTypeIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_PHYSICAL_PORT_NUM                               portNum;
    CPSS_DXCH_POLICER_STORM_TYPE_ENT    stormType;
    GT_U32                              index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stormType = (CPSS_DXCH_POLICER_STORM_TYPE_ENT)inArgs[2];
    index = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPolicerPortStormTypeIndexSet(devNum, plrStage, portNum,
                                                  stormType, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerPortStormTypeIndexGet function
* @endinternal
*
* @brief   Get policing profile for given source/target port and storm rate type.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType or portNum.
* @retval GT_OUT_OF_RANGE          - on out of range of port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerPortStormTypeIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_PHYSICAL_PORT_NUM                               portNum;
    CPSS_DXCH_POLICER_STORM_TYPE_ENT    stormType;
    GT_U32                              index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    stormType = (CPSS_DXCH_POLICER_STORM_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPolicerPortStormTypeIndexGet(devNum, plrStage, portNum,
                                                  stormType, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", index);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEgressL2RemarkModelSet function
* @endinternal
*
* @brief   Sets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerEgressL2RemarkModelSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_U8                                    devNum;
    CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    model;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    model = (CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerEgressL2RemarkModelSet(devNum, model);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEgressL2RemarkModelGet function
* @endinternal
*
* @brief   Gets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerEgressL2RemarkModelGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_U8                                    devNum;
    CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    model;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerEgressL2RemarkModelGet(devNum, &model);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", model);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEgressQosUpdateEnableSet function
* @endinternal
*
* @brief   The function enables or disables QoS remarking of conforming packets.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerEgressQosUpdateEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerEgressQosUpdateEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEgressQosUpdateEnableGet function
* @endinternal
*
* @brief   The function get QoS remarking status of conforming packets
*         (enable/disable).
*
* @note   APPLICABLE DEVICES:      All DxChXcat Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerEgressQosUpdateEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerEgressQosUpdateEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerVlanCntrSet function
* @endinternal
*
* @brief   Sets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*       In order to set Vlan counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static CMD_STATUS wrCpssDxChPolicerVlanCntrSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_U16          vid;
    GT_U32          cntrValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vid = (GT_U16)inArgs[1];
    cntrValue = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPolicerVlanCntrSet(devNum, plrStage, vid, cntrValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerVlanCntrGet function
* @endinternal
*
* @brief   Gets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
static CMD_STATUS wrCpssDxChPolicerVlanCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_U16          vid;
    GT_U32          cntrValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vid = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerVlanCntrGet(devNum, plrStage, vid, &cntrValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cntrValue);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerVlanCountingPacketCmdTriggerSet function
* @endinternal
*
* @brief   Enables or disables VLAN counting triggering according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported egress direction
*
* @note All packet commands are enabled in the HW by default.
*       For egress policer the default trigger values shouldn't be changed.
*
*/
static CMD_STATUS wrCpssDxChPolicerVlanCountingPacketCmdTriggerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     cmdTrigger;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cmdTrigger = (CPSS_PACKET_CMD_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPolicerVlanCountingPacketCmdTriggerSet(devNum, plrStage,
                                                            cmdTrigger, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerVlanCountingPacketCmdTriggerGet function
* @endinternal
*
* @brief   Gets VLAN counting triggering status (Enable/Disable) according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All packet commands are enabled in the HW by default.
*
*/
static CMD_STATUS wrCpssDxChPolicerVlanCountingPacketCmdTriggerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     cmdTrigger;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cmdTrigger = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerVlanCountingPacketCmdTriggerGet(devNum, plrStage,
                                                            cmdTrigger, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerVlanCountingModeSet function
* @endinternal
*
* @brief   Sets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerVlanCountingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerVlanCountingModeSet(devNum, plrStage, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerVlanCountingModeGet function
* @endinternal
*
* @brief   Gets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerVlanCountingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerVlanCountingModeGet(devNum, plrStage, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a Policy rules.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters).
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static CMD_STATUS wrCpssDxChPolicerPolicyCntrSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      index;
    GT_U32      cntrValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    cntrValue = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPolicerPolicyCntrSet(devNum, index, cntrValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a a Policy rules.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*
*/
static CMD_STATUS wrCpssDxChPolicerPolicyCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      index;
    GT_U32      cntrValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPolicerPolicyCntrGet(devNum, index, &cntrValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cntrValue);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMemorySizeModeSet function
* @endinternal
*
* @brief   Sets internal table sizes and the way they are shared between the Ingress
*         policers.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mode change should not be done when meter or counter entries are in use.
*
*/
static CMD_STATUS wrCpssDxChPolicerMemorySizeModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  mode;
    GT_U32                                  numEntriesIngressStage0=0xFFFFFFFF;
    GT_U32                                  numEntriesIngressStage1=0xFFFFFFFF;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerMemorySizeModeSet(devNum, mode , numEntriesIngressStage0 , numEntriesIngressStage1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMemorySizeModeGet function
* @endinternal
*
* @brief   Gets internal table sizes and the way they are shared between the Ingress
*         policers.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerMemorySizeModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerMemorySizeModeGet(devNum, &mode,NULL,NULL,NULL);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}
/**
* @internal wrCpssDxChPolicerMemorySizeModeSet2 function
* @endinternal
*
* @brief   Sets internal table sizes and the way they are shared between the Ingress
*         policers.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mode change should not be done when meter or counter entries are in use.
*
*/
static CMD_STATUS wrCpssDxChPolicerMemorySizeModeSet2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  mode;
    GT_U32                                  numEntriesIngressStage0;
    GT_U32                                  numEntriesIngressStage1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT)inArgs[1];
    numEntriesIngressStage0 = (GT_U32)inArgs[2];
    numEntriesIngressStage1 = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPolicerMemorySizeModeSet(devNum, mode , numEntriesIngressStage0 , numEntriesIngressStage1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMemorySizeModeGet2 function
* @endinternal
*
* @brief   Gets internal table sizes and the way they are shared between the Ingress
*         policers.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerMemorySizeModeGet2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  mode;
    GT_U32                                  numEntriesIngressStage0;
    GT_U32                                  numEntriesIngressStage1;
    GT_U32                                  numEntriesegressStage;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerMemorySizeModeGet(devNum, &mode,
        &numEntriesIngressStage0,
        &numEntriesIngressStage1,
        &numEntriesegressStage);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d",
        mode,
        numEntriesIngressStage0,
        numEntriesIngressStage1,
        numEntriesegressStage);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerCountingWriteBackCacheFlush function
* @endinternal
*
* @brief   Flush internal Write Back Cache (WBC) of counting entries.
*         The Policer implements internal Write Back Cache for frequent and rapid
*         update of counting entries.
*         Since the WBC holds the policer entries' data, the CPU must clear its
*         content when performing direct write access of the counting entries by
*         one of following functions:
*         - cpssDxCh3PolicerBillingEntrySet
*         - cpssDxChPolicerVlanCntrSet
*         - cpssDxChPolicerPolicyCntrSet
*         The policer counting should be disabled before flush of WBC if direct
*         write access is performed under traffic. And policer counting should be
*         enabled again after finish of write access.
*         The cpssDxChPolicerCountingModeSet may be used to disable/enable
*         the policer counting.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To use the listed above APIs (cpssDxCh3PolicerBillingEntrySet,
*       cpssDxChPolicerVlanCntrSet and cpssDxChPolicerPolicyCntrSet) under
*       traffic, perform the following algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static CMD_STATUS wrCpssDxChPolicerCountingWriteBackCacheFlush
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, plrStage);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerGlobalStageSet function
* @endinternal
*
* @brief   Sets global Policer stage parameter.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
*
* @note Mode change should not be done when meter or counter entries are in use.
*
*/
static CMD_STATUS wrCpssDxChPolicerGlobalStageSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    plrStage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[0];

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerManagementCountersSetV1 function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or mngCntrType or Management
*                                       Counters Set number.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerManagementCountersSetV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT mngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT mngCntrType;
    CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC mngCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

   devNum = (GT_U8)inArgs[0];
   mngCntrSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inArgs[1];
   mngCntrType=(CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT)inArgs[2];
   /* counter word 0 */
   mngCntr.duMngCntr.l[0]=(GT_U32)inArgs[3];
   /* counter word 1 */
   mngCntr.duMngCntr.l[1]=(GT_U32)inArgs[4];
   mngCntr.packetMngCntr=(GT_U32)inArgs[5];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerManagementCountersSet(devNum, mngCntrSet,
                                                           mngCntrType, &mngCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerManagementCountersGetV1 function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or mngCntrType or Management
*                                       Counters Set number.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerManagementCountersGetV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT mngCntrSet;
    CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT mngCntrType;
    CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC mngCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

     devNum = (GT_U8)inArgs[0];
     mngCntrSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inArgs[1];
     mngCntrType=(CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerManagementCountersGet(devNum, mngCntrSet,
                                                           mngCntrType, &mngCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",mngCntr.duMngCntr.l[0],
                 mngCntr.duMngCntr.l[1],mngCntr.packetMngCntr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMeteringCalcMethodSet function
* @endinternal
*
* @brief   Sets Metering Policer parameters calculation orientation method.
*         Due to the fact that selecting algorithm has effect on how HW metering
*         parameters are set, this API influence on
*         "cpssDxCh3PolicerMeteringEntrySet" and
*         "cpssDxCh3PolicerEntryMeterParamsCalculate" APIs.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
static CMD_STATUS wrCpssDxChPolicerMeteringCalcMethodSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT   meteringCalcMethod;
    GT_U32                                       cirPirAllowedDeviation;
    GT_BOOL                                      cbsPbsCalcOnFail;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    meteringCalcMethod=(CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT)inArgs[1];
    cirPirAllowedDeviation=(GT_U32)inArgs[2];
    cbsPbsCalcOnFail=(GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPolicerMeteringCalcMethodSet(devNum,
                                                  meteringCalcMethod,
                                                  cirPirAllowedDeviation,
                                                  cbsPbsCalcOnFail);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMeteringCalcMethodGet function
* @endinternal
*
* @brief   Gets Metering Policer parameters calculation orientation method.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - wrong devNum or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
static CMD_STATUS wrCpssDxChPolicerMeteringCalcMethodGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT   meteringCalcMethod;
    GT_U32                                       cirPirAllowedDeviation;
    GT_BOOL                                      cbsPbsCalcOnFail;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    cirPirAllowedDeviation = 0xFFFFFFFF;
    cbsPbsCalcOnFail = GT_FALSE;

    /* call cpss api function */
    result = cpssDxChPolicerMeteringCalcMethodGet(devNum,
                                                  &meteringCalcMethod,
                                                  &cirPirAllowedDeviation,
                                                  &cbsPbsCalcOnFail);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",meteringCalcMethod,
                 cirPirAllowedDeviation,cbsPbsCalcOnFail);

    return CMD_OK;
}

 /**************Table:cpssDxChXcatPolicerMeteringEntry********************************/
static  GT_U32 minMeteringEntryXcatIndex;
static  GT_U32 maxMeteringEntryXcatIndex;
static  GT_U32 meteringEntryXcatIndex;
/**
* @internal wrCpssDxCh3PolicerMeteringEntrySetV1 function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or entry parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of Policer Tables non-direct access.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntrySetV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 entryIndex;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC entry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT tbParams;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&entry, 0, sizeof(entry));

    /* map input arguments to locals */

    devNum =(GT_U8)inArgs[0];

    entryIndex=(GT_U32)inFields[0];
    entry.countingEntryIndex=(GT_U32)inFields[1];
    entry.mngCounterSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inFields[2];
    entry.meterColorMode=(CPSS_POLICER_COLOR_MODE_ENT)inFields[3];
    entry.meterMode=(CPSS_DXCH3_POLICER_METER_MODE_ENT)inFields[4];

    if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
          entry.tokenBucketParams.srTcmParams.cir=(GT_U32)inFields[5];
          entry.tokenBucketParams.srTcmParams.cbs=(GT_U32)inFields[6];
          entry.tokenBucketParams.srTcmParams.ebs=(GT_U32)inFields[7];
        }
    else
        if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
        {
          entry.tokenBucketParams.trTcmParams.cir=(GT_U32)inFields[8];
          entry.tokenBucketParams.trTcmParams.cbs=(GT_U32)inFields[9];
          entry.tokenBucketParams.trTcmParams.pir=(GT_U32)inFields[10];
          entry.tokenBucketParams.trTcmParams.pbs=(GT_U32)inFields[11];
        }

    entry.modifyUp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[12];
    entry.modifyDscp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[13];
    entry.modifyDp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[14];
    entry.yellowPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[15];
    entry.redPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[16];
    entry.qosProfile = (GT_U32)inFields[17];
    entry.remarkMode = (CPSS_DXCH_POLICER_REMARK_MODE_ENT)inFields[18];

    result = cpssDxCh3PolicerMeteringEntryLegacyUpdates(devNum, &entry);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerMeteringEntrySet(devNum, entryIndex,
                                                      &entry, &tbParams);

    /* pack output arguments to galtis string */


    if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
          galtisOutput(outArgs, result, "%d%d%d",tbParams.srTcmParams.cir,tbParams.srTcmParams.cbs,tbParams.srTcmParams.ebs);
        }
    else
        if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
        {
          galtisOutput(outArgs, result, "%d%d%d%d",tbParams.trTcmParams.cir,tbParams.trTcmParams.cbs,
                                               tbParams.trTcmParams.pir,tbParams.trTcmParams.pbs);
        }

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerLastEntryGet function
* @endinternal
*
* @brief   Gets metering and counting last entry for refresh procedures.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] devNum                   - device number.
*
* @retval GT_U32                   - memory size
*/
static GT_U32 wrCpssDxChPolicerLastEntryGet
(
    IN  GT_U8 devNum
)
{
    if (PRV_CPSS_MAX_PP_DEVICES_CNS <= devNum)
    {
        return 8191;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return 8191;
    }
    else
    {
        return 2047;
    }
}

/**
* @internal wrCpssDxCh3PolicerMeteringEntryGetEntryV1 function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetEntryV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;

    CPSS_DXCH3_POLICER_METERING_ENTRY_STC entry;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =(GT_U8)inArgs[0];

     if (meteringEntryXcatIndex>maxMeteringEntryXcatIndex||meteringEntryXcatIndex>wrCpssDxChPolicerLastEntryGet(devNum))
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* reset entry to avoid trash in output for not used fields */
    cmdOsMemSet(&entry, 0, sizeof(entry));

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerMeteringEntryGet(devNum, meteringEntryXcatIndex,
                                                      &entry);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    cmdOsMemSet(inFields,0,CMD_MAX_FIELDS);

    inFields[0]=meteringEntryXcatIndex;
    inFields[1]=entry.countingEntryIndex;
    inFields[2]=entry.mngCounterSet;
    inFields[3]=entry.meterColorMode;
    inFields[4]=entry.meterMode;

    if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
          inFields[5]=entry.tokenBucketParams.srTcmParams.cir;
          inFields[6]=entry.tokenBucketParams.srTcmParams.cbs;
          inFields[7]=entry.tokenBucketParams.srTcmParams.ebs;
        }
    else
        if(entry.meterMode==CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E)
        {
          inFields[8]=entry.tokenBucketParams.trTcmParams.cir;
          inFields[9]=entry.tokenBucketParams.trTcmParams.cbs;
          inFields[10]=entry.tokenBucketParams.trTcmParams.pir;
          inFields[11]=entry.tokenBucketParams.trTcmParams.pbs;
        }

    inFields[12]=entry.modifyUp;
    inFields[13]=entry.modifyDscp;
    inFields[14]=entry.modifyDp;
    inFields[15]=entry.yellowPcktCmd;
    inFields[16]=entry.redPcktCmd;
    inFields[17]=entry.qosProfile;
    inFields[18]=entry.remarkMode;


    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],
                inFields[5],inFields[6],inFields[7],inFields[8],inFields[9],
                inFields[10],inFields[11],inFields[12],inFields[13],
                inFields[14],inFields[15],inFields[16],inFields[17],inFields[18]);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*******************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetFirstV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    minMeteringEntryXcatIndex=(GT_U32)inArgs[1];
    maxMeteringEntryXcatIndex=(GT_U32)inArgs[2];

    meteringEntryXcatIndex=minMeteringEntryXcatIndex;

    return wrCpssDxCh3PolicerMeteringEntryGetEntryV1(inArgs,inFields,numFields,outArgs);

}
/*******************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetNextV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    meteringEntryXcatIndex++;

    return wrCpssDxCh3PolicerMeteringEntryGetEntryV1(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxCh3PolicerMeteringEntrySetV2 function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or entry parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of Policer Tables non-direct access.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntrySetV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 entryIndex;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC entry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT tbParams;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&entry, 0, sizeof(entry));

    /* map input arguments to locals */

    devNum =(GT_U8)inArgs[0];

    entryIndex=(GT_U32)inFields[0];
    entry.countingEntryIndex=(GT_U32)inFields[1];
    entry.mngCounterSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inFields[2];
    entry.meterColorMode=(CPSS_POLICER_COLOR_MODE_ENT)inFields[3];
    entry.meterMode=(CPSS_DXCH3_POLICER_METER_MODE_ENT)inFields[4];

    switch(entry.meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            entry.tokenBucketParams.srTcmParams.cir=(GT_U32)inFields[5];
            entry.tokenBucketParams.srTcmParams.cbs=(GT_U32)inFields[6];
            entry.tokenBucketParams.srTcmParams.ebs=(GT_U32)inFields[7];
            break;
        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            entry.tokenBucketParams.trTcmParams.cir=(GT_U32)inFields[8];
            entry.tokenBucketParams.trTcmParams.cbs=(GT_U32)inFields[9];
            entry.tokenBucketParams.trTcmParams.pir=(GT_U32)inFields[10];
            entry.tokenBucketParams.trTcmParams.pbs=(GT_U32)inFields[11];
            break;
        default:
            galtisOutput(outArgs, GT_BAD_PARAM, "%d", -1);
            return CMD_OK;
    }

    entry.modifyUp = (CPSS_DXCH_POLICER_MODIFY_UP_ENT)inFields[12];
    entry.modifyDscp = (CPSS_DXCH_POLICER_MODIFY_DSCP_ENT)inFields[13];
    entry.modifyDp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[14];
    entry.modifyExp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[15];
    entry.modifyTc = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[16];
    entry.greenPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[17];
    entry.yellowPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[18];
    entry.redPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[19];
    entry.qosProfile = (GT_U32)inFields[20];
    entry.remarkMode = (CPSS_DXCH_POLICER_REMARK_MODE_ENT)inFields[21];

    entry.byteOrPacketCountingMode =
                    (CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT)inFields[22];
    entry.packetSizeMode = (CPSS_POLICER_PACKET_SIZE_MODE_ENT)inFields[23];
    entry.tunnelTerminationPacketSizeMode =
                    (CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT)inFields[24];
    entry.dsaTagCountingMode =
                    (CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT)inFields[25];
    entry.timeStampCountingMode =
                    (CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT)inFields[26];
    entry.yellowEcnMarkingEnable = (GT_BOOL)inFields[27];
    entry.greenMirrorToAnalyzerEnable = (GT_BOOL)inFields[28];
    entry.yellowMirrorToAnalyzerEnable = (GT_BOOL)inFields[29];
    entry.redMirrorToAnalyzerEnable = (GT_BOOL)inFields[30];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerMeteringEntrySet(devNum, entryIndex,
                                                      &entry, &tbParams);

    /* pack output arguments to galtis string */
    switch(entry.meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            galtisOutput(outArgs, result, "%d%d%d",tbParams.srTcmParams.cir,tbParams.srTcmParams.cbs,tbParams.srTcmParams.ebs);
            break;
        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            galtisOutput(outArgs, result, "%d%d%d%d",tbParams.trTcmParams.cir,tbParams.trTcmParams.cbs,
                                               tbParams.trTcmParams.pir,tbParams.trTcmParams.pbs);
            break;
        default:
            galtisOutput(outArgs, GT_BAD_PARAM, "%d", -1);
            return CMD_OK;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerMeteringEntryGetEntryV2 function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetEntryV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =(GT_U8)inArgs[0];

    if (meteringEntryXcatIndex>maxMeteringEntryXcatIndex||meteringEntryXcatIndex>wrCpssDxChPolicerLastEntryGet(devNum))
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* reset entry to avoid trash in output for not used fields */
    cmdOsMemSet(&entry, 0, sizeof(entry));

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerMeteringEntryGet(devNum, meteringEntryXcatIndex,
                                                      &entry);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    cmdOsMemSet(inFields,0,CMD_MAX_FIELDS);

    inFields[0]=meteringEntryXcatIndex;
    inFields[1]=entry.countingEntryIndex;
    inFields[2]=entry.mngCounterSet;
    inFields[3]=entry.meterColorMode;
    inFields[4]=entry.meterMode;

    switch(entry.meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            inFields[5]=entry.tokenBucketParams.srTcmParams.cir;
            inFields[6]=entry.tokenBucketParams.srTcmParams.cbs;
            inFields[7]=entry.tokenBucketParams.srTcmParams.ebs;
            break;
        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            inFields[8]=entry.tokenBucketParams.trTcmParams.cir;
            inFields[9]=entry.tokenBucketParams.trTcmParams.cbs;
            inFields[10]=entry.tokenBucketParams.trTcmParams.pir;
            inFields[11]=entry.tokenBucketParams.trTcmParams.pbs;
            break;
        default:
            galtisOutput(outArgs, GT_BAD_PARAM, "%d", -1);
            return CMD_OK;
    }

    inFields[12]=entry.modifyUp;
    inFields[13]=entry.modifyDscp;
    inFields[14]=entry.modifyDp;
    inFields[15]=entry.modifyExp;
    inFields[16]=entry.modifyTc;
    inFields[17]=entry.greenPcktCmd;
    inFields[18]=entry.yellowPcktCmd;
    inFields[19]=entry.redPcktCmd;
    inFields[20]=entry.qosProfile;
    inFields[21]=entry.remarkMode;
    inFields[22]=entry.byteOrPacketCountingMode;
    inFields[23]=entry.packetSizeMode;
    inFields[24]=entry.tunnelTerminationPacketSizeMode;
    inFields[25]=entry.dsaTagCountingMode;
    inFields[26]=entry.timeStampCountingMode;
    inFields[27]=entry.yellowEcnMarkingEnable;
    inFields[28]=entry.greenMirrorToAnalyzerEnable;
    inFields[29]=entry.yellowMirrorToAnalyzerEnable;
    inFields[30]=entry.redMirrorToAnalyzerEnable;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],
                inFields[5],inFields[6],inFields[7],inFields[8],inFields[9],
                inFields[10],inFields[11],inFields[12],inFields[13],
                inFields[14],inFields[15],inFields[16],inFields[17],
                inFields[18],inFields[19],inFields[20],inFields[21],
                inFields[22],inFields[23],inFields[24],inFields[25],
                inFields[26],inFields[27],inFields[28],inFields[29],inFields[30]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*******************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetFirstV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    minMeteringEntryXcatIndex=(GT_U32)inArgs[1];
    maxMeteringEntryXcatIndex=(GT_U32)inArgs[2];

    meteringEntryXcatIndex=minMeteringEntryXcatIndex;

    return wrCpssDxCh3PolicerMeteringEntryGetEntryV2(inArgs,inFields,numFields,outArgs);

}
/*******************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerMeteringEntryGetNextV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    meteringEntryXcatIndex++;

    return wrCpssDxCh3PolicerMeteringEntryGetEntryV2(inArgs,inFields,numFields,outArgs);
}

/**************Table:cpssDxChXcatPolicerBillingEntry********************************/
static  GT_U32 minBillingEntryXcatIndex;
static  GT_U32 maxBillingEntryXcatIndex;
static  GT_U32 billingEntryXcatIndex;
/**
* @internal wrCpssDxCh3PolicerBillingEntrySetV1 function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntrySetV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 entryIndex;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    entryIndex=(GT_U32)inFields[0];
    billingCntr.greenCntr.l[0]=(GT_U32)inFields[1];
    billingCntr.greenCntr.l[1]=(GT_U32)inFields[2];
    billingCntr.yellowCntr.l[0]=(GT_U32)inFields[3];
    billingCntr.yellowCntr.l[1]=(GT_U32)inFields[4];
    billingCntr.redCntr.l[0]=(GT_U32)inFields[5];
    billingCntr.redCntr.l[1]=(GT_U32)inFields[6];
    billingCntr.billingCntrMode =
        (CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT)inFields[7];

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        billingCntr.billingCntrAllEnable = GT_FALSE;
        billingCntr.lmCntrCaptureMode =
            CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_COPY_E;
        billingCntr.greenCntrSnapshot.l[0] = 0;
        billingCntr.greenCntrSnapshot.l[1] = 0;
        billingCntr.greenCntrSnapshotValid = GT_FALSE;
        billingCntr.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        billingCntr.tunnelTerminationPacketSizeMode =
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E;
        billingCntr.dsaTagCountingMode =
            CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E;
        billingCntr.timeStampCountingMode =
            CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E;
    }

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerBillingEntrySet(devNum, entryIndex,
                                                     &billingCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerBillingEntryGetEntryV1 function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of Policer Tables non-direct access.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetEntryV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL reset;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    reset=(GT_BOOL)inArgs[1];

    if (billingEntryXcatIndex>maxBillingEntryXcatIndex||billingEntryXcatIndex>wrCpssDxChPolicerLastEntryGet(devNum))
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerBillingEntryGet(devNum, billingEntryXcatIndex,
                                                     reset, &billingCntr);

   if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0]=billingEntryXcatIndex;
    inFields[1]=billingCntr.greenCntr.l[0];
    inFields[2]=billingCntr.greenCntr.l[1];
    inFields[3]=billingCntr.yellowCntr.l[0];
    inFields[4]=billingCntr.yellowCntr.l[1];
    inFields[5]=billingCntr.redCntr.l[0];
    inFields[6]=billingCntr.redCntr.l[1];
    inFields[7]=billingCntr.billingCntrMode;

    fieldOutput("%d%d%d%d%d%d%d%d", inFields[0],inFields[1],inFields[2],
                inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*********************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetFirstV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    minBillingEntryXcatIndex=(GT_U32)inArgs[2];
    maxBillingEntryXcatIndex=(GT_U32)inArgs[3];
    billingEntryXcatIndex=minBillingEntryXcatIndex;

    return wrCpssDxCh3PolicerBillingEntryGetEntryV1(inArgs,inFields,numFields,outArgs);
}
/*********************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetNextV1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
     billingEntryXcatIndex++;

     return wrCpssDxCh3PolicerBillingEntryGetEntryV1(inArgs,inFields,numFields,outArgs);

}

/**
* @internal wrCpssDxCh3PolicerBillingEntrySetV2 function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntrySetV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 entryIndex;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    entryIndex=(GT_U32)inFields[0];
    billingCntr.greenCntr.l[0]=(GT_U32)inFields[1];
    billingCntr.greenCntr.l[1]=(GT_U32)inFields[2];
    billingCntr.yellowCntr.l[0]=(GT_U32)inFields[3];
    billingCntr.yellowCntr.l[1]=(GT_U32)inFields[4];
    billingCntr.redCntr.l[0]=(GT_U32)inFields[5];
    billingCntr.redCntr.l[1]=(GT_U32)inFields[6];
    billingCntr.billingCntrMode =
        (CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT)inFields[7];
    billingCntr.billingCntrAllEnable = (GT_BOOL)inFields[8];
    billingCntr.lmCntrCaptureMode =
        (CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_ENT)inFields[9];
    billingCntr.greenCntrSnapshot.l[0] = (GT_U32)inFields[10];
    billingCntr.greenCntrSnapshot.l[1] = (GT_U32)inFields[11];
    billingCntr.greenCntrSnapshotValid = (GT_BOOL)inFields[12];
    billingCntr.packetSizeMode =
        (CPSS_POLICER_PACKET_SIZE_MODE_ENT)inFields[13];
    billingCntr.tunnelTerminationPacketSizeMode =
        (CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT)inFields[14];
    billingCntr.dsaTagCountingMode =
        (CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT)inFields[15];
    billingCntr.timeStampCountingMode =
        (CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT)inFields[16];

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerBillingEntrySet(devNum, entryIndex,
                                                     &billingCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxCh3PolicerBillingEntryGetEntryV2 function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of Policer Tables non-direct access.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetEntryV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL reset;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    reset=(GT_BOOL)inArgs[1];

    if (billingEntryXcatIndex>maxBillingEntryXcatIndex||billingEntryXcatIndex>wrCpssDxChPolicerLastEntryGet(devNum))
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = pg_wrap_cpssDxCh3PolicerBillingEntryGet(devNum, billingEntryXcatIndex,
                                                     reset, &billingCntr);

   if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = billingEntryXcatIndex;
    inFields[1] = billingCntr.greenCntr.l[0];
    inFields[2] = billingCntr.greenCntr.l[1];
    inFields[3] = billingCntr.yellowCntr.l[0];
    inFields[4] = billingCntr.yellowCntr.l[1];
    inFields[5] = billingCntr.redCntr.l[0];
    inFields[6] = billingCntr.redCntr.l[1];
    inFields[7] = billingCntr.billingCntrMode;
    inFields[8] = billingCntr.billingCntrAllEnable;
    inFields[9] = billingCntr.lmCntrCaptureMode;
    inFields[10] = billingCntr.greenCntrSnapshot.l[0];
    inFields[11] = billingCntr.greenCntrSnapshot.l[1];
    inFields[12] = billingCntr.greenCntrSnapshotValid;
    inFields[13] = billingCntr.packetSizeMode;
    inFields[14] = billingCntr.tunnelTerminationPacketSizeMode;
    inFields[15] = billingCntr.dsaTagCountingMode;
    inFields[16] = billingCntr.timeStampCountingMode;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0],inFields[1],inFields[2],
                inFields[3],inFields[4],inFields[5],inFields[6],inFields[7],
                inFields[8],inFields[9],inFields[10],inFields[11],inFields[12],
                inFields[13],inFields[14],inFields[15],inFields[16]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}
/*********************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetFirstV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    minBillingEntryXcatIndex=(GT_U32)inArgs[2];
    maxBillingEntryXcatIndex=(GT_U32)inArgs[3];
    billingEntryXcatIndex=minBillingEntryXcatIndex;

    return wrCpssDxCh3PolicerBillingEntryGetEntryV2(inArgs,inFields,numFields,outArgs);
}
/*********************************************************************************/
static CMD_STATUS wrCpssDxCh3PolicerBillingEntryGetNextV2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
     billingEntryXcatIndex++;

     return wrCpssDxCh3PolicerBillingEntryGetEntryV2(inArgs,inFields,numFields,outArgs);

}

/**
* @internal wrCpssDxChPolicerCountingTriggerByPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Counting when port based metering is triggered for
*         the packet.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerCountingTriggerByPortEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerCountingTriggerByPortEnableSet(devNum, plrStage, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerCountingTriggerByPortEnableGet function
* @endinternal
*
* @brief   Get Counting trigger by port status.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerCountingTriggerByPortEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerCountingTriggerByPortEnableGet(devNum, plrStage, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerPortModeAddressSelectSet function
* @endinternal
*
* @brief   Configure Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or type.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerPortModeAddressSelectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                                                devNum;
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT     type;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    type = (CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerPortModeAddressSelectSet(devNum, plrStage, type);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPortModeAddressSelectGet function
* @endinternal
*
* @brief   Get Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerPortModeAddressSelectGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                               devNum;
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT    type;
    GT_STATUS                                           result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerPortModeAddressSelectGet(devNum, plrStage, &type);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", type);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEAttributesMeteringModeSet function
* @endinternal
*
* @brief   Set ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerEAttributesMeteringModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT     mode;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    stage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    mode = (CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPolicerEAttributesMeteringModeSet(devNum, stage, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerEAttributesMeteringModeGet function
* @endinternal
*
* @brief   Get ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerEAttributesMeteringModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT mode;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerEAttributesMeteringModeGet(devNum, stage, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/*******************************************************************************/
static void wrCpssDxChPolicerTriggerEntryBuild
(
    IN  GT_UINTPTR                                   inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR                                   inFields[CMD_MAX_FIELDS],
    OUT CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
)
{
    entryPtr->meteringEnable = (GT_BOOL)inFields[0];
    entryPtr->countingEnable = (GT_BOOL)inFields[1];
    entryPtr->policerIndex = (GT_U32)inFields[2];
    entryPtr->ucKnownEnable = (GT_BOOL)inFields[3];
    entryPtr->ucKnownOffset = (GT_U32)inFields[4];
    entryPtr->ucUnknownEnable = (GT_BOOL)inFields[5];
    entryPtr->ucUnknownOffset = (GT_U32)inFields[6];
    entryPtr->mcRegisteredEnable = (GT_BOOL)inFields[7];
    entryPtr->mcRegisteredOffset = (GT_U32)inFields[8];
    entryPtr->mcUnregisteredEnable = (GT_BOOL)inFields[9];
    entryPtr->mcUnregisteredOffset = (GT_U32)inFields[10];
    entryPtr->bcEnable = (GT_BOOL)inFields[11];
    entryPtr->bcOffset = (GT_U32)inFields[12];
    entryPtr->tcpSynEnable = (GT_BOOL)inFields[13];
    entryPtr->tcpSynOffset = (GT_U32)inFields[14];
}

/*******************************************************************************/
static CMD_STATUS wrCpssDxChPolicerTriggerEntryParse
(
    IN  GT_UINTPTR                                   inFields[CMD_MAX_FIELDS],
    IN  GT_32                                   triggerEntryIndex,
    IN CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC      entry,
    OUT GT_8                                    outArgs[CMD_MAX_BUFFER]
)
{
   inFields[0] = entry.meteringEnable;
   inFields[1] = entry.countingEnable;
   inFields[2] = entry.policerIndex;
   inFields[3] = entry.ucKnownEnable;
   inFields[4] = entry.ucKnownOffset;
   inFields[5] = entry.ucUnknownEnable;
   inFields[6] = entry.ucUnknownOffset;
   inFields[7] = entry.mcRegisteredEnable;
   inFields[8] = entry.mcRegisteredOffset;
   inFields[9] = entry.mcUnregisteredEnable;
   inFields[10] = entry.mcUnregisteredOffset;
   inFields[11] = entry.bcEnable;
   inFields[12] = entry.bcOffset;
   inFields[13] = entry.tcpSynEnable;
   inFields[14] = entry.tcpSynOffset;

   fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
               inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],
               inFields[5],inFields[6],inFields[7],inFields[8],inFields[9],
               inFields[10],inFields[11],inFields[12],inFields[13],
               inFields[14]);

   /* pack output arguments to galtis string */
   galtisOutput(outArgs, GT_OK, "%f");

   return CMD_OK;
}

/**************Table:wrCpssDxChPolicerTriggerEntrySet********************************/
/**
* @internal wrCpssDxChPolicerTriggerEntrySet function
* @endinternal
*
* @brief   Set Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on table index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerTriggerEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                   devNum;
    GT_STATUS                               result;
    GT_U32                                  entryIndex;
    CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     entry;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    entryIndex = (GT_U32)inArgs[2];

    cmdOsMemSet(&entry, 0, sizeof(entry));

    /* build the entry */
    wrCpssDxChPolicerTriggerEntryBuild(inArgs,inFields,&entry);

    /* call cpss api function */
    result = cpssDxChPolicerTriggerEntrySet(devNum, stage,
                                            entryIndex, &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerTriggerEntryGetEntry function
* @endinternal
*
* @brief   Get Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on table index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerTriggerEntryGetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     entry;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage;
    GT_U32                                  triggerEntryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

   /* map input arguments to locals */
   devNum = (GT_U8)inArgs[0];
   stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
   triggerEntryIndex = (GT_U32)inArgs[2];

   /* reset entry to avoid trash in output for not used fields */
   cmdOsMemSet(&entry, 0, sizeof(entry));

   /* call cpss api function */
   result = cpssDxChPolicerTriggerEntryGet(devNum, stage,
                                           triggerEntryIndex,
                                           &entry);

   if (result != GT_OK)
   {
       galtisOutput(outArgs, result, "%d", -1);
       return CMD_OK;
   }
   cmdOsMemSet(inFields,0,CMD_MAX_FIELDS);

   return wrCpssDxChPolicerTriggerEntryParse(inFields,
                                             triggerEntryIndex,
                                             entry,
                                             outArgs);
}

/*******************************************************************************/
static CMD_STATUS wrCpssDxChPolicerTriggerEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPolicerTriggerEntryGetEntry(inArgs,inFields,numFields,outArgs);

}
/*******************************************************************************/
static CMD_STATUS wrCpssDxChPolicerTriggerEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, CMD_OK, "%d", -1);
    return CMD_OK;
}


/**** database initialization **************************************/

/**
* @internal wrCpssDxChPolicerGetNext function
* @endinternal
*
* @brief   Default getNext Function to stop loop.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex.
* @retval GT_TIMEOUT               - on time out.
*/
static CMD_STATUS wrCpssDxChPolicerGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%d", -1);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerFlowIdCountingCfgSet function
* @endinternal
*
* @brief   Set Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerFlowIdCountingCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage;
    CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC billingIndexCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    billingIndexCfg.billingIndexMode       =
        (CPSS_DXCH_POLICER_BILLING_INDEX_MODE_ENT)inArgs[2];
    billingIndexCfg.billingFlowIdIndexBase = (GT_U32)inArgs[3];
    billingIndexCfg.billingMinFlowId       = (GT_U32)inArgs[4];
    billingIndexCfg.billingMaxFlowId       = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssDxChPolicerFlowIdCountingCfgSet(
        devNum, stage, &billingIndexCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerFlowIdCountingCfgGet function
* @endinternal
*
* @brief   Get Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerFlowIdCountingCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage;
    CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC billingIndexCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    stage  = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    cpssOsMemSet(&billingIndexCfg, 0, sizeof(billingIndexCfg));

    /* call cpss api function */
    result = cpssDxChPolicerFlowIdCountingCfgGet(
        devNum, stage, &billingIndexCfg);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d %d %d %d",
        billingIndexCfg.billingIndexMode,
        billingIndexCfg.billingFlowIdIndexBase,
        billingIndexCfg.billingMinFlowId,
        billingIndexCfg.billingMaxFlowId);

    return CMD_OK;
}

/* cpssDxChPolicerHierarchicalTable */

/* current relative index for getNext - cleared by getFirst */
static GT_U32 prvCpssDxChPolicerHierarchicalTable_relativeIndexForGet = 0xFFFF;

/**
* @internal wrCpssDxChPolicerHierarchicalTableSet function
* @endinternal
*
* @brief   Set Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
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
static CMD_STATUS wrCpssDxChPolicerHierarchicalTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           devNum;
    GT_U32                                          index;
    CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    cpssOsMemSet(&entry, 0, sizeof(entry));
    devNum               = (GT_U8)inArgs[0];
    index                = (GT_U32)inFields[0];
    entry.countingEnable = (GT_BOOL)inFields[1];
    entry.meteringEnable = (GT_BOOL)inFields[2];
    entry.policerPointer = (GT_U32)inFields[3];

    /* call cpss api function */
    result = cpssDxChPolicerHierarchicalTableEntrySet(
        devNum, index, &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerHierarchicalTableGetNext function
* @endinternal
*
* @brief   Get Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
static CMD_STATUS wrCpssDxChPolicerHierarchicalTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           devNum;
    GT_U32                                          baseIndex;
    GT_U32                                          amount;
    GT_U32                                          index;
    CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    cpssOsMemSet(&entry, 0, sizeof(entry));
    devNum     = (GT_U8)inArgs[0];
    baseIndex  = (GT_U32)inArgs[1];
    amount     = (GT_U32)inArgs[2];

    if (prvCpssDxChPolicerHierarchicalTable_relativeIndexForGet >= amount)
    {
        /* end of required entries range */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    index =
        baseIndex + prvCpssDxChPolicerHierarchicalTable_relativeIndexForGet;
    prvCpssDxChPolicerHierarchicalTable_relativeIndexForGet ++;

    /* call cpss api function */
    result = cpssDxChPolicerHierarchicalTableEntryGet(
        devNum, index, &entry);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d", index, entry.countingEnable,
        entry.meteringEnable, entry.policerPointer);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPolicerHierarchicalTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    prvCpssDxChPolicerHierarchicalTable_relativeIndexForGet = 0;

    return wrCpssDxChPolicerHierarchicalTableGetNext(
        inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssDxChPolicerSecondStageIndexMaskSet function
* @endinternal
*
* @brief   Sets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on illegal plrIndexMask
*
* @note Note: Lion device applicable starting from revision B2
*
*/
static CMD_STATUS wrCpssDxChPolicerSecondStageIndexMaskSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    GT_U32              plrIndexMask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    plrIndexMask = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerSecondStageIndexMaskSet(devNum, plrIndexMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerSecondStageIndexMaskGet function
* @endinternal
*
* @brief   Gets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Note: Lion device applicable starting from revision B2
*
*/
static CMD_STATUS wrCpssDxChPolicerSecondStageIndexMaskGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    GT_U32              plrIndexMask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerSecondStageIndexMaskGet(devNum, &plrIndexMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", plrIndexMask);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerTrappedPacketsBillingEnableSet function
* @endinternal
*
* @brief   Enables the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerTrappedPacketsBillingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerTrappedPacketsBillingEnableSet(devNum, plrStage, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerTrappedPacketsBillingEnableGet function
* @endinternal
*
* @brief   Get state of the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerTrappedPacketsBillingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerTrappedPacketsBillingEnableGet(devNum, plrStage, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerLossMeasurementCounterCaptureEnableSet function
* @endinternal
*
* @brief   Enables or disables Loss Measurement capture.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerLossMeasurementCounterCaptureEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerLossMeasurementCounterCaptureEnableSet(devNum, plrStage, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerLossMeasurementCounterCaptureEnableGet function
* @endinternal
*
* @brief   Get Loss Measurement capture state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerLossMeasurementCounterCaptureEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerLossMeasurementCounterCaptureEnableGet(devNum, plrStage, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPortStormTypeInFlowModeEnableSet function
* @endinternal
*
* @brief   Enable port-based metering triggered in Flow mode to work in
*         (storm-pkt-type, port) based, otherwise (port) based is used.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerPortStormTypeInFlowModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT plrStage;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    plrStage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPolicerPortStormTypeInFlowModeEnableSet(devNum, plrStage, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPortStormTypeInFlowModeEnableGet function
* @endinternal
*
* @brief   Get if port-based metering triggered in Flow mode is enabled for
*         (storm-pkt-type, port) based (otherwise (port) based is used).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerPortStormTypeInFlowModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT plrStage;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    plrStage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerPortStormTypeInFlowModeEnableGet(devNum, plrStage, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPacketSizeModeForTimestampTagSet function
* @endinternal
*
* @brief   Determines whether the timestamp tag should be included in the byte count
*         computation or not.
*         The configuration is used for IPFIX, VLAN and Management counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or timestampTagMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerPacketSizeModeForTimestampTagSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerPacketSizeModeForTimestampTagSet(devNum, plrStage, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerPacketSizeModeForTimestampTagGet function
* @endinternal
*
* @brief   Retrieves whether the timestamp tag should be included in the byte count
*         computation or not.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static CMD_STATUS wrCpssDxChPolicerPacketSizeModeForTimestampTagGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerPacketSizeModeForTimestampTagGet(devNum, plrStage, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
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
static CMD_STATUS wrCpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
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
static CMD_STATUS wrCpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
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
static CMD_STATUS wrCpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
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
static CMD_STATUS wrCpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/* MEF10.3 commands start */

/* static valiables for table state mashines */
static GT_U32                                 bucketEnvelop_entryIndex;
static CPSS_DXCH3_POLICER_METERING_ENTRY_STC  bucketEnvelop_entryArr[8];
static CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT bucketEnvelop_tbParamsArr[8];
static const GT_U32                           bucketEnvelop_entryIndexLimit = 8;
static GT_U32                                 bucketEnvelop_envelopeSize;
static GT_BOOL                                bucketEnvelop_couplingFlag0;
static GT_U32                                 bucketEnvelop_startQosProfileIndex;
static GT_U32                                 bucketEnvelop_numOfQosProfiles;
static GT_U32                                 bucketEnvelop_maxRateEntryIndex;
static GT_U32                                 bucketEnvelop_maxRateBucketIndex;
static GT_U32                                 bucketEnvelop_maxRateNumOfEntries;
static GT_U32                                 bucketEnvelop_maxRateBucketMask;

/* retrieve fields from command line */
static void prvWrCpssDxChPolicerMeteringEntryEnvelopeFieldsFromCmdLine
(
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS]
)
{
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryPtr;

    entryPtr = &(bucketEnvelop_entryArr[bucketEnvelop_entryIndex]);

    entryPtr->countingEntryIndex=(GT_U32)inFields[0];
    entryPtr->mngCounterSet=(CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT)inFields[1];
    entryPtr->meterColorMode=(CPSS_POLICER_COLOR_MODE_ENT)inFields[2];
    entryPtr->meterMode = (bucketEnvelop_entryIndex == 0)
        ? CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E
        : CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E;
    entryPtr->tokenBucketParams.envelope.cir    =(GT_U32)inFields[3];
    entryPtr->tokenBucketParams.envelope.cbs    =(GT_U32)inFields[4];
    entryPtr->tokenBucketParams.envelope.eir    =(GT_U32)inFields[5];
    entryPtr->tokenBucketParams.envelope.ebs    =(GT_U32)inFields[6];
    entryPtr->tokenBucketParams.envelope.maxCir =(GT_U32)inFields[7];
    entryPtr->tokenBucketParams.envelope.maxEir =(GT_U32)inFields[8];
    entryPtr->modifyUp = (CPSS_DXCH_POLICER_MODIFY_UP_ENT)inFields[9];
    entryPtr->modifyDscp = (CPSS_DXCH_POLICER_MODIFY_DSCP_ENT)inFields[10];
    entryPtr->modifyDp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[11];
    entryPtr->modifyExp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[12];
    entryPtr->modifyTc = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[13];
    entryPtr->greenPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[14];
    entryPtr->yellowPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[15];
    entryPtr->redPcktCmd = (CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT)inFields[16];
    entryPtr->qosProfile = (GT_U32)inFields[17];
    entryPtr->remarkMode = (CPSS_DXCH_POLICER_REMARK_MODE_ENT)inFields[18];

    entryPtr->byteOrPacketCountingMode =
                        (CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT)inFields[19];
    entryPtr->packetSizeMode = (CPSS_POLICER_PACKET_SIZE_MODE_ENT)inFields[20];
    entryPtr->tunnelTerminationPacketSizeMode =
                        (CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT)inFields[21];
    entryPtr->dsaTagCountingMode =
                        (CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT)inFields[22];
    entryPtr->timeStampCountingMode =
                        (CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT)inFields[23];
    entryPtr->yellowEcnMarkingEnable = (GT_BOOL)inFields[24];
    entryPtr->couplingFlag = (GT_BOOL)inFields[25];
    entryPtr->maxRateIndex = (GT_U32)inFields[26];
    entryPtr->greenMirrorToAnalyzerEnable = (GT_BOOL)inFields[27];
    entryPtr->yellowMirrorToAnalyzerEnable = (GT_BOOL)inFields[28];
    entryPtr->redMirrorToAnalyzerEnable = (GT_BOOL)inFields[29];
}

/* put fields to reply line */
static void prvWrCpssDxChPolicerMeteringEntryEnvelopeFieldsToReplyLine()
{
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryPtr;

    entryPtr = &(bucketEnvelop_entryArr[bucketEnvelop_entryIndex]);

    fieldOutput(
        "%d%d%d",
        (GT_U32)entryPtr->countingEntryIndex,
        (GT_U32)entryPtr->mngCounterSet,
        (GT_U32)entryPtr->meterColorMode);

    fieldOutputSetAppendMode();
    fieldOutput(
        "%d%d%d%d%d%d",
        (GT_U32)entryPtr->tokenBucketParams.envelope.cir,
        (GT_U32)entryPtr->tokenBucketParams.envelope.cbs,
        (GT_U32)entryPtr->tokenBucketParams.envelope.eir,
        (GT_U32)entryPtr->tokenBucketParams.envelope.ebs,
        (GT_U32)entryPtr->tokenBucketParams.envelope.maxCir,
        (GT_U32)entryPtr->tokenBucketParams.envelope.maxEir);

    fieldOutputSetAppendMode();
    fieldOutput(
        "%d%d%d%d%d",
        (GT_U32)entryPtr->modifyUp,
        (GT_U32)entryPtr->modifyDscp,
        (GT_U32)entryPtr->modifyDp,
        (GT_U32)entryPtr->modifyExp,
        (GT_U32)entryPtr->modifyTc);

    fieldOutputSetAppendMode();
    fieldOutput(
        "%d%d%d%d%d",
        (GT_U32)entryPtr->greenPcktCmd,
        (GT_U32)entryPtr->yellowPcktCmd,
        (GT_U32)entryPtr->redPcktCmd,
        (GT_U32)entryPtr->qosProfile,
        (GT_U32)entryPtr->remarkMode);

    fieldOutputSetAppendMode();
    fieldOutput(
        "%d%d%d%d%d%d",
        (GT_U32)entryPtr->byteOrPacketCountingMode,
        (GT_U32)entryPtr->packetSizeMode,
        (GT_U32)entryPtr->tunnelTerminationPacketSizeMode,
        (GT_U32)entryPtr->dsaTagCountingMode,
        (GT_U32)entryPtr->timeStampCountingMode,
        (GT_U32)entryPtr->yellowEcnMarkingEnable);

    fieldOutputSetAppendMode();
    fieldOutput(
        "%d%d%d%d%d",
        (GT_U32)entryPtr->couplingFlag,
        (GT_U32)entryPtr->maxRateIndex,
        (GT_U32)entryPtr->greenMirrorToAnalyzerEnable,
        (GT_U32)entryPtr->yellowMirrorToAnalyzerEnable,
        (GT_U32)entryPtr->redMirrorToAnalyzerEnable);
}

/* save first command line data */
static CMD_STATUS wrCpssDxChPolicerMeteringEntryEnvelopeSetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    cpssOsMemSet(bucketEnvelop_entryArr, 0, sizeof(bucketEnvelop_entryArr));
    cpssOsMemSet(bucketEnvelop_tbParamsArr, 0, sizeof(bucketEnvelop_tbParamsArr));
    bucketEnvelop_entryIndex = 0;
    prvWrCpssDxChPolicerMeteringEntryEnvelopeFieldsFromCmdLine(inFields);
    bucketEnvelop_entryIndex ++;
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/* save next command lines data */
static CMD_STATUS wrCpssDxChPolicerMeteringEntryEnvelopeSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if (bucketEnvelop_entryIndex >= bucketEnvelop_entryIndexLimit)
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "%d", -1);
        return CMD_OK;
    }
    prvWrCpssDxChPolicerMeteringEntryEnvelopeFieldsFromCmdLine(inFields);
    bucketEnvelop_entryIndex ++;
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/* save last command line data and store to HW */
static CMD_STATUS wrCpssDxChPolicerMeteringEntryEnvelopeEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage;
    GT_U32                                      startEntryIndex;
    GT_BOOL                                     couplingFlag0;
    GT_STATUS                                   rc;

    devNum             = (GT_U8)                           inArgs[0];
    stage              = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    startEntryIndex    = (GT_U32)                          inArgs[2];
    couplingFlag0      = (GT_BOOL)                         inArgs[3];

    rc = cpssDxChPolicerMeteringEntryEnvelopeSet(
        devNum, stage, startEntryIndex,
        bucketEnvelop_entryIndex /*envelopeSize*/, couplingFlag0,
        bucketEnvelop_entryArr, bucketEnvelop_tbParamsArr);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/* Retrieve all envelope from HW, send first line to reply line */
static CMD_STATUS wrCpssDxChPolicerMeteringEntryEnvelopeGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage;
    GT_U32                                      startEntryIndex;
    GT_STATUS                                   rc;

    devNum             = (GT_U8)                           inArgs[0];
    stage              = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    startEntryIndex    = (GT_U32)                          inArgs[2];

    cpssOsMemSet(bucketEnvelop_entryArr, 0, sizeof(bucketEnvelop_entryArr));
    bucketEnvelop_entryIndex = 0;

    rc = cpssDxChPolicerMeteringEntryEnvelopeGet(
        devNum, stage, startEntryIndex,
        8 /*maxEnvelopeSize*/,
        &bucketEnvelop_envelopeSize /*envelopeSizePtr*/,
        &bucketEnvelop_couplingFlag0 /*couplingFlag0Ptr*/,
        bucketEnvelop_entryArr);

    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "%d", -1);
        return CMD_OK;
    }

    prvWrCpssDxChPolicerMeteringEntryEnvelopeFieldsToReplyLine();
    bucketEnvelop_entryIndex ++;

    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/* send first next to reply line */
static CMD_STATUS wrCpssDxChPolicerMeteringEntryEnvelopeGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if (bucketEnvelop_entryIndex >= bucketEnvelop_envelopeSize)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);

        cmdOsPrintf(
            "\nenvelopeSize: %d, couplingFlag0: %d\n",
            bucketEnvelop_envelopeSize, bucketEnvelop_couplingFlag0);

        return CMD_OK;
    }

    prvWrCpssDxChPolicerMeteringEntryEnvelopeFieldsToReplyLine();
    bucketEnvelop_entryIndex ++;

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMeterTableFlowBasedIndexConfigSet function
* @endinternal
*
* @brief   Set Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPolicerMeterTableFlowBasedIndexConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_U8                                    devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage;
    GT_U32                                   threshold;
    CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT  maxSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)                                  inArgs[0];
    stage     = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)       inArgs[1];
    threshold = (GT_U32)                                 inArgs[2];
    maxSize   = (CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPolicerMeterTableFlowBasedIndexConfigSet(
        devNum, stage, threshold, maxSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMeterTableFlowBasedIndexConfigGet function
* @endinternal
*
* @brief   Get Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong HW data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPolicerMeterTableFlowBasedIndexConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_U8                                    devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage;
    GT_U32                                   threshold;
    CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT  maxSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)                                  inArgs[0];
    stage     = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)       inArgs[1];
    threshold = (GT_U32)                                 inArgs[2];
    maxSize   = (CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPolicerMeterTableFlowBasedIndexConfigGet(
        devNum, stage, &threshold, &maxSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", threshold, maxSize);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerQosProfileToPriorityMapSet function
* @endinternal
*
* @brief   Set entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChPolicerQosProfileToPriorityMapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_U8                              devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT   stage;
    GT_U32                             qosProfileIndex;
    GT_U32                             priority;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)                             inArgs[0];
    stage           = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)  inArgs[1];
    qosProfileIndex = (GT_U32)                            inFields[0];
    priority        = (GT_U32)                            inFields[1];

    /* call cpss api function */
    result = cpssDxChPolicerQosProfileToPriorityMapSet(
        devNum, stage, qosProfileIndex, priority);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal prvWrCpssDxChPolicerQosProfileToPriorityMapGet function
* @endinternal
*
* @brief   Get entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS prvWrCpssDxChPolicerQosProfileToPriorityMapGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_U8                                    devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage;
    GT_U32                                   priority;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)                             inArgs[0];
    stage           = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)  inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerQosProfileToPriorityMapGet(
        devNum, stage, bucketEnvelop_startQosProfileIndex, &priority);

    /* pack output arguments to galtis string */
    fieldOutput("%d%d", bucketEnvelop_startQosProfileIndex, priority);
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPolicerQosProfileToPriorityMapGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    bucketEnvelop_startQosProfileIndex = (GT_U32)inArgs[2];
    bucketEnvelop_numOfQosProfiles     = (GT_U32)inArgs[3];

    if (bucketEnvelop_numOfQosProfiles == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    return prvWrCpssDxChPolicerQosProfileToPriorityMapGet(
        inArgs, inFields, numFields, outArgs);
}

static CMD_STATUS wrCpssDxChPolicerQosProfileToPriorityMapGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    bucketEnvelop_startQosProfileIndex ++;
    bucketEnvelop_numOfQosProfiles     --;

    if (bucketEnvelop_numOfQosProfiles == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    return prvWrCpssDxChPolicerQosProfileToPriorityMapGet(
        inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssDxChPolicerTokenBucketMaxRateSet function
* @endinternal
*
* @brief   Set the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See MEF10.3 examples in FS to know relation between bucket0/1 and Commited/Exess
*
*/
static CMD_STATUS wrCpssDxChPolicerTokenBucketMaxRateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_U8                                    devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage;
    GT_U32                                   entryIndex;
    GT_U32                                   bucketIndex;
    GT_U32                                   meterEntryRate;
    GT_U32                                   meterEntryBurstSize;
    GT_U32                                   maxRate;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                  = (GT_U8)                             inArgs[0];
    stage                   = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)  inArgs[1];
    meterEntryRate          = (GT_U32)                            inArgs[2];
    meterEntryBurstSize     = (GT_U32)                            inArgs[3];
    entryIndex              = (GT_U32)                            inFields[0];
    bucketIndex             = (GT_U32)                            inFields[1];
    maxRate                 = (GT_U32)                            inFields[2];

    /* call cpss api function */
    result = cpssDxChPolicerTokenBucketMaxRateSet(
        devNum, stage, entryIndex, bucketIndex,
        meterEntryRate, meterEntryBurstSize, maxRate);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal prvWrCpssDxChPolicerTokenBucketMaxRateGet function
* @endinternal
*
* @brief   Get the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS prvWrCpssDxChPolicerTokenBucketMaxRateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_U8                                    devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage;
    GT_U32                                   meterEntryRate;
    GT_U32                                   meterEntryBurstSize;
    GT_U32                                   maxRate;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                  = (GT_U8)                             inArgs[0];
    stage                   = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)  inArgs[1];
    meterEntryRate          = (GT_U32)                            inArgs[2];
    meterEntryBurstSize     = (GT_U32)                            inArgs[3];

    /* call cpss api function */
    result = cpssDxChPolicerTokenBucketMaxRateGet(
        devNum, stage,
        bucketEnvelop_maxRateEntryIndex, bucketEnvelop_maxRateBucketIndex,
        meterEntryRate, meterEntryBurstSize, &maxRate);

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d", bucketEnvelop_maxRateEntryIndex,
        bucketEnvelop_maxRateBucketIndex, maxRate);
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/* state mashine for next table line                                           */
/* bucketEnvelop_maxRateBucketIndex >= 32 means look for first bit in the mask */
/* bucketEnvelop_maxRateNumOfEntries == 0 means the end of table               */
static void prvWrCpssDxChPolicerTokenBucketMaxRateNextIndex()
{
    GT_U32 i;
    GT_U32 firstBit;

    if (bucketEnvelop_maxRateBucketMask == 0)
    {
        /* probably error, detected by caller */
        bucketEnvelop_maxRateNumOfEntries = 0;
        return;
    }
    if (bucketEnvelop_maxRateNumOfEntries == 0)
    {
        return;
    }
    firstBit = (bucketEnvelop_maxRateBucketIndex + 1);
    if (firstBit >= 32)
    {
        firstBit = 0;
    }
    for (i = firstBit; (i < 32); i++)
    {
        if ((1 << i) & bucketEnvelop_maxRateBucketMask)
        {
            /* next bucket in the same entry */
            bucketEnvelop_maxRateBucketIndex = i;
            return;
        }
    }
    /* (i >= 32) ==> pass to next line */
    bucketEnvelop_maxRateEntryIndex ++;
    bucketEnvelop_maxRateNumOfEntries --;
    if (bucketEnvelop_maxRateNumOfEntries == 0)
    {
        return;
    }
    for (i = 0; (i < 32); i++)
    {
        if ((1 << i) & bucketEnvelop_maxRateBucketMask)
        {
            /* next bucket in the same entry */
            bucketEnvelop_maxRateBucketIndex = i;
            return;
        }
    }
}

static CMD_STATUS wrCpssDxChPolicerTokenBucketMaxRateGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    prvWrCpssDxChPolicerTokenBucketMaxRateNextIndex();
    if (bucketEnvelop_maxRateNumOfEntries == 0)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    return prvWrCpssDxChPolicerTokenBucketMaxRateGet(
        inArgs, inFields, numFields, outArgs);
}

static CMD_STATUS wrCpssDxChPolicerTokenBucketMaxRateGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    bucketEnvelop_maxRateEntryIndex    = (GT_U32)inArgs[4];
    bucketEnvelop_maxRateNumOfEntries  = (GT_U32)inArgs[5];
    bucketEnvelop_maxRateBucketMask    = (GT_U32)inArgs[6];
    bucketEnvelop_maxRateBucketIndex   = 0xFF;

    return wrCpssDxChPolicerTokenBucketMaxRateGetNext(
        inArgs, inFields, numFields, outArgs);
}

/* MEF10.3 commands end */

/* current number of line*/
static GT_U32 currentBucketsState_relativeIndex = 0;

static CMD_STATUS wrCpssDxChPolicerBucketsCurrentStateGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8                                       devNum;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage;
    CPSS_DXCH_POLICER_BUCKETS_STATE_STC         bucketsState;
    GT_U32                                      baseIndex;
    GT_U32                                      numOfEntries;

    devNum       = (GT_U8)inArgs[0];
    stage        = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    baseIndex    = (GT_U32)inArgs[2];
    numOfEntries = (GT_U32)inArgs[3];

    if (currentBucketsState_relativeIndex >= numOfEntries)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    rc = pg_wrap_cpssDxChPolicerBucketsCurrentStateGet(
        devNum, stage,
        (baseIndex + currentBucketsState_relativeIndex),
        &bucketsState);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d%d%d%d",
       bucketsState.lastTimeUpdate0,
       bucketsState.lastTimeUpdate1,
       bucketsState.wrapAround0,
       bucketsState.wrapAround1,
       bucketsState.bucketSize0,
       bucketsState.bucketSize1,
       bucketsState.bucketSignPositive0,
       bucketsState.bucketSignPositive1);
    galtisOutput(outArgs, GT_OK, "%f");

    currentBucketsState_relativeIndex ++;

    return CMD_OK;
}
static CMD_STATUS wrCpssDxChPolicerBucketsCurrentStateGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    currentBucketsState_relativeIndex = 0;

    return wrCpssDxChPolicerBucketsCurrentStateGetNext(
        inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssDxChPolicerMeteringAnalyzerIndexSet function
* @endinternal
*
* @brief   Set analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerMeteringAnalyzerIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               dev;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage;
    CPSS_DP_LEVEL_ENT                   color;
    GT_BOOL                             enable;
    GT_U32                              index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    stage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    color = (CPSS_DP_LEVEL_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];
    index =  (GT_U32)inArgs[4];   

    /* call cpss api function */
    result = cpssDxChPolicerMeteringAnalyzerIndexSet(dev, stage, color, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPolicerMeteringAnalyzerIndexGet function
* @endinternal
*
* @brief   Get analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPolicerMeteringAnalyzerIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                               dev;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage;
    CPSS_DP_LEVEL_ENT                   color;
    GT_BOOL                             enable;
    GT_U32                              index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    stage = (CPSS_DXCH_POLICER_STAGE_TYPE_ENT)inArgs[1];
    color = (CPSS_DP_LEVEL_ENT)inArgs[2];   

    /* call cpss api function */
    result = cpssDxChPolicerMeteringAnalyzerIndexGet(dev, stage, color, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

static CMD_COMMAND dbCommands[] =
{
        {"cpssDxCh3PolicerMeteringEnableSet",
         &wrCpssDxCh3PolicerMeteringEnableSet,
         2, 0},
        {"cpssDxCh3PolicerMeteringEnableGet",
         &wrCpssDxCh3PolicerMeteringEnableGet,
         1, 0},
        {"cpssDxCh3PolicerBillingCountingEnableSet",
         &wrCpssDxCh3PolicerBillingCountingEnableSet,
         2, 0},
        {"cpssDxCh3PolicerBillingCountingEnableGet",
         &wrCpssDxCh3PolicerBillingCountingEnableGet,
         1, 0},
        {"cpssDxCh3PolicerBillingCountersModeSet",
         &wrCpssDxCh3PolicerBillingCountersModeSet,
         2, 0},
        {"cpssDxCh3PolicerBillingCountersModeGet",
         &wrCpssDxCh3PolicerBillingCountersModeGet,
         1, 0},
        {"cpssDxCh3PolicerPacketSizeModeSet",
         &wrCpssDxCh3PolicerPacketSizeModeSet,
         2, 0},
        {"cpssDxCh3PolicerPacketSizeModeGet",
         &wrCpssDxCh3PolicerPacketSizeModeGet,
         1, 0},
        {"cpssDxCh3PolicerMeterResolutionSet",
         &wrCpssDxCh3PolicerMeterResolutionSet,
         2, 0},
        {"cpssDxCh3PolicerMeterResolutionGet",
         &wrCpssDxCh3PolicerMeterResolutionGet,
         1, 0},
        {"cpssDxCh3PolicerDropTypeSet",
         &wrCpssDxCh3PolicerDropTypeSet,
         2, 0},
        {"cpssDxCh3PolicerDropTypeGet",
         &wrCpssDxCh3PolicerDropTypeGet,
         1, 0},
        {"cpssDxCh3PolicerCountingColorModeSet",
         &wrCpssDxCh3PolicerCountingColorModeSet,
         2, 0},
        {"cpssDxCh3PolicerCountingColorModeGet",
         &wrCpssDxCh3PolicerCountingColorModeGet,
         1, 0},
        {"cpssDxCh3PolicerManagementCntrsResolutionSet",
         &wrCpssDxCh3PolicerManagementCntrsResolutionSet,
         3, 0},
        {"cpssDxCh3PolicerManagementCntrsResolutionGet",
         &wrCpssDxCh3PolicerManagementCntrsResolutionGet,
         2, 0},
        {"cpssDxCh3PolicerPacketSizeModeForTunnelTermSet",
         &wrCpssDxCh3PolicerPacketSizeModeForTunnelTermSet,
         2, 0},
        {"cpssDxCh3PolicerPacketSizeModeForTunnelTermGet",
         &wrCpssDxCh3PolicerPacketSizeModeForTunnelTermGet,
         1, 0},
        {"cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet",
         &wrCpssDxCh3PolicerMeteringAutoRefreshScanEnableSet,
         2, 0},
        {"cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet",
         &wrCpssDxCh3PolicerMeteringAutoRefreshScanEnableGet,
         1, 0},
        {"cpssDxCh3PolicerMeteringAutoRefreshRangeSet",
         &wrCpssDxCh3PolicerMeteringAutoRefreshRangeSet,
         3, 0},
        {"cpssDxCh3PolicerMeteringAutoRefreshRangeGet",
         &wrCpssDxCh3PolicerMeteringAutoRefreshRangeGet,
         1, 0},
        {"cpssDxCh3PolicerMeteringAutoRefreshIntervalSet",
         &wrCpssDxCh3PolicerMeteringAutoRefreshIntervalSet,
         2, 0},
        {"cpssDxCh3PolicerMeteringAutoRefreshIntervalGet",
         &wrCpssDxCh3PolicerMeteringAutoRefreshIntervalGet,
         1, 0},
        {"cpssDxCh3PolicerMeterEntryRefresh",
         &wrCpssDxCh3PolicerMeteringEntryRefresh,
         2, 0},
        {"cpssDxCh3PolicerPortMeteringEnableSet",
         &wrCpssDxCh3PolicerPortMeteringEnableSet,
         3, 0},
        {"cpssDxCh3PolicerPortMeteringEnableGet",
         &wrCpssDxCh3PolicerPortMeteringEnableGet,
         2, 0},
        {"cpssDxCh3PolicerMruSet",
         &wrCpssDxCh3PolicerMruSet,
         2, 0},
        {"cpssDxCh3PolicerMruGet",
         &wrCpssDxCh3PolicerMruGet,
         1, 0},
        {"cpssDxCh3PolicerErrorGet",
         &wrCpssDxCh3PolicerErrorGet,
         1, 0},
        {"cpssDxCh3PolicerErrorCounterGet",
         &wrCpssDxCh3PolicerErrorCounterGet,
         1, 0},
        {"cpssDxCh3PolicerManagementCountersSet",
         &wrCpssDxCh3PolicerManagementCountersSet,
         5, 0},
        {"cpssDxCh3PolicerManagementCountersGet",
         &wrCpssDxCh3PolicerManagementCountersGet,
         3, 0},
        {"cpssDxCh3PolicerMeteringEntrySet",
         &wrCpssDxCh3PolicerMeteringEntrySet,
         3, 16},
        {"cpssDxCh3PolicerMeteringEntryGetFirst",
         &wrCpssDxCh3PolicerMeteringEntryGetFirst,
         3, 0},
        {"cpssDxCh3PolicerMeteringEntryGetNext",
         &wrCpssDxCh3PolicerMeteringEntryGetNext,
         3, 0},
        {"cpssDxCh3PolicerEntryMeterParamsCalculate",
         &wrCpssDxCh3PolicerEntryMeterParamsCalculate,
         9, 0},
        {"cpssDxCh3PolicerBillingEntrySet",
         &wrCpssDxCh3PolicerBillingEntrySet,
         4, 4},
        {"cpssDxCh3PolicerBillingEntryGetFirst",
         &wrCpssDxCh3PolicerBillingEntryGetFirst,
         4, 0},
        {"cpssDxCh3PolicerBillingEntryGetNext",
         &wrCpssDxCh3PolicerBillingEntryGetNext,
         4, 0},
        {"cpssDxCh3PolicerQosRemarkingEntrySet",
         &wrCpssDxCh3PolicerQosRemarkingEntrySet,
         3, 4},
        {"cpssDxCh3PolicerQosRemarkingEntryGetFirst",
         &wrCpssDxCh3PolicerQosRemarkingEntryGetFirst,
         3, 0},
        {"cpssDxCh3PolicerQosRemarkingEntryGetNext",
         &wrCpssDxCh3PolicerQosRemarkingEntryGetNext,
         3, 0},
        /* DxChXcat and above */
        {"cpssDxChPolicerCountingModeSet",
         &wrCpssDxChPolicerCountingModeSet,
         2, 0},
        {"cpssDxChPolicerCountingModeGet",
         &wrCpssDxChPolicerCountingModeGet,
         1, 0},
        {"cpssDxChPolicerEgressQosRemarkingEntrySet",
         &wrCpssDxChPolicerEgressQosRemarkingEntrySet,
         4, 3},
        {"cpssDxChPolicerEgressQosRemarkingEntryGetFirst",
         &wrCpssDxChPolicerEgressQosRemarkingEntryGet,
         4, 0},
        {"cpssDxChPolicerEgressQosRemarkingEntryGetNext",
         &wrCpssDxChPolicerGetNext,
         4, 0},
        {"cpssDxChPolicerStageMeterModeSet",
         &wrCpssDxChPolicerStageMeterModeSet,
         2, 0},
        {"cpssDxChPolicerStageMeterModeGet",
         &wrCpssDxChPolicerStageMeterModeGet,
         1, 0},
        {"cpssDxChPolicerMeteringOnTrappedPktsEnableSet",
         &wrCpssDxChPolicerMeteringOnTrappedPktsEnableSet,
         2, 0},
        {"cpssDxChPolicerMeteringOnTrappedPktsEnableGet",
         &wrCpssDxChPolicerMeteringOnTrappedPktsEnableGet,
         1, 0},
        {"cpssDxChPolicerPortStormTypeIndexSet",
         &wrCpssDxChPolicerPortStormTypeIndexSet,
         4, 0},
        {"cpssDxChPolicerPortStormTypeIndexGet",
         &wrCpssDxChPolicerPortStormTypeIndexGet,
         3, 0},
        {"cpssDxChPolicerEgressL2RemarkModelSet",
         &wrCpssDxChPolicerEgressL2RemarkModelSet,
         2, 0},
        {"cpssDxChPolicerEgressL2RemarkModelGet",
         &wrCpssDxChPolicerEgressL2RemarkModelGet,
         1, 0},
        {"cpssDxChPolicerEgressQosUpdateEnableSet",
         &wrCpssDxChPolicerEgressQosUpdateEnableSet,
         2, 0},
        {"cpssDxChPolicerEgressQosUpdateEnableGet",
         &wrCpssDxChPolicerEgressQosUpdateEnableGet,
         1, 0},
        {"cpssDxChPolicerVlanCntrSet",
         &wrCpssDxChPolicerVlanCntrSet,
         3, 0},
        {"cpssDxChPolicerVlanCntrGet",
         &wrCpssDxChPolicerVlanCntrGet,
         2, 0},
        {"cpssDxChPolicerVlanCountingPacketCmdTriggerSet",
         &wrCpssDxChPolicerVlanCountingPacketCmdTriggerSet,
         3, 0},
        {"cpssDxChPolicerVlanCountingPacketCmdTriggerGet",
         &wrCpssDxChPolicerVlanCountingPacketCmdTriggerGet,
         2, 0},
        {"cpssDxChPolicerVlanCountingModeSet",
         &wrCpssDxChPolicerVlanCountingModeSet,
         2, 0},
        {"cpssDxChPolicerVlanCountingModeGet",
         &wrCpssDxChPolicerVlanCountingModeGet,
         1, 0},
        {"cpssDxChPolicerPolicyCntrSet",
         &wrCpssDxChPolicerPolicyCntrSet,
         3, 0},
        {"cpssDxChPolicerPolicyCntrGet",
         &wrCpssDxChPolicerPolicyCntrGet,
         2, 0},
        {"cpssDxChPolicerMemorySizeModeSet",
         &wrCpssDxChPolicerMemorySizeModeSet,
         2, 0},
        {"cpssDxChPolicerMemorySizeModeGet",
         &wrCpssDxChPolicerMemorySizeModeGet,
         1, 0},
        {"cpssDxChPolicerMemorySizeModeSet1",
         &wrCpssDxChPolicerMemorySizeModeSet,
         2, 0},
        {"cpssDxChPolicerMemorySizeModeGet1",
         &wrCpssDxChPolicerMemorySizeModeGet,
         1, 0},
        {"cpssDxChPolicerMemorySizeModeSet2",
         &wrCpssDxChPolicerMemorySizeModeSet2,
         4, 0},
        {"cpssDxChPolicerMemorySizeModeGet2",
         &wrCpssDxChPolicerMemorySizeModeGet2,
         1, 0},
        {"cpssDxChPolicerCountingWriteBackCacheFlush",
         &wrCpssDxChPolicerCountingWriteBackCacheFlush,
         1, 0},
        {"cpssDxChPolicerGlobalStageSet",
         &wrCpssDxChPolicerGlobalStageSet,
         1, 0},
        {"cpssDxCh3PolicerManagementCountersSetV1",
         &wrCpssDxCh3PolicerManagementCountersSetV1,
         6, 0},
        {"cpssDxCh3PolicerManagementCountersGetV1",
         &wrCpssDxCh3PolicerManagementCountersGetV1,
         3, 0},
        {"cpssDxChPolicerMeteringCalcMethodSet",
         &wrCpssDxChPolicerMeteringCalcMethodSet,
         4, 0},
        {"cpssDxChPolicerMeteringCalcMethodGet",
         &wrCpssDxChPolicerMeteringCalcMethodGet,
         1, 0},
        {"cpssDxCh3PolicerMeteringEntryV1Set",
         &wrCpssDxCh3PolicerMeteringEntrySetV1,
         3, 19},
        {"cpssDxCh3PolicerMeteringEntryV1GetFirst",
         &wrCpssDxCh3PolicerMeteringEntryGetFirstV1,
         3, 0},
        {"cpssDxCh3PolicerMeteringEntryV1GetNext",
         &wrCpssDxCh3PolicerMeteringEntryGetNextV1,
         3, 0},
        {"cpssDxCh3PolicerMeteringEntryV2Set",
         &wrCpssDxCh3PolicerMeteringEntrySetV2,
         3, 30},
        {"cpssDxCh3PolicerMeteringEntryV2GetFirst",
         &wrCpssDxCh3PolicerMeteringEntryGetFirstV2,
         3, 0},
        {"cpssDxCh3PolicerMeteringEntryV2GetNext",
         &wrCpssDxCh3PolicerMeteringEntryGetNextV2,
         3, 0},
        {"cpssDxCh3PolicerBillingEntryV1Set",
         &wrCpssDxCh3PolicerBillingEntrySetV1,
         4, 8},
        {"cpssDxCh3PolicerBillingEntryV1GetFirst",
         &wrCpssDxCh3PolicerBillingEntryGetFirstV1,
         4, 0},
        {"cpssDxCh3PolicerBillingEntryV1GetNext",
         &wrCpssDxCh3PolicerBillingEntryGetNextV1,
         4, 0},
        {"cpssDxCh3PolicerBillingEntryV2Set",
         &wrCpssDxCh3PolicerBillingEntrySetV2,
         4, 17},
        {"cpssDxCh3PolicerBillingEntryV2GetFirst",
         &wrCpssDxCh3PolicerBillingEntryGetFirstV2,
         4, 0},
        {"cpssDxCh3PolicerBillingEntryV2GetNext",
         &wrCpssDxCh3PolicerBillingEntryGetNextV2,
         4, 0},
        {"cpssDxChPolicerCountingTriggerByPortEnableSet",
         &wrCpssDxChPolicerCountingTriggerByPortEnableSet,
         2, 0},
        {"cpssDxChPolicerCountingTriggerByPortEnableGet",
         &wrCpssDxChPolicerCountingTriggerByPortEnableGet,
         1, 0},
        {"cpssDxChPolicerPortModeAddressSelectSet",
         &wrCpssDxChPolicerPortModeAddressSelectSet,
         2, 0},
        {"cpssDxChPolicerPortModeAddressSelectGet",
         &wrCpssDxChPolicerPortModeAddressSelectGet,
         1, 0},
        {"cpssDxChPolicerEAttributesMeteringModeSet",
         &wrCpssDxChPolicerEAttributesMeteringModeSet,
         3, 0},
        {"cpssDxChPolicerEAttributesMeteringModeGet",
         &wrCpssDxChPolicerEAttributesMeteringModeGet,
         2, 0},
        {"cpssDxChPolicerTriggerEntrySet",
         &wrCpssDxChPolicerTriggerEntrySet,
         3, 16},
        {"cpssDxChPolicerTriggerEntryGetFirst",
         &wrCpssDxChPolicerTriggerEntryGetFirst,
         3, 0},
        {"cpssDxChPolicerTriggerEntryGetNext",
         &wrCpssDxChPolicerTriggerEntryGetNext,
         3, 0},
        {"cpssDxChPolicerFlowIdCountingCfgSet",
         &wrCpssDxChPolicerFlowIdCountingCfgSet,
         6, 0},
        {"cpssDxChPolicerFlowIdCountingCfgGet",
         &wrCpssDxChPolicerFlowIdCountingCfgGet,
         2, 0},
        {"cpssDxChPolicerHierarchicalTableSet",
         &wrCpssDxChPolicerHierarchicalTableSet,
         3, 4},
        {"cpssDxChPolicerHierarchicalTableGetNext",
         &wrCpssDxChPolicerHierarchicalTableGetNext,
         3, 0},
        {"cpssDxChPolicerHierarchicalTableGetFirst",
         &wrCpssDxChPolicerHierarchicalTableGetFirst,
         3, 0},
        {"cpssDxChPolicerSecondStageIndexMaskSet",
         &wrCpssDxChPolicerSecondStageIndexMaskSet,
         2, 0},
        {"cpssDxChPolicerSecondStageIndexMaskGet",
         &wrCpssDxChPolicerSecondStageIndexMaskGet,
         1, 0},

        {"cpssDxChPolicerTrappedPacketsBillingEnableSet",
         &wrCpssDxChPolicerTrappedPacketsBillingEnableSet,
         2, 0},
        {"cpssDxChPolicerTrappedPacketsBillingEnableGet",
         &wrCpssDxChPolicerTrappedPacketsBillingEnableGet,
         1, 0},

        {"cpssDxChPolicerLossMeasurementCntrCaptureEnSet",
         &wrCpssDxChPolicerLossMeasurementCounterCaptureEnableSet,
         2, 0},

        {"cpssDxChPolicerLossMeasurementCntrCaptureEnGet",
         &wrCpssDxChPolicerLossMeasurementCounterCaptureEnableGet,
         1, 0},

        {"cpssDxChPolicerPortStormTypeInFlowModeEnableSet",
         &wrCpssDxChPolicerPortStormTypeInFlowModeEnableSet,
         3, 0},

        {"cpssDxChPolicerPortStormTypeInFlowModeEnableGet",
         &wrCpssDxChPolicerPortStormTypeInFlowModeEnableGet,
         2, 0},

        {"cpssDxChPolicerPacketSizeModeForTimestampTagSet",
         &wrCpssDxChPolicerPacketSizeModeForTimestampTagSet,
         2, 0},

        {"cpssDxChPolicerPacketSizeModeForTimestampTagGet",
         &wrCpssDxChPolicerPacketSizeModeForTimestampTagGet,
         1, 0},
        {"cpssDxChPolicerFlowBasedMeteringToFloodOnlyEnSet",
         &wrCpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet,
         2, 0},
        {"cpssDxChPolicerFlowBasedMeteringToFloodOnlyEnGet",
         &wrCpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet,
         1, 0},
        {"cpssDxChPolicerFlowBasedBillingToFloodOnlyEnSet",
         &wrCpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet,
         2, 0},
        {"cpssDxChPolicerFlowBasedBillingToFloodOnlyEnGet",
         &wrCpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet,
         1, 0},
        {"cpssDxChPolicerMeteringEntryEnvelopeSetFirst",
         &wrCpssDxChPolicerMeteringEntryEnvelopeSetFirst,
         4, 29},
        {"cpssDxChPolicerMeteringEntryEnvelopeSetNext",
         &wrCpssDxChPolicerMeteringEntryEnvelopeSetNext,
         4, 29},
        {"cpssDxChPolicerMeteringEntryEnvelopeEndSet",
         &wrCpssDxChPolicerMeteringEntryEnvelopeEndSet,
         4, 0},
        {"cpssDxChPolicerMeteringEntryEnvelopeGetFirst",
         &wrCpssDxChPolicerMeteringEntryEnvelopeGetFirst,
         4, 0},
        {"cpssDxChPolicerMeteringEntryEnvelopeGetNext",
         &wrCpssDxChPolicerMeteringEntryEnvelopeGetNext,
         4, 0},
        {"cpssDxChPolicerMeterTableFlowBasedIndexConfigSet",
         &wrCpssDxChPolicerMeterTableFlowBasedIndexConfigSet,
         4, 0},
        {"cpssDxChPolicerMeterTableFlowBasedIndexConfigGet",
         &wrCpssDxChPolicerMeterTableFlowBasedIndexConfigGet,
         2, 0},
        {"cpssDxChPolicerQosProfileToPriorityMapSet",
         &wrCpssDxChPolicerQosProfileToPriorityMapSet,
         2, 2},
        {"cpssDxChPolicerQosProfileToPriorityMapGetFirst",
         &wrCpssDxChPolicerQosProfileToPriorityMapGetFirst,
         4, 0},
        {"cpssDxChPolicerQosProfileToPriorityMapGetNext",
         &wrCpssDxChPolicerQosProfileToPriorityMapGetNext,
         4, 0},
        {"cpssDxChPolicerTokenBucketMaxRateSet",
         &wrCpssDxChPolicerTokenBucketMaxRateSet,
         7, 3},
        {"cpssDxChPolicerTokenBucketMaxRateGetFirst",
         &wrCpssDxChPolicerTokenBucketMaxRateGetFirst,
         7, 0},
        {"cpssDxChPolicerTokenBucketMaxRateGetNext",
         &wrCpssDxChPolicerTokenBucketMaxRateGetNext,
         7, 0},
        {"cpssDxChPolicerBucketsCurrentStateGetGetFirst",
         &wrCpssDxChPolicerBucketsCurrentStateGetFirst,
         4, 0},
        {"cpssDxChPolicerBucketsCurrentStateGetGetNext",
         &wrCpssDxChPolicerBucketsCurrentStateGetNext,
         4, 0},
        {"cpssDxChPolicerMeteringAnalyzerIndexSet",
         &wrCpssDxChPolicerMeteringAnalyzerIndexSet,
         5, 0},
        {"cpssDxChPolicerMeteringAnalyzerIndexGet",
         &wrCpssDxChPolicerMeteringAnalyzerIndexGet,
         3, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxCh3Policer function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssDxCh3Policer
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


