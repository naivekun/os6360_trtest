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
* @file wrapCpssPxPortTxScheduler.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortTxScheduler.h API's
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
#include <cpss/px/port/cpssPxPortTxScheduler.h>


/**
* @internal wrCpssPxPortTxSchedulerProfileIdSet function
* @endinternal
*
* @brief   Bind a port to scheduler profile set.
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
CMD_STATUS wrCpssPxPortTxSchedulerProfileIdSet
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
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM) inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    profileSet  = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxSchedulerProfileIdSet(devNum, portNum, profileSet);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerProfileIdGet function
* @endinternal
*
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerProfileIdGet
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
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxSchedulerProfileIdGet(devNum, portNum, &profileSet);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileSet);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerWrrMtuSet function
* @endinternal
*
* @brief   Set global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerWrrMtuSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];
    wrrMtu = (CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerWrrMtuSet(devNum, wrrMtu);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerWrrMtuGet function
* @endinternal
*
* @brief   Get global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssPxPortTxSchedulerWrrMtuGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerWrrMtuGet(devNum, &wrrMtu);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", wrrMtu);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerWrrProfileSet function
* @endinternal
*
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If weight will be less then port's MTU (maximum transmit unit) there is
*       possibility for empty WRR loops for given TC queue, but to not tight
*       user and for future ASIC's where this problem will be solved check of
*       wrrWeight 256 > MTU not implemented.
*
*/
CMD_STATUS wrCpssPxPortTxSchedulerWrrProfileSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tcQueue;
    GT_U32          wrrWeight;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM) inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];
    tcQueue    = (GT_U32) inArgs[2];
    wrrWeight  = (GT_U32) inArgs[3];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerWrrProfileSet(devNum, profileSet, tcQueue, wrrWeight);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerWrrProfileGet function
* @endinternal
*
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerWrrProfileGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tcQueue;
    GT_U32          wrrWeight;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM) inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];
    tcQueue    = (GT_U32) inArgs[2];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerWrrProfileGet(devNum, profileSet, tcQueue, &wrrWeight);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", wrrWeight);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerArbitrationGroupSet function
* @endinternal
*
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device or arbGroup
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerArbitrationGroupSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tcQueue;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;
    CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     arbGroup;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM) inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];
    tcQueue    = (GT_U32) inArgs[2];
    arbGroup   = (CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT) inArgs[3];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerArbitrationGroupSet(devNum, profileSet, tcQueue,
                                                      arbGroup);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerArbitrationGroupGet function
* @endinternal
*
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerArbitrationGroupGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_U32          tcQueue;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;
    CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     arbGroup;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM) inArgs[0];
    profileSet = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];
    tcQueue    = (GT_U32) inArgs[2];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerArbitrationGroupGet(devNum, profileSet, tcQueue,
                                                      &arbGroup);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", arbGroup);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerProfileByteCountChangeEnableSet function
* @endinternal
*
* @brief   Enables/Disables profile Byte Count Change of the packet length by per
*         port constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile, bcMode, bcOp
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerProfileByteCountChangeEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile;
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode;
    CPSS_ADJUST_OPERATION_ENT                       bcOp;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum   = (GT_SW_DEV_NUM) inArgs[0];
    profile  = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];
    bcMode   = (CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT) inArgs[2];
    bcOp     = (CPSS_ADJUST_OPERATION_ENT) inArgs[3];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(devNum, profile,
                                                                  bcMode, bcOp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerProfileByteCountChangeEnableGet function
* @endinternal
*
* @brief   Get profile Byte Count Change mode.
*         The byte count change mode changes the length of a packet by per port
*         constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerProfileByteCountChangeEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile;
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode;
    CPSS_ADJUST_OPERATION_ENT                       bcOp;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum   = (GT_SW_DEV_NUM) inArgs[0];
    profile  = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(devNum, profile,
                                                                  &bcMode, &bcOp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", bcMode, bcOp);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerShaperByteCountChangeValueSet function
* @endinternal
*
* @brief   Sets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerShaperByteCountChangeValueSet
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
    GT_U32                  bcValue;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    bcValue   = (GT_U32) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(devNum, portNum, bcValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerShaperByteCountChangeValueGet function
* @endinternal
*
* @brief   Gets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerShaperByteCountChangeValueGet
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
    GT_U32                  bcValue;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM) inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(devNum, portNum,
                                                                &bcValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", bcValue);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerProfileCountModeSet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortTxSchedulerProfileCountModeSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];
    wrrMode = (CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT) inArgs[2];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerProfileCountModeSet(devNum, profile, wrrMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortTxSchedulerProfileCountModeGet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
CMD_STATUS wrCpssPxPortTxSchedulerProfileCountModeGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    profile = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortTxSchedulerProfileCountModeGet(devNum, profile, &wrrMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", wrrMode);

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
        "cpssPxPortTxSchedulerProfileIdSet",
        &wrCpssPxPortTxSchedulerProfileIdSet,
        3, 0
    },
    {
        "cpssPxPortTxSchedulerProfileIdGet",
        &wrCpssPxPortTxSchedulerProfileIdGet,
        2, 0
    },
    {
        "cpssPxPortTxSchedulerWrrMtuSet",
        &wrCpssPxPortTxSchedulerWrrMtuSet,
        2, 0
    },
    {
        "cpssPxPortTxSchedulerWrrMtuGet",
        &wrCpssPxPortTxSchedulerWrrMtuGet,
        1, 0
    },
    {
        "cpssPxPortTxSchedulerWrrProfileSet",
        &wrCpssPxPortTxSchedulerWrrProfileSet,
        4, 0
    },
    {
        "cpssPxPortTxSchedulerWrrProfileGet",
        &wrCpssPxPortTxSchedulerWrrProfileGet,
        3, 0
    },
    {
        "cpssPxPortTxSchedulerArbitrationGroupSet",
        &wrCpssPxPortTxSchedulerArbitrationGroupSet,
        4, 0
    },
    {
        "cpssPxPortTxSchedulerArbitrationGroupGet",
        &wrCpssPxPortTxSchedulerArbitrationGroupGet,
        3, 0
    },
    {
        "cpssPxPortTxSchedulerProfileByteCountChangeEnSet",
        &wrCpssPxPortTxSchedulerProfileByteCountChangeEnableSet,
        4, 0
    },
    {
        "cpssPxPortTxSchedulerProfileByteCountChangeEnGet",
        &wrCpssPxPortTxSchedulerProfileByteCountChangeEnableGet,
        2, 0
    },
    {
        "cpssPxPortTxSchedulerShaperByteCountChangeValueSet",
        &wrCpssPxPortTxSchedulerShaperByteCountChangeValueSet,
        3, 0
    },
    {
        "cpssPxPortTxSchedulerShaperByteCountChangeValueGet",
        &wrCpssPxPortTxSchedulerShaperByteCountChangeValueGet,
        2, 0
    },
    {
        "cpssPxPortTxSchedulerProfileCountModeSet",
        &wrCpssPxPortTxSchedulerProfileCountModeSet,
        3, 0
    },
    {
        "cpssPxPortTxSchedulerProfileCountModeGet",
        &wrCpssPxPortTxSchedulerProfileCountModeGet,
        2, 0
    }
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortTxScheduler function
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
GT_STATUS cmdLibInitCpssPxPortTxScheduler
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

