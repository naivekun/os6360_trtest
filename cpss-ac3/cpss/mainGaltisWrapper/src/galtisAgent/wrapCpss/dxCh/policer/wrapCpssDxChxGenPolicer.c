/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapPolicerCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for Policer cpss.dxCh functions.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes */
#include <cpss/generic/policer/cpssGenPolicerTypes.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>

/**
* @internal wrCpssDxChPolicerInit function
* @endinternal
*
* @brief   Init Traffic Conditioner facility on specified device.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on illegal devNum
*/
static CMD_STATUS wrCpssDxChPolicerInit

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPolicerInit(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}



/**
* @internal wrCpssDxChPolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Configure Policing mode for non tunnel terminated packets
*         The Policy engine provides the following modes to define packet size:
*         - Layer 1 metering. Packet size includes the
*         entire packet + IPG + preamble.
*         - Layer 2 metering. Packet size includes the entire packet,
*         including Layer 2 header and CRC.
*         - Layer 3 metering. Packet size includes Layer 3 information only,
*         excluding Layer 2 header and CRC.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
*/
static CMD_STATUS wrCpssDxChPolicerPacketSizeModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                devNum;
    CPSS_POLICER_PACKET_SIZE_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_POLICER_PACKET_SIZE_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerPacketSizeModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerDropRedModeSet function
* @endinternal
*
* @brief   Set the type of the Policer drop action for red packets
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or dropRedMode
*/
static CMD_STATUS wrCpssDxChPolicerDropRedModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                      devNum;
    CPSS_DROP_MODE_TYPE_ENT    dropRedMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropRedMode = (CPSS_DROP_MODE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerDropRedModeSet(devNum, dropRedMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Set the policing counting mode for tunnel terminated packets
*         The Policy engine provides the following modes to define packet size:
*         - Layer 1 metering. Packet size includes the
*         entire packet + IPG + preamble.
*         - Layer 2 metering. Packet size includes the entire packet,
*         including Layer 2 header and CRC.
*         - Passenger packet metering.
*         For IP-Over-x packets, the counting includes the passenger packet's
*         BC, excluding the tunnel header and the packets CRC
*         For, Ethernet-Over-x packets, the counting includes the passenger
*         packet's BC, excluding the tunnel header and if packet includes Two
*         CRC patterns (one for the inner packet and one for the outer packets)
*         also excluding the outer CRC
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
*/
static CMD_STATUS wrCpssDxChPolicerPacketSizeModeForTunnelTermSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                devNum;
    CPSS_POLICER_PACKET_SIZE_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_POLICER_PACKET_SIZE_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerPacketSizeModeForTunnelTermSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerTokenBucketModeSet function
* @endinternal
*
* @brief   Set the policer token bucket mode and policer MRU.
*         To implement the token bucket algorithm, each of the 256 policers
*         incorporates a Bucket Size Counter (BucketSizeCnt).
*         This counter is incremented with tokens, according to the configured
*         policer rate (CIR) up to a maximal value of the configured
*         Policer Burst Size (CBS)
*         The Byte Count of each conforming packet is decremented from the counter.
*         When a new packet arrives, according to this configuration, the packet
*         conformance is checked according to one of the following modes:
*         - Strict Rate Limiter - If BucketSizeCnt > Packet's Byte Count the packet
*         is conforming else, it is out of profile.
*         - Loose Rate Limiter - If BucketSizeCnt > MRU the packet is conforming
*         else, it is out of profile.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChPolicerTokenBucketModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;

    GT_U8                       devNum;
    CPSS_POLICER_TB_MODE_ENT    mode;
    CPSS_POLICER_MRU_ENT        mru;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_POLICER_TB_MODE_ENT)inArgs[1];
    mru = (CPSS_POLICER_MRU_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPolicerTokenBucketModeSet(devNum, mode, mru);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerEntrySet function
* @endinternal
*
* @brief   Set Policer Entry configuration
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or entry parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChPolicerEntrySet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             entryIndex;
    CPSS_DXCH_POLICER_ENTRY_STC        entry;
    CPSS_DXCH_POLICER_TB_PARAMS_STC    tbParams;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];


    entry.policerEnable = (GT_BOOL)inFields[0];
    entry.meterColorMode = (CPSS_POLICER_COLOR_MODE_ENT)inFields[1];
    entry.tbParams.cir = (GT_U32)inFields[2];
    entry.tbParams.cbs = (GT_U32)inFields[3];
    entry.counterEnable = (GT_BOOL)inFields[4];
    entry.counterSetIndex = (GT_U32)inFields[5];
    entry.cmd = (CPSS_DXCH_POLICER_CMD_ENT)inFields[6];
    entry.qosProfile = (GT_U32)inFields[7];
    entry.modifyDscp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[8];
    entry.modifyUp = (CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT)inFields[9];

    cmdOsMemSet((GT_U32*)&tbParams, 0, sizeof(tbParams));

    /* call cpss api function */
    result = cpssDxChPolicerEntrySet(devNum, entryIndex, &entry, &tbParams);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", tbParams.cir, tbParams.cbs);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerEntryGet function
* @endinternal
*
* @brief   Get Policer Entry parameters
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChPolicerEntryGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             entryIndex;
    CPSS_DXCH_POLICER_ENTRY_STC        entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];


    result = cpssDxChPolicerEntryGet(devNum, entryIndex, &entry);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = entry.policerEnable;
    inFields[1] = entry.meterColorMode;
    inFields[2] = entry.tbParams.cir;
    inFields[3] = entry.tbParams.cbs;
    inFields[4] = entry.counterEnable;
    inFields[5] = entry.counterSetIndex;
    inFields[6] = entry.cmd;
    inFields[7] = entry.qosProfile;
    inFields[8] = entry.modifyDscp;
    inFields[9] = entry.modifyUp;


    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                        inFields[3], inFields[4], inFields[5],
                                        inFields[6], inFields[7], inFields[8],
                                                                  inFields[9]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerEntryInvalidate function
