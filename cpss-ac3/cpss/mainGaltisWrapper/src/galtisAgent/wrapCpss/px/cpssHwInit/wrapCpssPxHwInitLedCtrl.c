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
* @file wrapCpssPxHwInitLedCtrl.c
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>

/**
* @internal wrCpssPxLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamClassManipulationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32                               classNum;
    CPSS_PX_LED_CLASS_MANIPULATION_STC   classParams;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    classNum                       = (GT_U32)inArgs[1];
    classParams.blinkEnable        = (GT_BOOL)inArgs[2];
    classParams.blinkSelect        = (CPSS_LED_BLINK_SELECT_ENT)inArgs[3];
    classParams.forceEnable        = (GT_BOOL)inArgs[4];
    classParams.forceData          = (GT_U32)inArgs[5];
    classParams.pulseStretchEnable = (GT_BOOL)inArgs[6];
    classParams.disableOnLinkDown  = (GT_BOOL)inArgs[7];

    /* call cpss api function */
    result = cpssPxLedStreamClassManipulationSet(devNum, classNum,
        &classParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxLedStreamClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamClassManipulationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32                               classNum;
    CPSS_PX_LED_CLASS_MANIPULATION_STC   classParams;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    classNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxLedStreamClassManipulationGet(devNum, classNum,
        &classParams);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d%d%d",
        classParams.blinkEnable, classParams.blinkSelect,
        classParams.forceEnable, classParams.forceData,
        classParams.pulseStretchEnable, classParams.disableOnLinkDown);
    return CMD_OK;
}

/**
* @internal wrCpssPxLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..5)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamGroupConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32                      groupNum;
    CPSS_LED_GROUP_CONF_STC     groupParams;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum             = (GT_SW_DEV_NUM)inArgs[0];
    groupNum           = (GT_U32)inArgs[1];
    groupParams.classA = (GT_U32)inArgs[2];
    groupParams.classB = (GT_U32)inArgs[3];
    groupParams.classC = (GT_U32)inArgs[4];
    groupParams.classD = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssPxLedStreamGroupConfigSet(
        devNum, groupNum, &groupParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamGroupConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_U32                      groupNum;
    CPSS_LED_GROUP_CONF_STC     groupParams;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    groupNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxLedStreamGroupConfigGet(
        devNum, groupNum, &groupParams);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d", groupParams.classA,
        groupParams.classB, groupParams.classC, groupParams.classD);

    return CMD_OK;
}

/**
* @internal wrCpssPxLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_LED_CONF_STC   ledConf;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                    = (GT_SW_DEV_NUM)inArgs[0];
    ledConf.ledOrganize       = (CPSS_LED_ORDER_MODE_ENT)inArgs[1];
    ledConf.blink0DutyCycle   = (CPSS_LED_BLINK_DUTY_CYCLE_ENT)inArgs[2];
    ledConf.blink0Duration    = (CPSS_LED_BLINK_DURATION_ENT)inArgs[3];
    ledConf.blink1DutyCycle   = (CPSS_LED_BLINK_DUTY_CYCLE_ENT)inArgs[4];
    ledConf.blink1Duration    = (CPSS_LED_BLINK_DURATION_ENT)inArgs[5];
    ledConf.pulseStretch      = (CPSS_LED_PULSE_STRETCH_ENT)inArgs[6];
    ledConf.ledStart          = (GT_U32)inArgs[7];
    ledConf.ledEnd            = (GT_U32)inArgs[8];
    ledConf.invertEnable      = (GT_BOOL)inArgs[9];
    ledConf.ledClockFrequency = (CPSS_LED_CLOCK_OUT_FREQUENCY_ENT)inArgs[10];

    /* call cpss api function */
    result = cpssPxLedStreamConfigSet(devNum, &ledConf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_LED_CONF_STC   ledConf;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxLedStreamConfigGet(devNum, &ledConf);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d%d%d%d%d%d%d",
        ledConf.ledOrganize,
        ledConf.blink0DutyCycle,
        ledConf.blink0Duration,
        ledConf.blink1DutyCycle,
        ledConf.blink1Duration,
        ledConf.pulseStretch,
        ledConf.ledStart,
        ledConf.ledEnd,
        ledConf.invertEnable,
        ledConf.ledClockFrequency);
    return CMD_OK;
}

/**
* @internal wrCpssPxLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamPortPositionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  position;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    position = (GT_U32)inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxLedStreamPortPositionSet(devNum, portNum, position);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssPxLedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamPortPositionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  position;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxLedStreamPortPositionGet(devNum, portNum, &position);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", position);

    return CMD_OK;

}

/**
* @internal wrCpssPxLedStreamPortClassPolarityInvertEnableSet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      classNum;
    GT_BOOL     invertEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    classNum = (GT_U32)inArgs[2];
    invertEnable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxLedStreamPortClassPolarityInvertEnableSet(
        devNum, portNum, classNum, invertEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxLedStreamPortClassPolarityInvertEnableGet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssPxLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      classNum;
    GT_BOOL     invertEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    classNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxLedStreamPortClassPolarityInvertEnableGet(
        devNum, portNum, classNum, &invertEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", invertEnable);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssPxLedStreamClassManipulationSet",
        &wrCpssPxLedStreamClassManipulationSet,
        8, 0},

    {"cpssPxLedStreamClassManipulationGet",
        &wrCpssPxLedStreamClassManipulationGet,
        2, 0},

    {"cpssPxLedStreamGroupConfigSet",
        &wrCpssPxLedStreamGroupConfigSet,
        6, 0},

    {"cpssPxLedStreamGroupConfigGet",
        &wrCpssPxLedStreamGroupConfigGet,
        2, 0},

    {"cpssPxLedStreamConfigSet",
        &wrCpssPxLedStreamConfigSet,
        11, 0},

    {"cpssPxLedStreamConfigGet",
        &wrCpssPxLedStreamConfigGet,
        1, 0},

    {"cpssPxLedStreamPortPositionSet",
        &wrCpssPxLedStreamPortPositionSet,
        3, 0},

    {"cpssPxLedStreamPortPositionGet",
        &wrCpssPxLedStreamPortPositionGet,
        2, 0},

    {"cpssPxLedStreamPortClassPolarityInvertEnableSet",
        &wrCpssPxLedStreamPortClassPolarityInvertEnableSet,
        4, 0},

    {"cpssPxLedStreamPortClassPolarityInvertEnableGet",
        &wrCpssPxLedStreamPortClassPolarityInvertEnableGet,
        3, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxHwInitLedCtrl function
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
GT_STATUS cmdLibInitCpssPxHwInitLedCtrl
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

