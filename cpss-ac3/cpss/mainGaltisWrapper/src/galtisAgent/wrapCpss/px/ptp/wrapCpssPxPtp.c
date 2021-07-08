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
* @file wrapCpssPxPtp.c
*
* @brief Wrapper functions for
* PIPE PTP API's
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
#include <cpss/px/ptp/cpssPxPtp.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/**
* @internal wrCpssPxPtpTaiCaptureOverwriteEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Overwrite Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiCaptureOverwriteEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    result = cpssPxPtpTaiCaptureOverwriteEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiCaptureOverwriteEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Overwrite Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiCaptureOverwriteEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiClockCycleSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 seconds;
    GT_U32 nanoSeconds;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    seconds = (GT_U32)inArgs[1];
    nanoSeconds = (GT_U32)inArgs[2];

    result = cpssPxPtpTaiClockCycleSet(devNum, seconds, nanoSeconds);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiClockCycleGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 seconds;
    GT_U32 nanoSeconds;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiClockCycleGet(devNum, &seconds, &nanoSeconds);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", seconds, nanoSeconds);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiClockModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_CLOCK_MODE_ENT clockMode;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    clockMode = (CPSS_PX_PTP_TAI_CLOCK_MODE_ENT)inArgs[1];

    result = cpssPxPtpTaiClockModeSet(devNum, clockMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiClockModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_CLOCK_MODE_ENT clockMode;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiClockModeGet(devNum, &clockMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", clockMode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiExternalPulseWidthSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 extPulseWidth;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    extPulseWidth = (GT_U32)inArgs[1];

    result = cpssPxPtpTaiExternalPulseWidthSet(devNum, extPulseWidth);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiExternalPulseWidthGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 extPulseWidth;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiExternalPulseWidthGet(devNum, &extPulseWidth);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", extPulseWidth);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_32 fracNanoSecond;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    fracNanoSecond = (GT_32)inArgs[1];

    result = cpssPxPtpTaiFractionalNanosecondDriftSet(devNum, fracNanoSecond);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_32 fracNanoSecond;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiFractionalNanosecondDriftGet(devNum, &fracNanoSecond);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", fracNanoSecond);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiFrequencyDriftThesholdsSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Frequency Drift Thresholds.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiFrequencyDriftThesholdsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 interruptThreshold;
    GT_U32 adjustThreshold;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    interruptThreshold = (GT_U32)inArgs[1];
    adjustThreshold = (GT_U32)inArgs[2];

    result = cpssPxPtpTaiFrequencyDriftThesholdsSet(devNum, interruptThreshold, adjustThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiFrequencyDriftThesholdsGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Frequency Drift Thresholds.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiFrequencyDriftThesholdsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 interruptThreshold;
    GT_U32 adjustThreshold;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiFrequencyDriftThesholdsGet(devNum, &interruptThreshold, &adjustThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", interruptThreshold, adjustThreshold);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiGracefulStepSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 gracefulStep;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    gracefulStep = (GT_U32)inArgs[1];

    result = cpssPxPtpTaiGracefulStepSet(devNum, gracefulStep);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiGracefulStepGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 gracefulStep;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiGracefulStepGet(devNum, &gracefulStep);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", gracefulStep);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiIncomingClockCounterEnableSet function
* @endinternal
*
* @brief   Set enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiIncomingClockCounterEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    result = cpssPxPtpTaiIncomingClockCounterEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiIncomingClockCounterEnableGet function
* @endinternal
*
* @brief   Get enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiIncomingClockCounterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiIncomingClockCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 value;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiIncomingClockCounterGet(devNum, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiIncomingTriggerCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 value;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiIncomingTriggerCounterGet(devNum, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiInputClockSelectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT clockSelect;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    clockSelect = (CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT)inArgs[1];

    result = cpssPxPtpTaiInputClockSelectSet(devNum, clockSelect);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiInputClockSelectGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT clockSelect;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiInputClockSelectGet(devNum, &clockSelect);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", clockSelect);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiInputTriggersCountEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    result = cpssPxPtpTaiInputTriggersCountEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiInputTriggersCountEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiInputTriggersCountEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssPxPtpTaiTodSet with
*       CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
static CMD_STATUS wrCpssPxPtpTaiOutputTriggerEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    result = cpssPxPtpTaiOutputTriggerEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiOutputTriggerEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiOutputTriggerEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiPClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiPClockCycleSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 nanoSeconds;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    nanoSeconds = (GT_U32)inArgs[1];

    result = cpssPxPtpTaiPClockCycleSet(devNum, nanoSeconds);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiPClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiPClockCycleGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 nanoSeconds;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiPClockCycleGet(devNum, &nanoSeconds);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", nanoSeconds);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiPtpPClockDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiPtpPClockDriftAdjustEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    result = cpssPxPtpTaiPtpPClockDriftAdjustEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiPtpPClockDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiPtpPClockDriftAdjustEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodCaptureStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 captureIndex;
    GT_BOOL valid;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    captureIndex = (GT_U32)inArgs[1];

    result = cpssPxPtpTaiTodCaptureStatusGet(devNum, captureIndex, &valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", valid);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
static CMD_STATUS wrCpssPxPtpTaiTodSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_TOD_TYPE_ENT todValueType;
    CPSS_PX_PTP_TAI_TOD_COUNT_STC todValue;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    todValueType = (CPSS_PX_PTP_TAI_TOD_TYPE_ENT)inArgs[1];
    todValue.nanoSeconds = (GT_U32)inArgs[2];
    todValue.seconds.l[0] = (GT_U32)inArgs[3];
    todValue.seconds.l[1] = (GT_U32)inArgs[4];
    todValue.fracNanoSeconds = (GT_U32)inArgs[5];

    result = cpssPxPtpTaiTodSet(devNum, todValueType, &todValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_TOD_TYPE_ENT todValueType;
    CPSS_PX_PTP_TAI_TOD_COUNT_STC todValue;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    todValueType = (CPSS_PX_PTP_TAI_TOD_TYPE_ENT)inArgs[1];

    result = cpssPxPtpTaiTodGet(devNum, todValueType, &todValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", todValue.nanoSeconds,
                                              todValue.seconds.l[0], todValue.seconds.l[1],
                                              todValue.fracNanoSeconds);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodStepSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_TOD_STEP_STC todStep;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    todStep.nanoSeconds = (GT_U32)inArgs[1];
    todStep.fracNanoSeconds = (GT_U32)inArgs[2];

    result = cpssPxPtpTaiTodStepSet(devNum, &todStep);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodStepGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_TOD_STEP_STC todStep;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiTodStepGet(devNum, &todStep);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", todStep.nanoSeconds,
        todStep.fracNanoSeconds);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
static CMD_STATUS wrCpssPxPtpTaiTodUpdateCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32 value;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiTodUpdateCounterGet(devNum, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssPxPtpTaiTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodCounterFunctionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT function;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    function = (CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT)inArgs[1];

    result = cpssPxPtpTaiTodCounterFunctionSet(devNum, function);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssPxPtpTaiTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodCounterFunctionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT function;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiTodCounterFunctionGet(devNum, &function);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", function);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function according to the function set
*         by cpssPxPtpTaiTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodCounterFunctionTriggerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiTodCounterFunctionTriggerSet(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiTodCounterFunctionTriggerGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiTodCounterFunctionTriggerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    result = cpssPxPtpTaiTodCounterFunctionTriggerGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTsuControlSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PTP_TSU_CONTROL_STC control;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    control.unitEnable = (GT_BOOL)inArgs[2];

    result = cpssPxPtpTsuControlSet(devNum, portNum, &control);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTsuControlGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PTP_TSU_CONTROL_STC control;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    result = cpssPxPtpTsuControlGet(devNum, portNum, &control);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", control.unitEnable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTsuCountersClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    result = cpssPxPtpTsuCountersClear(devNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTsuPacketCouterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTsuPacketCouterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType;
    GT_U32 value;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    counterType = (CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT)inArgs[2];

    result = cpssPxPtpTsuPacketCouterGet(devNum, portNum, counterType, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

static GT_U32                                   gPortNum;
static CPSS_PX_PTP_TYPE_INDEX                   gPacketType;
static CPSS_PX_PTP_TYPE_KEY_FORMAT_STC          gKeyData;
static CPSS_PX_PTP_TYPE_KEY_FORMAT_STC          gKeyMask;
static GT_BOOL                                  bReadEntry = GT_TRUE;

/**
* @internal wrcpssPxPtpPortTypeKeySet function
* @endinternal
*
* @brief   Set port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrcpssPxPtpPortTypeKeySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    GT_PHYSICAL_PORT_NUM                 portNum;
    CPSS_PX_PTP_TYPE_KEY_STC portKey;
    GT_U32                               i;
    GT_U32                               j;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    /* map input fields to locals */
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inFields[0];

    i = 1;
    for(j = 0; j < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; j++)
    {
        portKey.portUdbPairArr[j].udbAnchorType =
            (CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT)inFields[i++];
        portKey.portUdbPairArr[j].udbByteOffset = (GT_U32)inFields[i++];
    }

    for(j = 0; j < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; j++)
    {
        portKey.portUdbArr[j].udbAnchorType =
            (CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT)inFields[i++];
        portKey.portUdbArr[j].udbByteOffset = (GT_U32)inFields[i++];
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api set function */
    result = cpssPxPtpPortTypeKeySet(devNum, portNum, &portKey);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrcpssPxPtpPortTypeKeyGet function
* @endinternal
*
* @brief   Get port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrcpssPxPtpPortTypeKeyGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_SW_DEV_NUM                           devNum;
    CPSS_PX_PTP_TYPE_KEY_STC                portKey;
    GT_U32                                  i;
    GT_U32                                  j;
    GT_U32                                  actualNumFields;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gPortNum >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, gPortNum);

    /* call cpss api get function */
    result = cpssPxPtpPortTypeKeyGet(devNum, gPortNum, &portKey);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    i = 0;
    inFields[i++] = gPortNum++;
    for(j = 0; j < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; j++)
    {
        inFields[i++] = portKey.portUdbPairArr[j].udbAnchorType;
        inFields[i++] = portKey.portUdbPairArr[j].udbByteOffset;
    }

    for(j = 0; j < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; j++)
    {
        inFields[i++] = portKey.portUdbArr[j].udbAnchorType;
        inFields[i++] = portKey.portUdbArr[j].udbByteOffset;
    }

    actualNumFields = i;

    /* pack and output table fields */
    for (i = 0; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxPtpPortTypeKeyGet */
CMD_STATUS wrcpssPxPtpPortTypeKeyGetFirst
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

    /* map input fields to global */
    gPortNum = 0;

    return wrcpssPxPtpPortTypeKeyGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxPtpPortTypeKeyGet */
CMD_STATUS wrcpssPxPtpPortTypeKeyGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrcpssPxPtpPortTypeKeyGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrcpssPxPtpTypeKeyEntrySet function
* @endinternal
*
* @brief   Set PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrcpssPxPtpTypeKeyEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                              result;
    GT_SW_DEV_NUM                          devNum;
    CPSS_PX_PTP_TYPE_INDEX                 ptpTypeIndex;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC        keyData;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC        keyMask;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC        *keyFormayPtr;
    GT_BYTE_ARRY                           bArr;   /* Binary data */
    GT_U32                                 i;
    GT_U32                                 j;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    ptpTypeIndex = (CPSS_PX_PTP_TYPE_INDEX)inFields[0];

    result = cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex,
        &keyData, &keyMask);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    if(inFields[1] == 0) /* data */
    {
        keyFormayPtr = &keyData;
    }
    else                 /* mask */
    {
        keyFormayPtr = &keyMask;
    }

    i = 2;

    keyFormayPtr->isUdp          =    (GT_BOOL)inFields[i++];
    keyFormayPtr->srcPortProfile =    (GT_U32)inFields[i++];

    for (j = 0; j < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; j++)
    {
        galtisBArray(&bArr, (GT_U8*)inFields[i++]);
        cmdOsMemCpy(&keyFormayPtr->udbPairsArr[j].udb, bArr.data, bArr.length);
    }

    galtisBArray(&bArr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(&keyFormayPtr->udbArr, bArr.data, bArr.length);

    galtisBArrayFree(&bArr);

    /* call cpss api set function */
    result = cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex,
        &keyData, &keyMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrcpssPxPtpTypeKeyEntryGet function
* @endinternal
*
* @brief   Get PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrcpssPxPtpTypeKeyEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_SW_DEV_NUM                           devNum;
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC        *keyPtr;
    GT_U32                                  dataType;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];

    if (gPacketType >= CPSS_PX_PTP_TYPE_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    if (bReadEntry == GT_TRUE)
    {
        /* call cpss api get function */
        result = cpssPxPtpTypeKeyEntryGet(devNum, gPacketType,
            &gKeyData, &gKeyMask);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }
        /* Parse key data */
        keyPtr = &gKeyData;
        dataType = 0;
    }
    else
    {
        /* Parse key mask */
        keyPtr = &gKeyMask;
        dataType = 1;
    }

    /* PTP type and dataType(mask/pattern) before fields */
    fieldOutput("%d%d", gPacketType, dataType);
    fieldOutputSetAppendMode();

    fieldOutput("%d%d%s%s%s",
        keyPtr->isUdp,
        keyPtr->srcPortProfile,
        galtisByteArrayToString(keyPtr->udbPairsArr[0].udb, 2),
        galtisByteArrayToString(keyPtr->udbPairsArr[1].udb, 2),
        galtisByteArrayToString(keyPtr->udbArr, 6));
    fieldOutputSetAppendMode();

    /* Inverse read action */
    bReadEntry = !bReadEntry;
    if(bReadEntry == GT_TRUE)
    {
        /* Increment PTP type before next read */
        gPacketType++;
    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxPtpTypeKeyEntryGet */
CMD_STATUS wrcpssPxPtpTypeKeyEntryGetFirst
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

    gPacketType = 0;

    return wrcpssPxPtpTypeKeyEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxPtpPortTypeKeyGet */
CMD_STATUS wrcpssPxPtpTypeKeyEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrcpssPxPtpTypeKeyEntryGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrcpssPxPtpTypeKeyEntryEnableSet function
* @endinternal
*
* @brief   Enable/disable PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssPxPtpTypeKeyEntryEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    CPSS_PX_PTP_TYPE_INDEX ptpTypeIndex;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    ptpTypeIndex = (CPSS_PX_PTP_TYPE_INDEX)inFields[0];
    enable = (GT_BOOL)inFields[1];

    result = cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrcpssPxPtpTypeKeyEntryEnableGet function
* @endinternal
*
* @brief   Get enabled state of PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssPxPtpTypeKeyEntryEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    OUT GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api set function */
    result = cpssPxPtpTypeKeyEntryEnableGet(devNum, gPacketType, &enable);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* Tab-number, PTP type before fields */
    fieldOutput("%d%d", gPacketType, enable);

    /* Increment PTP type before next read */
    gPacketType++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxPtpTypeKeyEntryEnableGet */
static CMD_STATUS wrcpssPxPtpTypeKeyEntryEnableGetFirst
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

    gPacketType = 0;
    return wrcpssPxPtpTypeKeyEntryEnableGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxPtpTypeKeyEntryEnableGet */
static CMD_STATUS wrcpssPxPtpTypeKeyEntryEnableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrcpssPxPtpTypeKeyEntryEnableGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrcpssPxPtpTaiPClockOutputEnableSet function
* @endinternal
*
* @brief   Set PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssPxPtpTaiPClockOutputEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    result = cpssPxPtpTaiPClockOutputEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrcpssPxPtpTaiPClockOutputEnableGet function
* @endinternal
*
* @brief   Get PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssPxPtpTaiPClockOutputEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    OUT GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api set function */
    result = cpssPxPtpTaiPClockOutputEnableGet(devNum, &enable);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrcpssPxPtpTaiPhaseUpdateEnableSet function
* @endinternal
*
* @brief   Set configuration of the PPS rising edge phase enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssPxPtpTaiPhaseUpdateEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM devNum;
    GT_BOOL enable;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    result = cpssPxPtpTaiPhaseUpdateEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrcpssPxPtpTaiPhaseUpdateEnableGet function
* @endinternal
*
* @brief   Get configuration of the PPS rising edge phase enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssPxPtpTaiPhaseUpdateEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    OUT GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api set function */
    result = cpssPxPtpTaiPhaseUpdateEnableGet(devNum, &enable);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set PTP PulseIn Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiPulseInModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_SW_DEV_NUM                       devNum;
    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT   pulseMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    pulseMode = (CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPtpTaiPulseInModeSet(devNum, pulseMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get PTP PulseIn Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPtpTaiPulseInModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_SW_DEV_NUM                       devNum;
    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT   pulseMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssPxPtpTaiPulseInModeGet(devNum, &pulseMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pulseMode);

    return CMD_OK;
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    {"cpssPxPtpTaiCaptureOverwriteEnableSet",
    &wrCpssPxPtpTaiCaptureOverwriteEnableSet,
    2, 0},
    {"cpssPxPtpTaiCaptureOverwriteEnableGet",
    &wrCpssPxPtpTaiCaptureOverwriteEnableGet,
    1, 0},
    {"cpssPxPtpTaiClockCycleSet",
    &wrCpssPxPtpTaiClockCycleSet,
    3, 0},
    {"cpssPxPtpTaiClockCycleGet",
    &wrCpssPxPtpTaiClockCycleGet,
    1, 0},
    {"cpssPxPtpTaiClockModeSet",
    &wrCpssPxPtpTaiClockModeSet,
    2, 0},
    {"cpssPxPtpTaiClockModeGet",
    &wrCpssPxPtpTaiClockModeGet,
    1, 0},
    {"cpssPxPtpTaiExternalPulseWidthSet",
    &wrCpssPxPtpTaiExternalPulseWidthSet,
    2, 0},
    {"cpssPxPtpTaiExternalPulseWidthGet",
    &wrCpssPxPtpTaiExternalPulseWidthGet,
    1, 0},
    {"cpssPxPtpTaiFractionalNanosecondDriftSet",
    &wrCpssPxPtpTaiFractionalNanosecondDriftSet,
    2, 0},
    {"cpssPxPtpTaiFractionalNanosecondDriftGet",
    &wrCpssPxPtpTaiFractionalNanosecondDriftGet,
    1, 0},
    {"cpssPxPtpTaiFrequencyDriftThesholdsSet",
    &wrCpssPxPtpTaiFrequencyDriftThesholdsSet,
    3, 0},
    {"cpssPxPtpTaiFrequencyDriftThesholdsGet",
    &wrCpssPxPtpTaiFrequencyDriftThesholdsGet,
    1, 0},
    {"cpssPxPtpTaiGracefulStepSet",
    &wrCpssPxPtpTaiGracefulStepSet,
    2, 0},
    {"cpssPxPtpTaiGracefulStepGet",
    &wrCpssPxPtpTaiGracefulStepGet,
    1, 0},
    {"cpssPxPtpTaiIncomingClockCounterEnableSet",
    &wrCpssPxPtpTaiIncomingClockCounterEnableSet,
    2, 0},
    {"cpssPxPtpTaiIncomingClockCounterEnableGet",
    &wrCpssPxPtpTaiIncomingClockCounterEnableGet,
    1, 0},
    {"cpssPxPtpTaiIncomingClockCounterGet",
    &wrCpssPxPtpTaiIncomingClockCounterGet,
    1, 0},
    {"cpssPxPtpTaiIncomingTriggerCounterGet",
    &wrCpssPxPtpTaiIncomingTriggerCounterGet,
    1, 0},
    {"cpssPxPtpTaiInputClockSelectSet",
    &wrCpssPxPtpTaiInputClockSelectSet,
    2, 0},
    {"cpssPxPtpTaiInputClockSelectGet",
    &wrCpssPxPtpTaiInputClockSelectGet,
    1, 0},
    {"cpssPxPtpTaiInputTriggersCountEnableSet",
    &wrCpssPxPtpTaiInputTriggersCountEnableSet,
    2, 0},
    {"cpssPxPtpTaiInputTriggersCountEnableGet",
    &wrCpssPxPtpTaiInputTriggersCountEnableGet,
    1, 0},
    {"cpssPxPtpTaiOutputTriggerEnableSet",
    &wrCpssPxPtpTaiOutputTriggerEnableSet,
    2, 0},
    {"cpssPxPtpTaiOutputTriggerEnableGet",
    &wrCpssPxPtpTaiOutputTriggerEnableGet,
    1, 0},
    {"cpssPxPtpTaiPClockCycleSet",
    &wrCpssPxPtpTaiPClockCycleSet,
    2, 0},
    {"cpssPxPtpTaiPClockCycleGet",
    &wrCpssPxPtpTaiPClockCycleGet,
    1, 0},
    {"cpssPxPtpTaiPtpPClockDriftAdjustEnableSet",
    &wrCpssPxPtpTaiPtpPClockDriftAdjustEnableSet,
    2, 0},
    {"cpssPxPtpTaiPtpPClockDriftAdjustEnableGet",
    &wrCpssPxPtpTaiPtpPClockDriftAdjustEnableGet,
    1, 0},
    {"cpssPxPtpTaiTodCaptureStatusGet",
    &wrCpssPxPtpTaiTodCaptureStatusGet,
    2, 0},
    {"cpssPxPtpTaiTodSet",
    &wrCpssPxPtpTaiTodSet,
    6, 0},
    {"cpssPxPtpTaiTodGet",
    &wrCpssPxPtpTaiTodGet,
    2, 0},
    {"cpssPxPtpTaiTodStepSet",
    &wrCpssPxPtpTaiTodStepSet,
    3, 0},
    {"cpssPxPtpTaiTodStepGet",
    &wrCpssPxPtpTaiTodStepGet,
    1, 0},
    {"cpssPxPtpTaiTodUpdateCounterGet",
    &wrCpssPxPtpTaiTodUpdateCounterGet,
    1, 0},
    {"cpssPxPtpTaiTodCounterFunctionSet",
    &wrCpssPxPtpTaiTodCounterFunctionSet,
    2, 0},
    {"cpssPxPtpTaiTodCounterFunctionGet",
    &wrCpssPxPtpTaiTodCounterFunctionGet,
    1, 0},
    {"cpssPxPtpTaiTodCounterFunctionTriggerSet",
    &wrCpssPxPtpTaiTodCounterFunctionTriggerSet,
    1, 0},
    {"cpssPxPtpTaiTodCounterFunctionTriggerGet",
    &wrCpssPxPtpTaiTodCounterFunctionTriggerGet,
    1, 0},
    {"cpssPxPtpTsuControlSet",
    &wrCpssPxPtpTsuControlSet,
    3, 0},
    {"cpssPxPtpTsuControlGet",
    &wrCpssPxPtpTsuControlGet,
    2, 0},
    {"cpssPxPtpTsuCountersClear",
    &wrCpssPxPtpTsuCountersClear,
    2, 0},
    {"cpssPxPtpTsuPacketCouterGet",
    &wrCpssPxPtpTsuPacketCouterGet,
    3, 0},
    {"cpssPxPtpPortTypeKeySet",
    &wrcpssPxPtpPortTypeKeySet,
    1, 17},
    {"cpssPxPtpPortTypeKeyGetFirst",
    &wrcpssPxPtpPortTypeKeyGetFirst,
    1, 0},
    {"cpssPxPtpPortTypeKeyGetNext",
    &wrcpssPxPtpPortTypeKeyGetNext,
    1, 0},
    {"cpssPxPtpTypeKeyEntrySet",
    &wrcpssPxPtpTypeKeyEntrySet,
    1, 12},
    {"cpssPxPtpTypeKeyEntryGetFirst",
    &wrcpssPxPtpTypeKeyEntryGetFirst,
    1, 0},
    {"cpssPxPtpTypeKeyEntryGetNext",
    &wrcpssPxPtpTypeKeyEntryGetNext,
    1, 0},
    {"cpssPxPtpTypeKeyEntryEnableSet",
    &wrcpssPxPtpTypeKeyEntryEnableSet,
    1, 2},
    {"cpssPxPtpTypeKeyEntryEnableGetFirst",
    &wrcpssPxPtpTypeKeyEntryEnableGetFirst,
    1, 0},
    {"cpssPxPtpTypeKeyEntryEnableGetNext",
    &wrcpssPxPtpTypeKeyEntryEnableGetNext,
    1, 0},
    {"cpssPxPtpTaiPClockOutputEnableSet",
    &wrcpssPxPtpTaiPClockOutputEnableSet,
    2, 0},
    {"cpssPxPtpTaiPClockOutputEnableGet",
    &wrcpssPxPtpTaiPClockOutputEnableGet,
    1, 0},
    {"cpssPxPtpTaiPhaseUpdateEnableSet",
    &wrcpssPxPtpTaiPhaseUpdateEnableSet,
    2, 0},
    {"cpssPxPtpTaiPhaseUpdateEnableGet",
    &wrcpssPxPtpTaiPhaseUpdateEnableGet,
    1, 0},
    {"cpssPxPtpTaiPulseInModeSet",
    &wrCpssPxPtpTaiPulseInModeSet,
    2, 0},
    {"cpssPxPtpTaiPulseInModeGet",
    &wrCpssPxPtpTaiPulseInModeGet,
    1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxPtp function
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
GT_STATUS cmdLibInitCpssPxPtp
(
    GT_VOID
)
{
    return  cmdInitLibrary(dbCommands, numCommands);
}