* @endinternal
*
* @brief   Invalidate Policer Entry
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex
*/
static CMD_STATUS wrCpssDxChPolicerEntryInvalidate

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;

    GT_U8        devNum;
    GT_U32       entryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPolicerEntryInvalidate(devNum, entryIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculate Token Bucket parameters in the Application format without
*         HW update.
*         The token bucket parameters are returned as output values. This is due to
*         the hardware rate resolution, the exact rate or burst size requested may
*         not be honored. The returned value gives the user the actual parameters
*         configured in the hardware.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChPolicerEntryMeterParamsCalculate

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                             result;

    GT_U8                                 devNum;
    CPSS_DXCH_POLICER_TB_PARAMS_STC       tbInParams;
    CPSS_DXCH_POLICER_TB_PARAMS_STC       tbOutParams;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tbInParams.cir = (GT_U32)inArgs[1];
    tbInParams.cbs = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPolicerEntryMeterParamsCalculate(devNum, &tbInParams,
                                                              &tbOutParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", tbOutParams.cir,
                                          tbOutParams.cbs);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerCountersGet function
* @endinternal
*
* @brief   Get Policer Counters
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChPolicerCountersGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             counterSetIndex;
    CPSS_DXCH_POLICER_COUNTERS_STC     counters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counterSetIndex = (GT_U32)inArgs[1];


    result = cpssDxChPolicerCountersGet(devNum, counterSetIndex, &counters);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = counters.outOfProfileBytesCnt;
    inFields[1] = counters.inProfileBytesCnt;


    /* pack and output table fields */
    fieldOutput("%d%d", inFields[0], inFields[1]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPolicerCountersSet function
* @endinternal
*
* @brief   Set Policer Counters.
*         To reset counters use zero values.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChPolicerCountersSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_U32                             counterSetIndex;
    CPSS_DXCH_POLICER_COUNTERS_STC     counters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counterSetIndex = (GT_U32)inArgs[1];


    counters.outOfProfileBytesCnt = (GT_U32)inFields[0];
    counters.inProfileBytesCnt = (GT_U32)inFields[1];

    /* call cpss api function */
    result = cpssDxChPolicerCountersSet(devNum, counterSetIndex, &counters);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}





/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPolicerInit",
        &wrCpssDxChPolicerInit,
        1, 0},

    {"cpssDxChPolicerPacketSizeModeSet",
        &wrCpssDxChPolicerPacketSizeModeSet,
        2, 0},

    {"cpssDxChPolicerDropRedModeSet",
        &wrCpssDxChPolicerDropRedModeSet,
        2, 0},

    {"cpssDxChPolicerPacketSizeModeForTunnelTermSet",
        &wrCpssDxChPolicerPacketSizeModeForTunnelTermSet,
        2, 0},

    {"cpssDxChPolicerTokenBucketModeSet",
        &wrCpssDxChPolicerTokenBucketModeSet,
        3, 0},

    {"cpssDxChPolicerEntrySet",
        &wrCpssDxChPolicerEntrySet,
        2, 10},

    {"cpssDxChPolicerEntryGetFirst",
        &wrCpssDxChPolicerEntryGet,
        2, 0},

    {"cpssDxChPolicerEntryGetNext",
        &wrCpssDxChPolicerEntryGet,
        2, 0},

    {"cpssDxChPolicerEntryDelete",
        &wrCpssDxChPolicerEntryInvalidate,
        2, 0},

    {"cpssDxChPolicerEntryMeterParamsCalculate",
        &wrCpssDxChPolicerEntryMeterParamsCalculate,
        3, 0},

    {"cpssDxChPolicerCountersGetFirst",
        &wrCpssDxChPolicerCountersGet,
        2, 0},

    {"cpssDxChPolicerCountersGetNext",
        &wrCpssDxChPolicerCountersGet,
        2, 0},

    {"cpssDxChPolicerCountersSet",
        &wrCpssDxChPolicerCountersSet,
        2, 2}


};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPolicer function
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
GT_STATUS cmdLibInitCpssDxChPolicer
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



