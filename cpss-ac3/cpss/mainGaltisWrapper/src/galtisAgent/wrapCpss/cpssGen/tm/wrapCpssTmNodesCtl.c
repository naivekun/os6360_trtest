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
* @file wrapCpssTmNodesCtl.c
*
* @brief Wrapper functions for TM Shaping APIs.
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/generic/tm/cpssTmNodesCtl.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>

/**
* @internal wrCpssTmNodesCtlQueueInstall function
* @endinternal
*
* @brief   Install queue by a given index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Makes a queue capable of getting traffic.
*
*/
static CMD_STATUS wrCpssTmNodesCtlQueueInstall
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      queueInd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queueInd = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmNodesCtlQueueInstall(devNum, queueInd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmNodesCtlQueueUninstall function
* @endinternal
*
* @brief   Uninstall queue by a given index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note - Makes a queue incapable of getting traffic. Queue must be not in use!
*       - User should flush Q before this call.
*
*/
static CMD_STATUS wrCpssTmNodesCtlQueueUninstall
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      queueInd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queueInd = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmNodesCtlQueueUninstall(devNum, queueInd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmNodesCtlQueueInstallStatusGet function
* @endinternal
*
* @brief   REtrieves install status if the queue by a given index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note - Makes a queue incapable of getting traffic. Queue must be not in use!
*       - User should flush Q before this call.
*
*/
static CMD_STATUS wrCpssTmNodesCtlQueueInstallStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      queueInd;
    GT_U32       status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queueInd = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmNodesCtlQueueInstallStatusGet(devNum, queueInd, &status);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", status);

    return CMD_OK;
}


/**
* @internal wrCpssTmNodesCtlNodeDelete function
* @endinternal
*
* @brief   Delete node from scheduling hierarchy.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_STATE             - on not free node's subtree.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmNodesCtlNodeDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    CPSS_TM_LEVEL_ENT    level;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    index = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssTmNodesCtlNodeDelete(devNum,level, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssTmNodesCtlTransPortDelete function
* @endinternal
*
* @brief   Delete port and all its subtree from scheduling hierarchy.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmNodesCtlTransPortDelete
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

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmNodesCtlTransPortDelete(devNum, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmNodesCtlQueueFlush function
* @endinternal
*
* @brief   Flush queue by a given index. All packets dequeued from this queue are dropped.
*         Wait till flush queue has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush queue blocking API.
*
*/
static CMD_STATUS wrCpssTmNodesCtlQueueFlush
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

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmQueueFlush(devNum, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmQueueFlushTriggerActionStart function
* @endinternal
*
* @brief   Activate Flush queue by a given index.
*         All packets dequeued from this queue are dropped.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush queue non blocking API.
*
*/
static CMD_STATUS wrCpssTmQueueFlushTriggerActionStart
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

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmQueueFlushTriggerActionStart(devNum, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmQueueFlushTriggerActionStatusGet function
* @endinternal
*
* @brief   Get whether Flush queue has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmQueueFlushTriggerActionStatusGet
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
    GT_BOOL     actFinished;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmQueueFlushTriggerActionStatusGet(devNum, index, &actFinished);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", actFinished);

    return CMD_OK;
}


/**
* @internal wrCpssTmNodesCtlPortFlush function
* @endinternal
*
* @brief   Flush port by a given index. All packets dequeued from this port (and all its queues)
*         are dropped. Wait till flush port has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush port blocking API.
*
*/
static CMD_STATUS wrCpssTmNodesCtlPortFlush
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

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmPortFlush(devNum, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmPortFlushTriggerActionStart function
* @endinternal
*
* @brief   Flush port by a given index. All packets dequeued from this port (and all its queues)
*         are dropped.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush port non blocking API.
*
*/
static CMD_STATUS wrCpssTmPortFlushTriggerActionStart
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

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmPortFlushTriggerActionStart(devNum, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmPortFlushTriggerActionStatusGet function
* @endinternal
*
* @brief   Get whether Flush port has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmPortFlushTriggerActionStatusGet
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
    GT_BOOL     actFinished;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmPortFlushTriggerActionStatusGet(devNum, index, &actFinished);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", actFinished);

    return CMD_OK;
}


/**
* @internal wrCpssTmNodesCtlReadNextChange function
* @endinternal
*
* @brief   Read next tree index/range change after reshuffling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_NO_RESOURCE           - on empty list.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmNodesCtlReadNextChange
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    CPSS_TM_TREE_CHANGE_STC      change;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssTmNodesCtlReadNextChange(devNum, &change);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", change.index, change.oldIndex, change.newIndex);

    return CMD_OK;
}


/**
* @internal wrCpssTmNodesCtlCleanList function
* @endinternal
*
* @brief   Empty list of reshuffling changes.
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
static CMD_STATUS wrCpssTmNodesCtlCleanList
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssTmNodesCtlCleanList(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
      {"cpssTmNodesCtlQueueInstall",
        &wrCpssTmNodesCtlQueueInstall,
        2, 0},
      {"cpssTmNodesCtlQueueUninstall",
        &wrCpssTmNodesCtlQueueUninstall,
        2, 0},
      {"cpssTmNodesCtlQueueInstallStatusGet",
        &wrCpssTmNodesCtlQueueInstallStatusGet,
        2, 0},
      {"cpssTmNodesCtlNodeDelete",
        &wrCpssTmNodesCtlNodeDelete,
        3, 0},
      {"cpssTmNodesCtlTransPortDelete",
        &wrCpssTmNodesCtlTransPortDelete,
        2, 0},
      {"cpssTmQueueFlush",
        &wrCpssTmNodesCtlQueueFlush,
        2, 0},
      {"cpssTmQueueFlushTriggerActionStart",
        &wrCpssTmQueueFlushTriggerActionStart,
        2, 0},
      {"cpssTmQueueFlushTriggerActionStatusGet",
        &wrCpssTmQueueFlushTriggerActionStatusGet,
        2, 0},
      {"cpssTmPortFlush",
        &wrCpssTmNodesCtlPortFlush,
        2, 0},
      {"cpssTmPortFlushTriggerActionStart",
        &wrCpssTmPortFlushTriggerActionStart,
        2, 0},
      {"cpssTmPortFlushTriggerActionStatusGet",
        &wrCpssTmPortFlushTriggerActionStatusGet,
        2, 0},
      {"cpssTmNodesCtlReadNextChange",
        &wrCpssTmNodesCtlReadNextChange,
        1, 0},
      {"cpssTmNodesCtlCleanList",
        &wrCpssTmNodesCtlCleanList,
        1, 0}
 };

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssTmNodesCtl function
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
GT_STATUS cmdLibInitCpssTmNodesCtl
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

