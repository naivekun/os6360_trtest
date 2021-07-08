/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file wrapCpssPxPortEcn.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortEcn.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortEcn.h>

/**
* @internal wrCpssPxPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or protocolStack parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortEcnMarkingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortEcnMarkingEnableSet(devNum, protocolStack, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxPortEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get status of ECN marking.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or protocolStack parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortEcnMarkingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortEcnMarkingEnableGet(devNum, protocolStack, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssPxPortEcnMarkingTailDropProfileEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking per profile according to Tail Drop limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or profileSet
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortEcnMarkingTailDropProfileEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet;
    CPSS_PX_PORT_ECN_ENABLERS_STC               enablers;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT)inArgs[1];
    enablers.tcDpLimit = (GT_BOOL)inArgs[2];
    enablers.portLimit = (GT_BOOL)inArgs[3];
    enablers.tcLimit = (GT_BOOL)inArgs[4];
    enablers.sharedPoolLimit = (GT_BOOL)inArgs[5];

    /* call cpss api function */
    result = cpssPxPortEcnMarkingTailDropProfileEnableSet(devNum, profileSet, &enablers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortEcnMarkingTailDropProfileEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable ECN marking per profile according to Tail Drop limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or profileSet
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortEcnMarkingTailDropProfileEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet;
    CPSS_PX_PORT_ECN_ENABLERS_STC               enablers;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortEcnMarkingTailDropProfileEnableGet(devNum, profileSet, &enablers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", enablers.tcDpLimit, enablers.portLimit, enablers.tcLimit, enablers.sharedPoolLimit);

    return CMD_OK;
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /* commandName                                   funcReference                  funcArgs  funcFields */
    {"cpssPxPortEcnMarkingEnableSet",
        &wrCpssPxPortEcnMarkingEnableSet,
        3, 0},
    {"cpssPxPortEcnMarkingEnableGet",
        &wrCpssPxPortEcnMarkingEnableGet,
        2, 0},
    {"cpssPxPortEcnMarkingTailDropProfileEnableSet",
        &wrCpssPxPortEcnMarkingTailDropProfileEnableSet,
        6, 0},
    {"cpssPxPortEcnMarkingTailDropProfileEnableGet",
        &wrCpssPxPortEcnMarkingTailDropProfileEnableGet,
        2, 0},
};

#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxPortEcn function
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
GT_STATUS cmdLibInitCpssPxPortEcn
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


