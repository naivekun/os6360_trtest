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
* @file wrapCpssTm2Tm.c
*
* @brief Wrapper functions for TM configuration library control interface.
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTm2Tm.h>

/**
* @internal wrCpssTm2TmGlobalConfig function
* @endinternal
*
* @brief   TM2TM Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmGlobalConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    cosSelector;
    CPSS_TM2TM_EXTERNAL_HDR_STC      extHeaders;
    CPSS_TM2TM_CNTRL_PKT_STRUCT_STC  ctrlPacketStr;
    CPSS_TM2TM_DELTA_RANGE_STC       range;
    GT_U32      ii;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cosSelector = (GT_U32)inArgs[1];
    extHeaders.size = (GT_U32)inArgs[2];
    for (ii=0; ii<4 ; ii++) {
        extHeaders.contentArr[ii].l[0] = (GT_32)inArgs[3+(ii*2)];
        extHeaders.contentArr[ii].l[1] = (GT_32)inArgs[4+(ii*2)];
    }
    ctrlPacketStr.portsNum = (GT_U32)inArgs[11];
	ctrlPacketStr.nodesNum = (GT_U32)inArgs[12];
    range.upperRange0 = (GT_U32)inArgs[13];
	range.upperRange1 = (GT_U32)inArgs[14];
	range.upperRange2 = (GT_U32)inArgs[15];

    /* call cpss api function */
    result = cpssTm2TmGlobalConfig(devNum, cosSelector, &extHeaders, &ctrlPacketStr, &range);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTm2TmChannelPortConfig function
* @endinternal
*
* @brief   TM2TM Port Channel Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmChannelPortConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                           devNum;
    GT_U32                          egressElements;
    CPSS_TM_LEVEL_ENT               srcLevel;
    CPSS_TM_LEVEL_ENT               bpLevel;
    GT_U32                          bpOffset;
    GT_U32                          bpXon;
    GT_U32                          bpXoff;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    egressElements = (GT_U32)inArgs[1];
    srcLevel = (CPSS_TM_LEVEL_ENT)inArgs[2];
    bpLevel = (CPSS_TM_LEVEL_ENT)inArgs[3];
	bpOffset = (GT_U32)inArgs[4];
    bpXon = (GT_U32)inArgs[5];
	bpXoff = (GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssTm2TmChannelPortConfig(devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon,bpXoff );

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTm2TmChannelNodeConfig function
* @endinternal
*
* @brief   TM2TM Node Channel Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmChannelNodeConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                           devNum;
    GT_U32                          egressElements;
    CPSS_TM_LEVEL_ENT               srcLevel;
    CPSS_TM_LEVEL_ENT               bpLevel;
    GT_U32                          bpOffset;
    GT_U32                          bpXon;
    GT_U32                          bpXoff;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    egressElements = (GT_U32)inArgs[1];
    srcLevel = (CPSS_TM_LEVEL_ENT)inArgs[2];
    bpLevel = (CPSS_TM_LEVEL_ENT)inArgs[3];
	bpOffset = (GT_U32)inArgs[4];
    bpXon = (GT_U32)inArgs[5];
	bpXoff = (GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssTm2TmChannelNodeConfig(devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon,bpXoff );

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTm2TmDpQueryResponceLevelSet function
* @endinternal
*
* @brief   Drop Query Response Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmDpQueryResponceLevelSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                           devNum;
    GT_U32                          dpType;
    GT_U32                          portDp;
    CPSS_TM_LEVEL_ENT               localLevel;
    CPSS_TM2TM_CHANNEL_ENT          remoteLevel;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dpType = (GT_U32)inArgs[1];
    portDp = (GT_U32)inArgs[2];
    localLevel = (CPSS_TM_LEVEL_ENT)inArgs[3];
    remoteLevel = (CPSS_TM2TM_CHANNEL_ENT)inArgs[4];


    /* call cpss api function */
    result = cpssTm2TmDpQueryResponceLevelSet(devNum, dpType, portDp, localLevel, remoteLevel);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTm2TmLcConfig function
* @endinternal
*
* @brief   TM2TM LC Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmLcConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_U32          quantum;
    GT_U64          qmapHeader;
    GT_U32          ctrlHeader;
    GT_U32          tmPort;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    quantum = (GT_U32)inArgs[1];
    qmapHeader.l[0] = (GT_U32)inArgs[2];
    qmapHeader.l[1] = (GT_U32)inArgs[3];
    ctrlHeader = (GT_U32)inArgs[4];
    tmPort = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssTm2TmLcConfig(devNum, quantum, qmapHeader, ctrlHeader, tmPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTm2TmEgressDropAqmModeSet function
* @endinternal
*
* @brief   TM2TM Egress Drop AQM Mode Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmEgressDropAqmModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                  result;
    GT_U8                      devNum;
    CPSS_TM_LEVEL_ENT          level;
    CPSS_TM_COLOR_NUM_ENT      colorNum;
    CPSS_TM_DP_SOURCE_ENT      dpSource;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    colorNum = (CPSS_TM_COLOR_NUM_ENT)inArgs[2];
    dpSource = (CPSS_TM_DP_SOURCE_ENT)inArgs[3];


    /* call cpss api function */
    result = cpssTm2TmEgressDropAqmModeSet(devNum, level, colorNum, &dpSource);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssTm2TmIngressDropAqmModeSet function
* @endinternal
*
* @brief   TM2TM Ingress Drop AQM Mode Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or color number.
* @retval GT_NOT_INITIALIZED       - on not configured channel at system setup.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmIngressDropAqmModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                  result;
    GT_U8                      devNum;
    CPSS_TM2TM_CHANNEL_ENT     channel;
    CPSS_TM_COLOR_NUM_ENT      colorNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    channel = (CPSS_TM2TM_CHANNEL_ENT)inArgs[1];
    colorNum = (CPSS_TM_COLOR_NUM_ENT)inArgs[2];



    /* call cpss api function */
    result = cpssTm2TmIngressDropAqmModeSet(devNum, channel, colorNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssTm2TmEnable function
* @endinternal
*
* @brief   TM2TM Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTm2TmEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                  result;
    GT_U8                      devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssTm2TmEnable(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssTm2TmGlobalConfig",
        &wrCpssTm2TmGlobalConfig,
        16, 0},

    {"cpssTm2TmChannelPortConfig",
        &wrCpssTm2TmChannelPortConfig,
        7, 0},

    {"cpssTm2TmChannelNodeConfig",
        &wrCpssTm2TmChannelNodeConfig,
        7, 0},

    {"cpssTm2TmDpQueryResponceLevelSet",
        &wrCpssTm2TmDpQueryResponceLevelSet,
        5, 0},

    {"cpssTm2TmLcConfig",
        &wrCpssTm2TmLcConfig,
        6, 0},

    {"cpssTm2TmEgressDropAqmModeSet",
        &wrCpssTm2TmEgressDropAqmModeSet,
        4, 0},

    {"cpssTm2TmIngressDropAqmModeSet",
        &wrCpssTm2TmIngressDropAqmModeSet,
        3, 0},

    {"cpssTm2TmEnable",
        &wrCpssTm2TmEnable,
        1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssTm2Tm function
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
GT_STATUS cmdLibInitCpssTm2Tm
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

