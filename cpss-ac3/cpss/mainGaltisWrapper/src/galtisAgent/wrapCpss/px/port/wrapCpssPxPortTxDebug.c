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
* @file wrapCpssPxPortTxDebug.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortTxDebug.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortTxDebug.h>


/**
* @internal wrCpssPxPortTxDebugQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue on the specified port
*         of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugQueueingEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  tcQueue;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    tcQueue   = (GT_U8) inArgs[2];
    enable    = (GT_BOOL) inArgs[3];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxDebugQueueingEnableSet(devNum, portNum, tcQueue, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugQueueingEnableGet function
* @endinternal
*
* @brief   Get the status of enqueuing to a Traffic Class queue on the specified
*         port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugQueueingEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  tcQueue;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    tcQueue   = (GT_U8) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxDebugQueueingEnableGet(devNum, portNum, tcQueue, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission from a Traffic Class queue on the specified
*         port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugQueueTxEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  tcQueue;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    tcQueue   = (GT_U8) inArgs[2];
    enable    = (GT_BOOL) inArgs[3];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxDebugQueueTxEnableSet(devNum, portNum, tcQueue, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission from a Traffic Class queue on the
*         specified port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugQueueTxEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  tcQueue;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    tcQueue   = (GT_U8) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxDebugQueueTxEnableGet(devNum, portNum, tcQueue, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugResourceHistogramThresholdSet function
* @endinternal
*
* @brief   Sets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugResourceHistogramThresholdSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          cntrNum;
    GT_U32          threshold;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    cntrNum   = (GT_U32) inArgs[1];
    threshold = (GT_U32) inArgs[2];

    /* call cpss api function */
    result = cpssPxPortTxDebugResourceHistogramThresholdSet(devNum, cntrNum, threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugResourceHistogramThresholdGet function
* @endinternal
*
* @brief   Gets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugResourceHistogramThresholdGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          cntrNum;
    GT_U32          threshold;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    cntrNum   = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxDebugResourceHistogramThresholdGet(devNum, cntrNum, &threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", threshold);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugResourceHistogramCounterGet function
* @endinternal
*
* @brief   Gets Histogram Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Histogram Counter is cleared on read.
*
*/
CMD_STATUS wrCpssPxPortTxDebugResourceHistogramCounterGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          cntrNum;
    GT_U32          counter;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    cntrNum   = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxDebugResourceHistogramCounterGet(devNum, cntrNum, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counter);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugGlobalDescLimitSet function
* @endinternal
*
* @brief   Set limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on limit out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of descriptors limit may affect entire system
*       behavior.
*
*/
CMD_STATUS wrCpssPxPortTxDebugGlobalDescLimitSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          limit;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    limit   = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxDebugGlobalDescLimitSet(devNum, limit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugGlobalDescLimitGet function
* @endinternal
*
* @brief   Get limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugGlobalDescLimitGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          limit;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxDebugGlobalDescLimitGet(devNum, &limit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", limit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugGlobalQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugGlobalQueueTxEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    enable  = (GT_BOOL) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxDebugGlobalQueueTxEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxDebugGlobalQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxDebugGlobalQueueTxEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxDebugGlobalQueueTxEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /*
        commandName,
        funcReference,
        funcArgs, funcFields
    */
    {
        "cpssPxPortTxDebugQueueingEnableSet",
        &wrCpssPxPortTxDebugQueueingEnableSet,
        4, 0
    },
    {
        "cpssPxPortTxDebugQueueingEnableGet",
        &wrCpssPxPortTxDebugQueueingEnableGet,
        3, 0
    },
    {
        "cpssPxPortTxDebugQueueTxEnableSet",
        &wrCpssPxPortTxDebugQueueTxEnableSet,
        4, 0
    },
    {
        "cpssPxPortTxDebugQueueTxEnableGet",
        &wrCpssPxPortTxDebugQueueTxEnableGet,
        3, 0
    },
    {
        "cpssPxPortTxDebugResourceHistogramThresholdSet",
        &wrCpssPxPortTxDebugResourceHistogramThresholdSet,
        3, 0
    },
    {
        "cpssPxPortTxDebugResourceHistogramThresholdGet",
        &wrCpssPxPortTxDebugResourceHistogramThresholdGet,
        2, 0
    },
    {
        "cpssPxPortTxDebugResourceHistogramCounterGet",
        &wrCpssPxPortTxDebugResourceHistogramCounterGet,
        2, 0
    },
    {
        "cpssPxPortTxDebugGlobalDescLimitSet",
        &wrCpssPxPortTxDebugGlobalDescLimitSet,
        2, 0
    },
    {
        "cpssPxPortTxDebugGlobalDescLimitGet",
        &wrCpssPxPortTxDebugGlobalDescLimitGet,
        1, 0
    },
    {
        "cpssPxPortTxDebugGlobalQueueTxEnableSet",
        &wrCpssPxPortTxDebugGlobalQueueTxEnableSet,
        2, 0
    },
    {
        "cpssPxPortTxDebugGlobalQueueTxEnableGet",
        &wrCpssPxPortTxDebugGlobalQueueTxEnableGet,
        1, 0
    },
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortTxDebug function
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
GT_STATUS cmdLibInitCpssPxPortTxDebug
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


