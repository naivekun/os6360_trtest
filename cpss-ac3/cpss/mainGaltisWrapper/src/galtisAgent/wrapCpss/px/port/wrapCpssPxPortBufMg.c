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
* @file wrapCpssPxPortBufMg.c
*
* @brief Wrapper functions for
* cpss/Px/port/cpssPxPortBufMg.h API's
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
#include <cpss/px/port/cpssPxPortBufMg.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


/**
* @internal wrCpssPxPortBufMgGlobalXonLimitSet function
* @endinternal
*
* @brief   Sets the Global Xon limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xon limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xonLimit), all ports that support 802.3x Flow Control send an Xon
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xonLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Global Xon limit must be not be greater than Global Xoff Limit.
*
*/
CMD_STATUS wrCpssPxPortBufMgGlobalXonLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_U32                  xonLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    xonLimit    = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortBufMgGlobalXonLimitSet(devNum, xonLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgGlobalXonLimitGet function
* @endinternal
*
* @brief   Gets the Global Xon limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgGlobalXonLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_U32                  xonLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    xonLimit    = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgGlobalXonLimitGet(devNum, &xonLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", xonLimit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgGlobalXoffLimitSet function
* @endinternal
*
* @brief   Sets the Global Xoff limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xoff limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xoffLimit), all ports that support 802.3x Flow Control send a pause
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xoffLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgGlobalXoffLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_U32                  xoffLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    xoffLimit   = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortBufMgGlobalXoffLimitSet(devNum, xoffLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgGlobalXoffLimitGet function
* @endinternal
*
* @brief   Gets the Global Xoff limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgGlobalXoffLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_U32                  xoffLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    xoffLimit   = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgGlobalXoffLimitGet(devNum, &xoffLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", xoffLimit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgRxProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or profileSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgRxProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    profileSet  = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortBufMgRxProfileSet(devNum, portNum, profileSet);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgRxProfileGet function
* @endinternal
*
* @brief   Get the port's flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgRxProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    profileSet  = CPSS_PORT_RX_FC_PROFILE_1_E;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortBufMgRxProfileGet(devNum, portNum, &profileSet);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileSet);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgProfileXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xonLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgProfileXonLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xonLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    profileSet  = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)inArgs[1];
    xonLimit    = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortBufMgProfileXonLimitSet(devNum, profileSet, xonLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgProfileXonLimitGet function
* @endinternal
*
* @brief   Gets the X-ON port limit. When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgProfileXonLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xonLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    profileSet  = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)inArgs[1];
    xonLimit    = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgProfileXonLimitGet(devNum, profileSet, &xonLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", xonLimit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgProfileXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xoffLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgProfileXoffLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xoffLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    profileSet  = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)inArgs[1];
    xoffLimit   = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortBufMgProfileXoffLimitSet(devNum, profileSet, xoffLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgProfileXoffLimitGet function
* @endinternal
*
* @brief   Gets the X-OFF port limit. When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgProfileXoffLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           xoffLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    profileSet  = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)inArgs[1];
    xoffLimit   = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgProfileXoffLimitGet(devNum, profileSet, &xoffLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", xoffLimit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgProfileRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong rxBufLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgProfileRxBufLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           rxBufLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    profileSet  = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)inArgs[1];
    rxBufLimit  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxPortBufMgProfileRxBufLimitSet(devNum, profileSet, rxBufLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgProfileRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgProfileRxBufLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet;
    GT_U32                           rxBufLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    profileSet  = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)inArgs[1];
    rxBufLimit  = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgProfileRxBufLimitGet(devNum, profileSet, &rxBufLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", rxBufLimit);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgGlobalRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgGlobalRxBufNumberGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_U32                  numOfBuffers;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_SW_DEV_NUM)inArgs[0];
    numOfBuffers   = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgGlobalRxBufNumberGet(devNum, &numOfBuffers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", numOfBuffers);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgRxBufNumberGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    GT_U32                           numOfBuffers;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum        = (GT_SW_DEV_NUM)inArgs[0];
    portNum       = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    numOfBuffers  = 0;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortBufMgRxBufNumberGet(devNum, portNum, &numOfBuffers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", numOfBuffers);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgGlobalPacketNumberGet function
* @endinternal
*
* @brief   Gets total number of unique packets currently in the system.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgGlobalPacketNumberGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_U32                  numOfPackets;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_SW_DEV_NUM)inArgs[0];
    numOfPackets   = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgGlobalPacketNumberGet(devNum, &numOfPackets);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", numOfPackets);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgRxMcCntrGet function
* @endinternal
*
* @brief   Gets multicast counter of a certain buffer.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or cntrIdx
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgRxMcCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_U32                           cntrIdx;
    GT_U32                           mcCntr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    cntrIdx     = (GT_U32)inArgs[1];
    mcCntr      = 0;

    /* call cpss api function */
    result = cpssPxPortBufMgRxMcCntrGet(devNum,  cntrIdx, &mcCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mcCntr);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgTxDmaBurstLimitEnableSet function
* @endinternal
*
* @brief   For a given port Enable/Disable TXDMA burst limit thresholds use.
*         When enabled: instruct the TxDMA to throttle the Transmit Queue Scheduler
*         as a function of the TxDMA FIFOs fill level measured in bytes and descriptor.
*         When disabled: the TxDMA throttles the Transmit Queue Scheduler
*         based on FIFOs fill level measured in descriptors only.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgTxDmaBurstLimitEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    GT_BOOL                          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable      = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortBufMgTxDmaBurstLimitEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgTxDmaBurstLimitEnableGet function
* @endinternal
*
* @brief   Gets the current status for a given port of TXDMA burst limit thresholds use.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgTxDmaBurstLimitEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    GT_BOOL                          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable      = GT_FALSE;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortBufMgTxDmaBurstLimitEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgTxDmaBurstLimitThresholdsSet function
* @endinternal
*
* @brief   Sets the TXDMA burst limit thresholds for a given port.
*         The TxDMA supports three throttling levels: Normal, Slow and OFF.
*         The levels are controlled using two thresholds (almostFullThreshold and
*         fullThreshold) measuring the FIFOs fill level.
*         Normal - the Transmit Queue scheduler is not throttled.
*         Slow - the Transmit Queue scheduler is throttled.
*         OFF - the Transmit Queue scheduler is paused.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of almostFullThreshold and fullThreshold fields in hardware
*       is 128 Bytes.
*       The value is rounded down before write to HW.
*
*/
CMD_STATUS wrCpssPxPortBufMgTxDmaBurstLimitThresholdsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    GT_U32                           almostFullThreshold;
    GT_U32                           fullThreshold;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_SW_DEV_NUM)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    almostFullThreshold = (GT_U32)inArgs[2];
    fullThreshold       = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
        devNum, portNum, almostFullThreshold, fullThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortBufMgTxDmaBurstLimitThresholdsGet function
* @endinternal
*
* @brief   Gets the TXDMA burst limit thresholds for a given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortBufMgTxDmaBurstLimitThresholdsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    GT_U32                           almostFullThreshold;
    GT_U32                           fullThreshold;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_SW_DEV_NUM)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    almostFullThreshold = 0;
    fullThreshold       = 0;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
        devNum, portNum, &almostFullThreshold, &fullThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", almostFullThreshold, fullThreshold);

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName,                                    funcReference                        funcArgs funcFields */
    { "cpssPxPortBufMgGlobalXonLimitSet",             &wrCpssPxPortBufMgGlobalXonLimitSet,            2, 0},
    { "cpssPxPortBufMgGlobalXonLimitGet",             &wrCpssPxPortBufMgGlobalXonLimitGet,            1, 0},
    { "cpssPxPortBufMgGlobalXoffLimitSet",            &wrCpssPxPortBufMgGlobalXoffLimitSet,           2, 0},
    { "cpssPxPortBufMgGlobalXoffLimitGet",            &wrCpssPxPortBufMgGlobalXoffLimitGet,           1, 0},
    { "cpssPxPortBufMgRxProfileSet",                  &wrCpssPxPortBufMgRxProfileSet,                 3, 0},
    { "cpssPxPortBufMgRxProfileGet",                  &wrCpssPxPortBufMgRxProfileGet,                 2, 0},
    { "cpssPxPortBufMgProfileXonLimitSet",            &wrCpssPxPortBufMgProfileXonLimitSet,           3, 0},
    { "cpssPxPortBufMgProfileXonLimitGet",            &wrCpssPxPortBufMgProfileXonLimitGet,           2, 0},
    { "cpssPxPortBufMgProfileXoffLimitSet",           &wrCpssPxPortBufMgProfileXoffLimitSet,          3, 0},
    { "cpssPxPortBufMgProfileXoffLimitGet",           &wrCpssPxPortBufMgProfileXoffLimitGet,          2, 0},
    { "cpssPxPortBufMgProfileRxBufLimitSet",          &wrCpssPxPortBufMgProfileRxBufLimitSet,         3, 0},
    { "cpssPxPortBufMgProfileRxBufLimitGet",          &wrCpssPxPortBufMgProfileRxBufLimitGet,         2, 0},
    { "cpssPxPortBufMgGlobalRxBufNumberGet",          &wrCpssPxPortBufMgGlobalRxBufNumberGet,         1, 0},
    { "cpssPxPortBufMgRxBufNumberGet",                &wrCpssPxPortBufMgRxBufNumberGet,               2, 0},
    { "cpssPxPortBufMgGlobalPacketNumberGet",         &wrCpssPxPortBufMgGlobalPacketNumberGet,        1, 0},
    { "cpssPxPortBufMgRxMcCntrGet",                   &wrCpssPxPortBufMgRxMcCntrGet,                  2, 0},
    { "cpssPxPortBufMgTxDmaBurstLimitEnableSet",      &wrCpssPxPortBufMgTxDmaBurstLimitEnableSet,     3, 0},
    { "cpssPxPortBufMgTxDmaBurstLimitEnableGet",      &wrCpssPxPortBufMgTxDmaBurstLimitEnableGet,     2, 0},
    { "cpssPxPortBufMgTxDmaBurstLimitThresholdsSet",  &wrCpssPxPortBufMgTxDmaBurstLimitThresholdsSet, 4, 0},
    { "cpssPxPortBufMgTxDmaBurstLimitThresholdsGet",  &wrCpssPxPortBufMgTxDmaBurstLimitThresholdsGet, 2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxPortMgBuf function
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
GT_STATUS cmdLibInitCpssPxPortMgBuf
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



