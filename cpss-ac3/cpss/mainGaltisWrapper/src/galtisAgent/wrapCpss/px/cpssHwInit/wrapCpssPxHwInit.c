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
* @file wrapCpssPxHwInit.c
*
* @brief Wrapper functions for
* cpss/px/cpssHwInit/cpssPxHwInit.h API's
*
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>

/**
* @internal wrCpssPxHwPpSoftResetTrigger function
* @endinternal
*
* @brief   This routine issues soft reset for a specific device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Soft reset is the process in which selected entities (as configured by
*       using the cpssPxHwPpSoftResetSkipParamSet API) are reset to their
*       default values.
*
*/
static CMD_STATUS wrCpssPxHwPpSoftResetTrigger
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxHwPpSoftResetTrigger(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxHwPpSoftResetSkipParamSet function
* @endinternal
*
* @brief   This routine configures skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxHwPpSoftResetSkipParamSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT skipType;
    GT_BOOL skipEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    skipType = (CPSS_HW_PP_RESET_SKIP_TYPE_ENT)inArgs[1];
    skipEnable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxHwPpSoftResetSkipParamSet(devNum,skipType, skipEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxHwPpSoftResetSkipParamGet function
* @endinternal
*
* @brief   This routine returns configuration of skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssPxHwPpSoftResetSkipParamGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType;
    GT_BOOL                         skipEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    skipType = (CPSS_HW_PP_RESET_SKIP_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxHwPpSoftResetSkipParamGet(devNum,skipType, &skipEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", skipEnable);


    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssPxHwPpSoftResetTrigger",
         &wrCpssPxHwPpSoftResetTrigger,
         1, 0},
    {"cpssPxHwPpSoftResetSkipParamSet",
         &wrCpssPxHwPpSoftResetSkipParamSet,
         3, 0},
    {"cpssPxHwPpSoftResetSkipParamGet",
         &wrCpssPxHwPpSoftResetSkipParamGet,
         2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxHwInit function
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
GT_STATUS cmdLibInitCpssPxHwInit
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


