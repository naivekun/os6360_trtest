/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
*/
/**
********************************************************************************
* @file wrapCpssDxChTmGlueDrop.c
*
* @brief Wrapper functions for TM Drop = cpssDxChTmGlueDrop.c functions
*
* @version   19
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDrop.h>


/**
* @internal wrCpssDxChTmGlueDropQueueProfileIdSet function
* @endinternal
*
* @brief   The function maps every Queue Id to its Profile Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueDropQueueProfileIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    queueId;
    GT_U32    profileId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    queueId    = (GT_U32)inArgs[1];
    profileId  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTmGlueDropQueueProfileIdSet(devNum, queueId, profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChTmGlueDropQueueProfileIdGet function
* @endinternal
*
* @brief   The function gets the Profile Id of a given QueueId.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueDropQueueProfileIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    queueId;
    GT_U32    profileId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    queueId  = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueDropQueueProfileIdGet(devNum, queueId, &profileId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileId);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueDropProfileDropMaskSet function
* @endinternal
*
* @brief   The function configures the Drop Masking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueDropProfileDropMaskSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    tc;
    GT_U32    profileId;
    CPSS_DXCH_TM_GLUE_DROP_MASK_STC dropMaskCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    profileId  = (GT_U32)inArgs[1];
    tc         = (GT_U32)inArgs[2];

    dropMaskCfg.qTailDropUnmask = (GT_U32)inArgs[3];
    dropMaskCfg.qWredDropUnmask = (GT_U32)inArgs[4];
    dropMaskCfg.aTailDropUnmask = (GT_U32)inArgs[5];
    dropMaskCfg.aWredDropUnmask = (GT_U32)inArgs[6];
    dropMaskCfg.bTailDropUnmask = (GT_U32)inArgs[7];
    dropMaskCfg.bWredDropUnmask = (GT_U32)inArgs[8];
    dropMaskCfg.cTailDropUnmask = (GT_U32)inArgs[9];
    dropMaskCfg.cWredDropUnmask = (GT_U32)inArgs[10];
    dropMaskCfg.portTailDropUnmask      = (GT_U32)inArgs[11];
    dropMaskCfg.portWredDropUnmask      = (GT_U32)inArgs[12];
    dropMaskCfg.outOfResourceDropUnmask = (GT_U32)inArgs[13];
    dropMaskCfg.redPacketsDropMode      = (GT_U32)inArgs[14];

    /* call cpss api function */
    result = cpssDxChTmGlueDropProfileDropMaskSet(devNum, profileId, tc, &dropMaskCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueDropProfileDropMaskGet function
* @endinternal
*
* @brief   The function gets the Drop Mask configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueDropProfileDropMaskGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    tc;
    GT_U32    profileId;
    CPSS_DXCH_TM_GLUE_DROP_MASK_STC dropMaskCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    profileId  = (GT_U32)inArgs[1];
    tc         = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTmGlueDropProfileDropMaskGet(devNum, profileId, tc, &dropMaskCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d",
                 dropMaskCfg.qTailDropUnmask, dropMaskCfg.qWredDropUnmask,
                 dropMaskCfg.aTailDropUnmask, dropMaskCfg.aWredDropUnmask,
                 dropMaskCfg.bTailDropUnmask, dropMaskCfg.bWredDropUnmask,
                 dropMaskCfg.cTailDropUnmask, dropMaskCfg.cWredDropUnmask,
                 dropMaskCfg.portTailDropUnmask, dropMaskCfg.portWredDropUnmask,
                 dropMaskCfg.outOfResourceDropUnmask, dropMaskCfg.redPacketsDropMode);
    return CMD_OK;
}


/**
* @internal wrCpssDxChTmGlueDropTcToCosSet function
* @endinternal
*
* @brief   The function configures TM TC to COS mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueDropTcToCosSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    tmTc;
    GT_U32    cos;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    tmTc    = (GT_U32)inArgs[1];
    cos     = (GT_U32)inArgs[2];


    /* call cpss api function */
    result = cpssDxChTmGlueDropTcToCosSet(devNum, tmTc, cos);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGlueDropTcToCosGet function
* @endinternal
*
* @brief   The function retrieves the TM TC to COS mapping COS value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChTmGlueDropTcToCosGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    tmTc;
    GT_U32    cos;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    tmTc    = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGlueDropTcToCosGet(devNum, tmTc, &cos);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cos);

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChTmGlueDropQueueProfileIdSet",
        &wrCpssDxChTmGlueDropQueueProfileIdSet,
        3, 0},

    {"cpssDxChTmGlueDropQueueProfileIdGet",
        &wrCpssDxChTmGlueDropQueueProfileIdGet,
        2, 0},

    {"cpssDxChTmGlueDropProfileDropMaskSet",
        &wrCpssDxChTmGlueDropProfileDropMaskSet,
        15, 0},

    {"cpssDxChTmGlueDropProfileDropMaskGet",
        &wrCpssDxChTmGlueDropProfileDropMaskGet,
        3, 0},

    {"cpssDxChTmGlueDropTcToCosSet",
        &wrCpssDxChTmGlueDropTcToCosSet,
        3, 0},

    {"cpssDxChTmGlueDropTcToCosGet",
        &wrCpssDxChTmGlueDropTcToCosGet,
        2, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChTmGlueDrop function
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
GT_STATUS cmdLibInitCpssDxChTmGlueDrop
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

