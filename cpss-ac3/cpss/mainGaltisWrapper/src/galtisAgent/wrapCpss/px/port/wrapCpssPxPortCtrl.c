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
* @file wrapCpssPxPortCtrl.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortCtrl.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/port/cpssPxPortSyncEther.h>
/**
* @internal wrCpssPxPortForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
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
CMD_STATUS wrCpssPxPortForceLinkPassEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state                           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortForceLinkPassEnableSet(devNum, portNum, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortForceLinkPassEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortForceLinkPassEnableGet(devNum, portNum, &state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", state);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
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
CMD_STATUS wrCpssPxPortForceLinkDownEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state                           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortForceLinkDownEnableSet(devNum, portNum, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortForceLinkDownEnableGet function
* @endinternal
*
* @brief   Get Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortForceLinkDownEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state                           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortForceLinkDownEnableGet(devNum, portNum, &state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", state);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMruSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          mruSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mruSize                         = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMruSet(devNum, portNum, mruSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMruGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          mruSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMruGet(devNum, portNum, &mruSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mruSize);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
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
CMD_STATUS wrCpssPxPortLinkStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         isLinkUp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortLinkStatusGet(devNum, portNum, &isLinkUp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", isLinkUp);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
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
CMD_STATUS wrCpssPxPortInternalLoopbackEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable                          = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortInternalLoopbackEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortInternalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the internal Loopback state in the packet processor MAC port.
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
CMD_STATUS wrCpssPxPortInternalLoopbackEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortInternalLoopbackEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The API rolls back a port's mode and speed to their last values
*       if they cannot be set together on the device.
*       Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*
*/
CMD_STATUS wrCpssPxPortModeSpeedSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PORTS_BMP_STC              portsBmp;
    GT_BOOL                         powerUp;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    powerUp = (GT_BOOL)inArgs[2];
    ifMode  = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[3];
    speed   = (CPSS_PORT_SPEED_ENT)inArgs[4];
    if(portNum >= PRV_CPSS_PX_GOP_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

    /* call cpss api function */
    result = cpssPxPortModeSpeedSet(devNum, &portsBmp, powerUp, ifMode, speed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
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
CMD_STATUS wrCpssPxPortEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable                           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssPxGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)

{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;

}

/**
* @internal wrCpssPxPortEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                          = (GT_SW_DEV_NUM)inArgs[0];
    portNum                         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesAutoTune function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*
* @note Auto tuning should be performed after the port's link is up.
*
*/
static CMD_STATUS wrCpssPxPortSerdesAutoTune
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portTuningMode = (CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesAutoTune(devNum, portNum, portTuningMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesAutoTuneExt function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssPxPortSerdesAutoTuneExt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;
    GT_SW_DEV_NUM                            devNum;
    GT_PHYSICAL_PORT_NUM                     portNum;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode;
    GT_U32      serdesOptAlgBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portTuningMode = (CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT)inArgs[2];
    serdesOptAlgBmp = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesAutoTuneExt(devNum, portNum, portTuningMode,
                                           serdesOptAlgBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesAutoTuneOptAlgGet function
* @endinternal
*
* @brief   Get bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortSerdesAutoTuneOptAlgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U32  serdesOptAlgBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesAutoTuneOptAlgGet(devNum, portNum, &serdesOptAlgBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", serdesOptAlgBmp);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesAutoTuneOptAlgSet function
* @endinternal
*
* @brief   Configure bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API configures field SW DB which will be used by
*       cpssPxPortSerdesAutoTune.
*
*/
static CMD_STATUS wrCpssPxPortSerdesAutoTuneOptAlgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U32  serdesOptAlgBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    serdesOptAlgBmp = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesAutoTuneOptAlgSet(devNum, portNum, serdesOptAlgBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Get current status of RX and TX serdes auto-tuning on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortSerdesAutoTuneStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum, &rxTuneStatus,
                                                 &txTuneStatus);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", rxTuneStatus, txTuneStatus);

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPcsResetSet function
* @endinternal
*
* @brief   Set/unset the PCS reset for given mode on port.
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reset only PCS unit used by port for currently configured interface.
*
*/
CMD_STATUS wrCpssPxPortPcsResetSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM                  devNum;
    GT_PHYSICAL_PORT_NUM           portNum;
    CPSS_PORT_PCS_RESET_MODE_ENT   mode;
    GT_BOOL                        state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    portNum    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode       = (CPSS_PORT_PCS_RESET_MODE_ENT)inArgs[2];
    state      = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    rc = cpssPxPortPcsResetSet(devNum, portNum, mode, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
CMD_STATUS wrCpssPxPortSerdesLanePolarityGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM       devNum;
    GT_U32              portGroupNum;
    GT_U32              laneNum;
    GT_BOOL             invertTx;
    GT_BOOL             invertRx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    rc = cpssPxPortSerdesLanePolarityGet(devNum, portGroupNum, laneNum, &invertTx, &invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "%d %d", invertTx, invertRx);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
CMD_STATUS wrCpssPxPortSerdesLanePolaritySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM       devNum;
    GT_U32      portGroupNum;
    GT_U32      laneNum;
    GT_BOOL     invertTx;
    GT_BOOL     invertRx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    invertTx        = (GT_BOOL)inArgs[3];
    invertRx        = (GT_BOOL)inArgs[4];

    rc = cpssPxPortSerdesLanePolaritySet(devNum, portGroupNum, laneNum, invertTx, invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
CMD_STATUS wrCpssPxPortSerdesLaneTuningGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_SW_DEV_NUM     devNum;
    GT_U32    portGroupNum;
    GT_U32    laneNum;
    CPSS_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_PORT_SERDES_TUNE_STC tuneValues;
    GT_U32 i;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_PORT_SERDES_SPEED_ENT)inArgs[3];

    rc = cpssPxPortSerdesLaneTuningGet(devNum, portGroupNum, laneNum,
                                         serdesFrequency,
                                         &tuneValues);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssPxPortSerdesTuningGet");
        return CMD_OK;
    }


    inFields[0] = tuneValues.BW ;
    inFields[1] = tuneValues.DC;
    inFields[2] = tuneValues.HF;
    inFields[3] = tuneValues.LB;
    inFields[4] = tuneValues.LF;
    inFields[5] = tuneValues.sqlch;
    inFields[6] = tuneValues.txAmp;
    inFields[7] = tuneValues.txEmph1;
    inFields[8] = tuneValues.txEmphAmp;
    for(i = 0; i < CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS; i++)
    {
        inFields[9+i] = tuneValues.DFE[i];
    }
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                    inFields[3], inFields[4], inFields[5], inFields[6], inFields[7]
                , inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15]
                , inFields[16], inFields[17], inFields[18], inFields[19], inFields[20], inFields[21]);



    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssPxPortSerdesPowerStatusSet or cpssPxPortModeSpeedSet will write
*       them to HW.
*
*/
CMD_STATUS wrCpssPxPortSerdesLaneTuningSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          portGroupNum;
    GT_U32                          laneNum;
    CPSS_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_PORT_SERDES_TUNE_STC tuneValues = {0};
    GT_U32 i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_PORT_SERDES_SPEED_ENT)inArgs[3];


    tuneValues.BW        = (GT_U32)inFields[0];
    tuneValues.DC        = (GT_U32)inFields[1];
    tuneValues.HF        = (GT_U32)inFields[2];
    tuneValues.LB        = (GT_U32)inFields[3];
    tuneValues.LF        = (GT_U32)inFields[4];
    tuneValues.sqlch     = (GT_U32)inFields[5];
    tuneValues.txAmp     = (GT_U32)inFields[6];
    tuneValues.txEmph1   = inFields[7];
    tuneValues.txEmphAmp = inFields[8];
    for(i = 0; i < CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS; i++)
    {
        tuneValues.DFE[i] = (GT_U32)inFields[9+i];
    }

    rc = cpssPxPortSerdesLaneTuningSet(devNum,
                                         portGroupNum,
                                         laneNum,
                                         serdesFrequency,
                                         &tuneValues);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesLoopbackModeGet function
* @endinternal
*
* @brief   Get current mode of loopback on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_SUPPORTED         - on not expected mode value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static CMD_STATUS wrCpssPxPortSerdesLoopbackModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      laneNum;
    CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesLoopbackModeGet(devNum, portNum, laneNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Pay attention - configuration of different types of loopbacks at same
*       time not supported by function implementation
*
*/
static CMD_STATUS wrCpssPxPortSerdesLoopbackModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM                         devNum;
    GT_PHYSICAL_PORT_NUM                 portNum;
    CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode = (CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesLoopbackModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Gets and shows configured specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*/
CMD_STATUS wrCpssPxPortSerdesManualRxConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_SW_DEV_NUM                           devNum;
    GT_PHYSICAL_PORT_NUM                   portNum;
    GT_U32                                 laneNum;
    CPSS_PORT_SERDES_RX_CONFIG_STC  serdesRxCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssPxPortSerdesManualRxConfigGet( devNum, portNum, laneNum, &serdesRxCfg);

    galtisOutput(outArgs, result, "%d%d%d%d%d%d",
        serdesRxCfg.sqlch, serdesRxCfg.ffeRes, serdesRxCfg.ffeCap,serdesRxCfg.dcGain, serdesRxCfg.bandWidth, serdesRxCfg.loopBandwidth);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
CMD_STATUS wrCpssPxPortSerdesManualRxConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM            devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  laneNum;
    CPSS_PORT_SERDES_RX_CONFIG_STC  serdesCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    serdesCfg.sqlch = (GT_U32)inArgs[3];
    serdesCfg.ffeRes = (GT_U32)inArgs[4];
    serdesCfg.ffeCap = (GT_U32)inArgs[5];
    serdesCfg.dcGain = (GT_U32)inArgs[6];
    serdesCfg.bandWidth = (GT_U32)inArgs[7];
    serdesCfg.loopBandwidth = (GT_U32)inArgs[8];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesManualRxConfigSet(devNum, portNum, laneNum,
                                                 &serdesCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Gets and shows configured specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*/
CMD_STATUS wrCpssPxPortSerdesManualTxConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  laneNum;
    CPSS_PORT_SERDES_TX_CONFIG_STC  serdesTxCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssPxPortSerdesManualTxConfigGet( devNum, portNum, laneNum, &serdesTxCfg);

    galtisOutput(outArgs, result, "%d%d%d",serdesTxCfg.txAmp, serdesTxCfg.emph0, serdesTxCfg.emph1);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
CMD_STATUS wrCpssPxPortSerdesManualTxConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          laneNum;
    CPSS_PORT_SERDES_TX_CONFIG_STC  serdesCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    serdesCfg.txAmp = (GT_U32)inArgs[3];

        serdesCfg.emph0 = inArgs[4];
        serdesCfg.emph1 = inArgs[5];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesManualTxConfigSet(devNum, portNum, laneNum,
                                                 &serdesCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesPolarityGet function
* @endinternal
*
* @brief   Get status of the Tx or Rx serdes polarity invert.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static CMD_STATUS wrCpssPxPortSerdesPolarityGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32                     laneNum;
    GT_BOOL                    invertTx;
    GT_BOOL                    invertRx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesPolarityGet(devNum, portNum, laneNum, &invertTx,
                                            &invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d", invertTx, invertRx);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesPolaritySet function
* @endinternal
*
* @brief   Invert the Tx or Rx serdes polarity.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port
*       disable and link_change interrupt lock and restore port enable and
*       reenable link change interrupt only after "serdes polarity set" to
*       prevent interrupt toggling during the process.
*
*/
static CMD_STATUS wrCpssPxPortSerdesPolaritySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM                  devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32                     laneBmp;
    GT_BOOL                   invertTx;
    GT_BOOL                   invertRx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneBmp = (GT_U32)inArgs[2];
    invertTx = (GT_BOOL)inArgs[3];
    invertRx = (GT_BOOL)inArgs[4];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesPolaritySet(devNum, portNum, laneBmp, invertTx,
                                            invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesEnhancedAutoTuneSet function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortSerdesEnhancedAutoTuneSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U8       minLF;
    GT_U8       maxLF;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    minLF = (GT_U8)inArgs[2];
    maxLF = (GT_U8)inArgs[3];

    /* call cpss api function */
    result = cpssPxPortSerdesEnhancedAutoTune(devNum, portNum, minLF, maxLF);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh3 and above devices supports only GT_PORT_DIRECTION_BOTH_RX_TX.
*
*/
static CMD_STATUS wrCpssPxPortSerdesPowerStatusSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_SW_DEV_NUM                          devNum;
    GT_PHYSICAL_PORT_NUM                   portNum;
    GT_BOOL                                powerUp;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    powerUp = (GT_U8)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesPowerStatusSet(devNum, portNum, powerUp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssPxPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortSerdesResetStateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM            devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL   state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesResetStateSet(devNum, portNum, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesTuningGet function
* @endinternal
*
* @brief   SerDes fine tuning values get.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reads values saved in SW DB by cpssPxPortSerdesTuningSet or
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
*/
static CMD_STATUS wrCpssPxPortSerdesTuningGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_SW_DEV_NUM            devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_U32    laneNum;
    CPSS_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_PORT_SERDES_TUNE_STC tuneValues;
    GT_U32 i;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum     = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_PORT_SERDES_SPEED_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    rc = cpssPxPortSerdesTuningGet(devNum,portNum,laneNum,serdesFrequency,
                                        &tuneValues);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssPxPortSerdesTuningGet");
        return CMD_OK;
    }

    inFields[0] = tuneValues.BW ;
    inFields[1] = tuneValues.DC;
    inFields[2] = tuneValues.HF;
    inFields[3] = tuneValues.LB;
    inFields[4] = tuneValues.LF;
    inFields[5] = tuneValues.sqlch;
    inFields[6] = tuneValues.txAmp;
    inFields[7] = tuneValues.txEmph1;
    inFields[8] = tuneValues.txEmphAmp;
    for(i = 0; i < CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS; i++)
    {
        inFields[9+i] = tuneValues.DFE[i];
    }
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                    inFields[3], inFields[4], inFields[5], inFields[6], inFields[7]
                , inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15]
                , inFields[16], inFields[17], inFields[18], inFields[19], inFields[20], inFields[21]);


    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesTuningSet function
* @endinternal
*
* @brief   SerDes fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set same tuning parameters for all lanes defined in laneBmp in SW DB,
*       then cpssPxPortSerdesPowerStatusSet will write them to HW.
*
*/
static CMD_STATUS wrCpssPxPortSerdesTuningSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_SW_DEV_NUM            devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_U32    laneNum;
    CPSS_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_PORT_SERDES_TUNE_STC tuneValues = {0};
    GT_U32 i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_SW_DEV_NUM)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum     = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_PORT_SERDES_SPEED_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

        tuneValues.BW        = (GT_U32)inFields[0];
        tuneValues.DC        = (GT_U32)inFields[1];
        tuneValues.HF        = (GT_U32)inFields[2];
        tuneValues.LB        = (GT_U32)inFields[3];
        tuneValues.LF        = (GT_U32)inFields[4];
        tuneValues.sqlch     = (GT_U32)inFields[5];
        tuneValues.txAmp     = (GT_U32)inFields[6];
        tuneValues.txEmph1   = inFields[7];
        tuneValues.txEmphAmp = inFields[8];

        for(i = 0; i < CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS; i++)
        {
            tuneValues.DFE[i] = (GT_U32)inFields[9+i];
        }

    rc = cpssPxPortSerdesTuningSet(devNum,portNum,(1<<laneNum),
                                        serdesFrequency,
                                        &tuneValues);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesTxEnableGet function
* @endinternal
*
* @brief   Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortSerdesTxEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesTxEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSerdesTxEnableSet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
static CMD_STATUS wrCpssPxPortSerdesTxEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortSerdesTxEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}




/*******************************************************************************
* wrCpssPxPortAttributesOnPortGet
*
* DESCRIPTION:
*       wrapper for port statistics
*
* APPLICABLE DEVICES:  pipe
*
* INPUTS:
*       devNum     - device number
*       portNum    - port number
*
* OUTPUTS:
*       portAttributSetArrayPtr - Pointer to attributes values array.
*
* RETURNS:
*       GT_OK           - on success
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_U8    wrPxDevNum = 0;
static GT_U32   wrPxPortNum = 0;
static GT_U32   ignorePortInLinkDown = 0;


 static GT_STATUS portPxAttributesGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result1, result2, result3,result4,result5,result6,result7;
    GT_BOOL flag1,flag2;

    OUT CPSS_PORT_ATTRIBUTES_STC        portAttributSetArray;
    OUT CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    OUT CPSS_PORT_FLOW_CONTROL_ENT      flowCtrlEnable;
    OUT GT_BOOL                         speedAutoNegEnable;
    OUT GT_BOOL                         duplexAutoNegEnable;

    OUT GT_BOOL                         flowCtrlAutoNegEnable;
    OUT GT_BOOL                         pauseAdvertise;
    OUT GT_U32                          mruSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return GT_INIT_ERROR;

checkCurrentPort_lbl:

    /* call cpss api function */
    result2 = cpssPxPortInterfaceModeGet(wrPxDevNum,wrPxPortNum,&ifMode);
    if(result2 == GT_BAD_PARAM && wrPxPortNum < CPSS_MAX_PORTS_NUM_CNS)
    {
        /* case that the port not valid */
        result7 = cpssPxPortMruGet(wrPxDevNum,wrPxPortNum,&mruSize);
        if(result7 == GT_BAD_PARAM)
        {
            /* case that the port not valid */
            wrPxPortNum++;

            goto checkCurrentPort_lbl;
        }
    }


    if((GT_OK == result2) && (CPSS_PORT_INTERFACE_MODE_HGL_E == ifMode))
    {
        CPSS_PORT_SPEED_ENT speed;
        GT_BOOL             linkUp;

        result2 = cpssPxPortSpeedGet(wrPxDevNum, wrPxPortNum, &speed);
        if(result2 != GT_OK)
        {
            galtisOutput(outArgs, result2, "%d", -1);
            return GT_OK;
        }

        if(speed != CPSS_PORT_SPEED_40000_E)
        {/* 40G is XLHGL supported by XLG MAC */
            inFields[0] = wrPxDevNum;
            inFields[1] = wrPxPortNum;

            result2 = cpssPxPortLinkStatusGet(wrPxDevNum, wrPxPortNum, &linkUp);
            if(result2 != GT_OK)
            {
                galtisOutput(outArgs, result2, "%d", -1);
                return GT_OK;
            }

            inFields[2] = linkUp;
            inFields[3] = speed;
            inFields[4] = 0;
            inFields[5] = ifMode;
            inFields[6] = 0;
            inFields[7] = 0;
            inFields[8] = 0;
            inFields[9] = 0;
            inFields[10] = 0;
            inFields[11] = 0;

            /* pack and output table fields */
            fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d",  inFields[0],  inFields[1],  inFields[2], inFields[3], inFields[4],
                                       inFields[5],  inFields[6],inFields[7],  inFields[8],inFields[9],  inFields[10],
                                       inFields[11]);
            galtisOutput(outArgs, result2, "%f");
            return GT_OK;
        }
    }

    result1 = cpssPxPortAttributesOnPortGet(wrPxDevNum, wrPxPortNum,
                                    &portAttributSetArray);
    if(PRV_CPSS_PP_MAC(wrPxDevNum) &&
       (wrPxPortNum < PRV_CPSS_PP_MAC(wrPxDevNum)->numOfPorts) &&
       ignorePortInLinkDown &&
       portAttributSetArray.portLinkUp == GT_FALSE)
    {
        /* skip ports with link down */
        wrPxPortNum++;

        goto checkCurrentPort_lbl;
    }

    result3 = cpssPxPortFlowControlEnableGet(wrPxDevNum,wrPxPortNum,&flowCtrlEnable);
    if (wrPxPortNum != CPSS_CPU_PORT_NUM_CNS)
    {
        result4 = cpssPxPortSpeedAutoNegEnableGet(wrPxDevNum,wrPxPortNum,&speedAutoNegEnable);
        result5 = cpssPxPortDuplexAutoNegEnableGet(wrPxDevNum,wrPxPortNum,&duplexAutoNegEnable);
        result6 = cpssPxPortFlowCntrlAutoNegEnableGet(wrPxDevNum,wrPxPortNum,
                                                            &flowCtrlAutoNegEnable,
                                                            &pauseAdvertise);
    }
    else
    {
        speedAutoNegEnable = GT_FALSE;
        result4 = GT_OK;

        duplexAutoNegEnable = GT_FALSE;
        result5 = GT_OK;

        flowCtrlAutoNegEnable = GT_FALSE;
        pauseAdvertise = GT_FALSE;
        result6 = GT_OK;
    }

    result7 = cpssPxPortMruGet(wrPxDevNum,wrPxPortNum,&mruSize);
    if(result1 == GT_NOT_APPLICABLE_DEVICE)
    {
        galtisOutput(outArgs, result1, "%d", -1);
        return GT_NOT_APPLICABLE_DEVICE;
    }

    flag1=(result1 != GT_OK)||(result2 != GT_OK)||(result3 != GT_OK)||(result7 != GT_OK);
    flag2=(result4 != GT_OK)||(result5 != GT_OK)||(result6 != GT_OK);

    if (flag1 || (flag2 && wrPxPortNum != CPSS_CPU_PORT_NUM_CNS))
    {
        galtisOutput(outArgs, result1, "%d", -1);
        return GT_OK;
    }

    inFields[0] = wrPxDevNum;
    inFields[1] = wrPxPortNum;
    inFields[2] = portAttributSetArray.portLinkUp;
    inFields[3] = portAttributSetArray.portSpeed;
    inFields[4] = portAttributSetArray.portDuplexity;
    inFields[5] = ifMode;
    inFields[6] = flowCtrlEnable;
    inFields[7] = speedAutoNegEnable;
    inFields[8] = duplexAutoNegEnable;
    inFields[9] = flowCtrlAutoNegEnable;
    inFields[10] = pauseAdvertise;
    inFields[11] = mruSize;



    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d",  inFields[0],  inFields[1],  inFields[2], inFields[3], inFields[4],
                               inFields[5],  inFields[6],inFields[7],  inFields[8],inFields[9],  inFields[10],
                               inFields[11]);


    galtisOutput(outArgs, result1, "%f");

    return GT_OK;
}

/*****************************************************************************************/
 static CMD_STATUS wrCpssPxPortAttributesGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result=GT_OK;

    wrPxDevNum = 0;
    wrPxPortNum = 0;

    while(wrPxDevNum < 64)
    {
        if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(wrPxDevNum))
        {
            wrPxDevNum++;
            continue;
        }

        result = portPxAttributesGet(inArgs,inFields,numFields,outArgs);
        if(result == GT_NOT_APPLICABLE_DEVICE)
        {
            if(wrPxPortNum == 84)
            {
                wrPxPortNum = 0;
                wrPxDevNum++;
            }
            wrPxPortNum++;
        }
        else break;
    }
    if (wrPxDevNum == 64)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    wrPxPortNum++;

    return CMD_OK;
}
/*******************************************************************************/
 static CMD_STATUS wrCpssPxPortAttributesGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result = GT_OK;
    CPSS_PX_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

    while(1)
    {
        if(wrPxDevNum == PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            /* no ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        if((!PRV_CPSS_IS_DEV_EXISTS_MAC(wrPxDevNum)) ||
           (wrPxPortNum == CPSS_MAX_PORTS_NUM_CNS))
        {
            /* move to the next PP */
            wrPxDevNum++;
            wrPxPortNum = 0;
            continue;
        }
        else
        {
            if (PRV_CPSS_PHY_PORT_IS_EXIST_MAC(wrPxDevNum, wrPxPortNum))
            {
                result = cpssPxPortPhysicalPortDetailedMapGet(wrPxDevNum, wrPxPortNum, /*OUT*/portMapShadowPtr);
                if (result != GT_OK)
                {
                    return result;
                }

                if((portMapShadowPtr->valid != GT_TRUE) || (portMapShadowPtr->portMap.mappingType !=
                                                            CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E))
                {
                    wrPxPortNum++;
                    continue;
                }
                /* get port's attributes */
                result = portPxAttributesGet(inArgs,inFields,numFields,outArgs);
                break;
            }
            else
            {
                wrPxPortNum++;
            }
        }
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* move to the next portNum */
    wrPxPortNum++;

    return CMD_OK;
}

/**
* @internal wrCpssPxPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortFlowControlModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PORT_FC_MODE_ENT fcMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_SW_DEV_NUM)inArgs[1];
    fcMode = (CPSS_PX_PORT_FC_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortFlowControlModeSet(devNum, portNum, fcMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortFlowControlModeGet function
* @endinternal
*
* @brief   Gets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortFlowControlModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PORT_FC_MODE_ENT fcMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortFlowControlModeGet(devNum, portNum, &fcMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", fcMode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
*/
static CMD_STATUS wrCpssPxPortSpeedAutoNegEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortSpeedAutoNegEnableSet(devNum, portNum, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation of interface speed on specified port
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the speed auto negotiation
*
*/
static CMD_STATUS wrCpssPxPortSpeedAutoNegEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    OUT  GT_BOOL           state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];


    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortSpeedAutoNegEnableGet(devNum, portNum, &state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",state);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortDuplexModeSet function
* @endinternal
*
* @brief   Set the port to enable and the port mode to half- or full-duplex mode.
*         When port is disabled it does not transmit pending packets and
*         drops packets received from the line.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or dMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported duplex mode
*
* @note The <SetFullDx> bit is meaningless when <ANDuplex> is set.
*
*/
static CMD_STATUS wrCpssPxPortDuplexModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    CPSS_PORT_DUPLEX_ENT  dMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    dMode = (CPSS_PORT_DUPLEX_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortDuplexModeSet(devNum, portNum, dMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssPxPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortDuplexModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    CPSS_PORT_DUPLEX_ENT   dModePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortDuplexModeGet(devNum, portNum, &dModePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dModePtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
*
* @note For XGMII (10 Gbps) ports feature is not supported.
*       If the port is enabled then the function disables the port before the
*       operation and re-enables it at the end.
*
*/
static CMD_STATUS wrCpssPxPortBackPressureEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;
    GT_SW_DEV_NUM            devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL                  state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortBackPressureEnableSet(devNum, portNum, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For XGMII (10 Gbps) ports feature is not supported.
*
*/
static CMD_STATUS wrCpssPxPortBackPressureEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortBackPressureEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortFlowControlEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL                state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state = (CPSS_PORT_FLOW_CONTROL_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortFlowControlEnableSet(devNum, portNum, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortFlowControlEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PORT_FLOW_CONTROL_ENT state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortFlowControlEnableGet(devNum, portNum, &state);

    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", state);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
*/
static CMD_STATUS wrCpssPxPortFlowCntrlAutoNegEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL                state;
    GT_BOOL                pauseAdvertise;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state = (GT_BOOL)inArgs[2];
    pauseAdvertise = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortFlowCntrlAutoNegEnableSet(devNum, portNum, state,
                                                           pauseAdvertise);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
static CMD_STATUS wrCpssPxPortFlowCntrlAutoNegEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL                state;
    GT_BOOL                pauseAdvertise;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortFlowCntrlAutoNegEnableGet(devNum, portNum, &state,
                                                           &pauseAdvertise);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",state,pauseAdvertise);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
*/
static CMD_STATUS wrCpssPxPortDuplexAutoNegEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortDuplexAutoNegEnableSet(devNum, portNum, state);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the duplex auto negotiation
*
*/
static CMD_STATUS wrCpssPxPortDuplexAutoNegEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL                state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortDuplexAutoNegEnableGet(devNum, portNum, &state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",state);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortCrcCheckEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL                enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortCrcCheckEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCrcCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortCrcCheckEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*
*/
static CMD_STATUS wrCpssPxPortExcessiveCollisionDropEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortExcessiveCollisionDropEnableSet(devNum, portNum, enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Not relevant in full duplex mode
*
*/
static CMD_STATUS wrCpssPxPortExcessiveCollisionDropEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortExcessiveCollisionDropEnableGet(devNum, portNum, &enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssPxPortInBandAutoNegBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable Auto-Negotiation by pass.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Supported for ports in 1000Base-X mode only.
*       Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*
*/
static CMD_STATUS wrCpssPxPortInBandAutoNegBypassEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL         enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortInBandAutoNegBypassEnableSet(devNum, portNum, enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortInBandAutoNegBypassEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation by pass status.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Supported for ports in 1000Base-X mode only.
*       Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*
*/
static CMD_STATUS wrCpssPxPortInBandAutoNegBypassEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortInBandAutoNegBypassEnableGet(devNum, portNum, &enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortMacResetStateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 state;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    state = (GT_BOOL)inArgs[2];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortMacResetStateSet(devNum, portNum, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortForward802_3xEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*         pipeline of a specified port. Processing of 802.3x Flow Control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port�s packet transmission if it is an XOFF packet, or to
*       resume the port�s packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet�s Length/EtherType field is 88-08
*       - Packet�s OpCode field is 00-01
*       - Packet�s MAC DA is 01-80-C2-00-00-01 or the port�s configured MAC Address
*
*/
static CMD_STATUS wrCpssPxPortForward802_3xEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortForward802_3xEnableSet(devNum, portNum, enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortForward802_3xEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x frames forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port packet transmission if it is an XOFF packet, or to
*       resume the port packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet Length/EtherType field is 88-08
*       - Packet OpCode field is 00-01
*       - Packet MAC DA is 01-80-C2-00-00-01 or the port configured MAC Address
*
*/
static CMD_STATUS wrCpssPxPortForward802_3xEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortForward802_3xEnableGet(devNum, portNum, &enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortForwardUnknownMacControlFramesEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of unknown MAC control frames to the ingress
*         pipeline of a specified port. Processing of unknown MAC control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet Length/EtherType field is 88-08
*       - Packet OpCode field is not 00-01 and not 01-01
*       OR
*       Packet MAC DA is not 01-80-C2-00-00-01 and not the ports configured
*       MAC Address
*
*/
static CMD_STATUS wrCpssPxPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortForwardUnknownMacControlFramesEnableSet(devNum,
                                                            portNum, enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortForwardUnknownMacControlFramesEnableGet function
* @endinternal
*
* @brief   Get current status of unknown MAC control frames
*         forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet Length/EtherType field is 88-08
*       - Packet OpCode field is not 00-01 and not 01-01
*       OR
*       Packet MAC DA is not 01-80-C2-00-00-01 and not the port configured
*       MAC Address
*
*/
static CMD_STATUS wrCpssPxPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortForwardUnknownMacControlFramesEnableGet(devNum,
                                                        portNum, &enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortInbandAutoNegEnableSet function
* @endinternal
*
* @brief   Configure Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
*
* @note Supported only on DX and SOHO packet processors.
*       Not supported for CPU port of DX devices and not supported for XG ports.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*
*/
static CMD_STATUS wrCpssPxPortInbandAutoNegEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortInbandAutoNegEnableSet(devNum, portNum, enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortInbandAutoNegEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port of DX devices and not supported for XG ports.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       Not relevant for the CPU port
*
*/
static CMD_STATUS wrCpssPxPortInbandAutoNegEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortInbandAutoNegEnableGet(devNum, portNum, &enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortIpgSet function
* @endinternal
*
* @brief   Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*         Using this API may be required to enable wire-speed in traffic paths
*         that include cascading ports, where it may not be feasible to reduce
*         the preamble length.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_OUT_OF_RANGE          - ipg value out of range
* @retval GT_BAD_PARAM             - on bad parameter
*/
static CMD_STATUS wrCpssPxPortIpgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    IN  GT_SW_DEV_NUM       devNum;
    IN  GT_PHYSICAL_PORT_NUM   portNum;
    IN  GT_U32              ipg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ipg     = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortIpgSet(devNum, portNum, ipg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}



/**
* @internal wrCpssPxPortIpgGet function
* @endinternal
*
* @brief   Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssPxPortIpgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    IN  GT_SW_DEV_NUM       devNum;
    IN  GT_PHYSICAL_PORT_NUM portNum;
    OUT GT_U32              ipg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum)
    /* call cpss api function */
    result = cpssPxPortIpgGet(devNum, portNum, &ipg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ipg);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortIpgBaseSet function
* @endinternal
*
* @brief   Sets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
static CMD_STATUS wrCpssPxPortIpgBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PORT_XG_FIXED_IPG_ENT ipgBase;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ipgBase = (CPSS_PORT_XG_FIXED_IPG_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortIpgBaseSet(devNum, portNum, ipgBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortIpgBaseGet function
* @endinternal
*
* @brief   Gets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
static CMD_STATUS wrCpssPxPortIpgBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PORT_XG_FIXED_IPG_ENT ipgBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortIpgBaseGet(devNum, portNum, &ipgBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ipgBase);

    return CMD_OK;
}
/**
* @internal wrCpssPxPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
*/
static CMD_STATUS wrCpssPxPortPreambleLengthSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;
    GT_SW_DEV_NUM            devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    CPSS_PORT_DIRECTION_ENT  direction;
    GT_U32                   length;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[2];
    length = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPreambleLengthSet(devNum, portNum, direction, length);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortPreambleLengthGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_DIRECTION_ENT direction;
    GT_U32              length;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPreambleLengthGet(devNum, portNum, direction, &length);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", length);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortPeriodicFcEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL                enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPeriodicFcEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortPeriodicFcEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPeriodicFcEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortFecModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM    devNum;
    GT_PHYSICAL_PORT_NUM      portNum;
    CPSS_PORT_FEC_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode = (CPSS_PORT_FEC_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortFecModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortFecModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result = GT_OK;
    GT_SW_DEV_NUM               devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    CPSS_PORT_FEC_MODE_ENT mode = CPSS_PORT_FEC_MODE_DISABLED_E;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortFecModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortPeriodicFlowControlCounterSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortPeriodicFlowControlCounterSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  value;
    GT_SW_DEV_NUM           devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    value = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    result = cpssPxPortPeriodicFlowControlCounterSet(devNum, portNum, value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}



/**
* @internal wrCpssPxPortPeriodicFlowControlCounterGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortPeriodicFlowControlCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  value;
    GT_SW_DEV_NUM           devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    result = cpssPxPortPeriodicFlowControlCounterGet(devNum, portNum, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacSaLsbSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*
* @note see srcAddrMiddle and srcAddrHigh as related parameters..
*
*/
static CMD_STATUS wrCpssPxPortMacSaLsbSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U8                  macSaLsb;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    macSaLsb = (GT_U8)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortMacSaLsbSet(devNum, portNum, macSaLsb);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacSaLsbGet function
* @endinternal
*
* @brief   Get the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note see srcAddrMiddle and srcAddrHigh as related parameters.
*
*/
static CMD_STATUS wrCpssPxPortMacSaLsbGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U8                 macSaLsb;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortMacSaLsbGet(devNum, portNum, &macSaLsb);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", macSaLsb);
    return CMD_OK;
}


/**
* @internal wrCpssPxPortMacSaBaseSet function
* @endinternal
*
* @brief   Sets the Source addresses of all of the device's ports.
*         This addresses is used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition this address is used for
*         reception of Flow Control packets.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssPxPortMacSaBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_SW_DEV_NUM devNum;
    GT_ETHERADDR  macPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    galtisMacAddr(&macPtr, (GT_U8*)inArgs[1]);

    /* call cpss api function */
    result = cpssPxPortMacSaBaseSet(devNum, &macPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssPxPortMacSaBaseGet function
* @endinternal
*
* @brief   Sets the Source addresses of all of the device's ports.
*         This addresses is used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition this address is used for
*         reception of Flow Control packets.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssPxPortMacSaBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_SW_DEV_NUM   devNum;
    GT_ETHERADDR    macPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxPortMacSaBaseGet(devNum, &macPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%6b", macPtr.arEther);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortXGmiiModeSet function
* @endinternal
*
* @brief   Sets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortXGmiiModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 result;
    GT_SW_DEV_NUM             devNum;
    GT_PHYSICAL_PORT_NUM      portNum;
    CPSS_PORT_XGMII_MODE_ENT  mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode = (CPSS_PORT_XGMII_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXGmiiModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortXGmiiModeGet function
* @endinternal
*
* @brief   Gets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortXGmiiModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_XGMII_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXGmiiModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortExtraIpgSet function
* @endinternal
*
* @brief   Sets the number of 32-bit words to add to the 12-byte IPG.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortExtraIpgSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS              result;
    GT_SW_DEV_NUM          devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U8                  number;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    number = (GT_U8)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortExtraIpgSet(devNum, portNum, number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortExtraIpgGet function
* @endinternal
*
* @brief   Gets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortExtraIpgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U8                number;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortExtraIpgGet(devNum, portNum, &number);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", number);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortXgmiiLocalFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected local
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortXgmiiLocalFaultGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               isLocalFaultPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXgmiiLocalFaultGet(devNum, portNum, &isLocalFaultPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", isLocalFaultPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortXgmiiRemoteFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected remote
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxPortXgmiiRemoteFaultGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               isRemoteFaultPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXgmiiRemoteFaultGet(devNum, portNum,
                                           &isRemoteFaultPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", isRemoteFaultPtr);
    return CMD_OK;
}

/*******************************************************************************
* cpssPxPortMacStatusGet
*
* DESCRIPTION:
*       Reads bits that indicate different problems on specified port.
*
* APPLICABLE DEVICES:  pipe
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*
* OUTPUTS:
*       portMacStatusPtr  - info about port MAC
*
* RETURNS:
*       GT_OK            - on success
*       GT_BAD_PARAM     - on wrong port number or device
*       GT_BAD_PTR       - one of the parameters is NULL pointer
*       GT_HW_ERROR      - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_PHYSICAL_PORT_NUM portNumber;

static CMD_STATUS wrCpssPxPortMacStatusGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PORT_MAC_STATUS_STC        portMacStatusPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNumber = 0;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNumber);

    /* call cpss api function */
    result = cpssPxPortMacStatusGet(devNum, portNumber, &portMacStatusPtr);

    while (result != GT_OK)
    {
        portNumber++;

        if (portNumber > 15)
        {
            galtisOutput(outArgs, CMD_OK, "%d", -1);
            return CMD_OK;
        }
        /* call cpss api function */
        result = cpssPxPortMacStatusGet(devNum, portNumber, &portMacStatusPtr);
    }

    inFields[0] = portMacStatusPtr.isPortRxPause;
    inFields[1] = portMacStatusPtr.isPortTxPause;
    inFields[2] = portMacStatusPtr.isPortBackPres;
    inFields[3] = portMacStatusPtr.isPortBufFull;
    inFields[4] = portMacStatusPtr.isPortSyncFail;
    inFields[5] = portMacStatusPtr.isPortHiErrorRate;
    inFields[6] = portMacStatusPtr.isPortAnDone;
    inFields[7] = portMacStatusPtr.isPortFatalError;

    portNumber++;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",  inFields[0], inFields[1],
                      inFields[2], inFields[3], inFields[4],
                      inFields[5], inFields[6], inFields[7]);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/*******************************************************************************/
static CMD_STATUS wrCpssPxPortMacStatusGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PORT_MAC_STATUS_STC        portMacStatusPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNumber);

    /* call cpss api function */
    result = cpssPxPortMacStatusGet(devNum, portNumber, &portMacStatusPtr);

    while(result != GT_OK)
    {
        portNumber++;

        if (portNumber > 254)
        {
            galtisOutput(outArgs, CMD_OK, "%d", -1);
            return CMD_OK;
        }

        /* Override Device and Port */
        CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNumber);

        /* call cpss api function */
        result = cpssPxPortMacStatusGet(devNum, portNumber, &portMacStatusPtr);
    }

    inFields[0] = portMacStatusPtr.isPortRxPause;
    inFields[1] = portMacStatusPtr.isPortTxPause;
    inFields[2] = portMacStatusPtr.isPortBackPres;
    inFields[3] = portMacStatusPtr.isPortBufFull;
    inFields[4] = portMacStatusPtr.isPortSyncFail;
    inFields[5] = portMacStatusPtr.isPortHiErrorRate;
    inFields[6] = portMacStatusPtr.isPortAnDone;
    inFields[7] = portMacStatusPtr.isPortFatalError;

    portNumber++;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",  inFields[0], inFields[1],
                      inFields[2], inFields[3], inFields[4],
                      inFields[5], inFields[6], inFields[7]);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssPxPortPaddingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPaddingEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssPxPortPaddingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPaddingEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPcsLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPcsLoopbackModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode = (CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPcsLoopbackModeGet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPcsLoopbackModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortXgLanesSwapEnableSet function
* @endinternal
*
* @brief   Enable/Disable swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
static CMD_STATUS wrCpssPxPortXgLanesSwapEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXgLanesSwapEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortXgLanesSwapEnableGet function
* @endinternal
*
* @brief   Gets status of swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
static CMD_STATUS wrCpssPxPortXgLanesSwapEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL             enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXgLanesSwapEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortXgPscLanesSwapSet function
* @endinternal
*
* @brief   Set swapping configuration of XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - wrong SERDES lane
* @retval GT_BAD_VALUE             - multiple connection detected
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
static CMD_STATUS wrCpssPxPortXgPscLanesSwapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    rxSerdesLaneArr[0] = (GT_U32)inArgs[2];
    rxSerdesLaneArr[1] = (GT_U32)inArgs[3];
    rxSerdesLaneArr[2] = (GT_U32)inArgs[4];
    rxSerdesLaneArr[3] = (GT_U32)inArgs[5];
    txSerdesLaneArr[0] = (GT_U32)inArgs[6];
    txSerdesLaneArr[1] = (GT_U32)inArgs[7];
    txSerdesLaneArr[2] = (GT_U32)inArgs[8];
    txSerdesLaneArr[3] = (GT_U32)inArgs[9];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXgPscLanesSwapSet(devNum, portNum,
                                           rxSerdesLaneArr, txSerdesLaneArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortXgPscLanesSwapGet function
* @endinternal
*
* @brief   Get swapping configuration of XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
static CMD_STATUS wrCpssPxPortXgPscLanesSwapGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxPortXgPscLanesSwapGet(devNum, portNum,
                                           rxSerdesLaneArr, txSerdesLaneArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d",
                 rxSerdesLaneArr[0],
                 rxSerdesLaneArr[1],
                 rxSerdesLaneArr[2],
                 rxSerdesLaneArr[3],
                 txSerdesLaneArr[0],
                 txSerdesLaneArr[1],
                 txSerdesLaneArr[2],
                 txSerdesLaneArr[3]);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortAutoNegAdvertismentConfigGet function
* @endinternal
*
* @brief   Getting <TX Config Reg> data for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on wrong port mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_VALUE             - on wrong speed value in the register
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
static CMD_STATUS wrCpssPxPortAutoNegAdvertismentConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC portAnAdvertisment;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortAutoNegAdvertismentConfigGet(devNum, portNum, &portAnAdvertisment);

    /* pack output arguments to galtis string */
    if(GT_OK == result)
    {
        galtisOutput(outArgs, result, "%d %d %d", portAnAdvertisment.link, portAnAdvertisment.speed, portAnAdvertisment.duplex);
    }
    else
    {
       galtisOutput(outArgs, result, "");
    }

    return CMD_OK;
}

/**
* @internal wrCpssPxPortAutoNegAdvertismentConfigSet function
* @endinternal
*
* @brief   Configure <TX Config Reg> for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
static CMD_STATUS wrCpssPxPortAutoNegAdvertismentConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC portAnAdvertisment;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portAnAdvertisment.link = (GT_BOOL)inArgs[2];
    portAnAdvertisment.speed = (CPSS_PORT_SPEED_ENT)inArgs[3];
    portAnAdvertisment.duplex = (CPSS_PORT_DUPLEX_ENT)inArgs[4];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortAutoNegAdvertismentConfigSet(devNum, portNum, &portAnAdvertisment);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortAutoNegMasterModeEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
static CMD_STATUS wrCpssPxPortAutoNegMasterModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL  enable;
    GT_CHAR *enablestatus;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortAutoNegMasterModeEnableGet(devNum, portNum, &enable);

    switch(enable)
    {
    case GT_TRUE:
        enablestatus = "enabled";
        break;
    default:
        enablestatus = "disabled";
        break;
    }

    /* pack output arguments to galtis string */
    if(GT_OK == result)
    {
        galtisOutput(outArgs, result, "%s", enablestatus);
    }
    else
    {
        galtisOutput(outArgs, result, "");
    }
    return CMD_OK;
}

/**
* @internal wrCpssPxPortAutoNegMasterModeEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
static CMD_STATUS wrCpssPxPortAutoNegMasterModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM   devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortAutoNegMasterModeEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortRefClockSourceOverrideEnableGet function
* @endinternal
*
* @brief   Gets status of reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortRefClockSourceOverrideEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         overrideEnable;
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortRefClockSourceOverrideEnableGet(devNum, portNum, &overrideEnable, &refClockSource);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", overrideEnable, refClockSource);

    return CMD_OK;

}

/**
* @internal wrCpssPxPortRefClockSourceOverrideEnableSet function
* @endinternal
*
* @brief   Enables/disables reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, refClockSource
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortRefClockSourceOverrideEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         overrideEnable;
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    overrideEnable = (GT_BOOL)inArgs[2];
    refClockSource = (CPSS_PORT_REF_CLOCK_SOURCE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortRefClockSourceOverrideEnableSet(devNum, portNum, overrideEnable, refClockSource);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssPxPortAttributesOnPortGet function
* @endinternal
*
* @brief   Gets port attributes for particular logical port.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
CMD_STATUS wrCpssPxPortAttributesOnPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result1, result2, result3,result4,result5,result6,result7;
    GT_BOOL flag1,flag2;

    GT_SW_DEV_NUM devNum = (GT_SW_DEV_NUM)inArgs[0];
    GT_PHYSICAL_PORT_NUM portNum = (GT_U32)inArgs[1];

    OUT CPSS_PORT_ATTRIBUTES_STC        portAttributSetArray;
    OUT CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    OUT CPSS_PORT_FLOW_CONTROL_ENT      flowCtrlEnable;
    OUT GT_BOOL                         speedAutoNegEnable;
    OUT GT_BOOL                         duplexAutoNegEnable;

    OUT GT_BOOL                         flowCtrlAutoNegEnable;
    OUT GT_BOOL                         pauseAdvertise;
    OUT GT_U32                          mruSize;



    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return GT_INIT_ERROR;


    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);


    /* call cpss api function */
    result2 = cpssPxPortInterfaceModeGet(devNum,portNum,&ifMode);
    if(result2 == GT_BAD_PARAM && portNum < CPSS_MAX_PORTS_NUM_CNS)
    {
        /* case that the port not valid */
        result7 = cpssPxPortMruGet(devNum,portNum,&mruSize);
        if(result7 == GT_BAD_PARAM)
        {
            if(result7 != GT_OK)
            {
                galtisOutput(outArgs, result7, "%d", -1);
                return GT_OK;
            }
        }
    }


    if((GT_OK == result2) && (CPSS_PORT_INTERFACE_MODE_HGL_E == ifMode))
    {
        CPSS_PORT_SPEED_ENT speed;
        GT_BOOL             linkUp;

        result2 = cpssPxPortSpeedGet(devNum, portNum, &speed);
        if(result2 != GT_OK)
        {
            galtisOutput(outArgs, result2, "%d", -1);
            return GT_OK;
        }

        if(speed != CPSS_PORT_SPEED_40000_E)
        {/* 40G is XLHGL supported by XLG MAC */

            result2 = cpssPxPortLinkStatusGet(devNum, portNum, &linkUp);
            if(result2 != GT_OK)
            {
                galtisOutput(outArgs, result2, "%d", -1);
                return GT_OK;
            }

            inFields[0] = linkUp;
            inFields[1] = speed;
            inFields[2] = 0;
            inFields[3] = ifMode;
            inFields[4] = 0;
            inFields[5] = 0;
            inFields[6] = 0;
            inFields[7] = 0;
            inFields[8] = 0;
            inFields[9] = 0;

            /* pack and output table fields */
            fieldOutput("%d%d%d%d%d%d%d%d%d%d",  inFields[0],  inFields[1],  inFields[2], inFields[3], inFields[4],
                                       inFields[5],  inFields[6],inFields[7],  inFields[8],inFields[9]);
            galtisOutput(outArgs, result2, "%f");
            return GT_OK;
        }
    }

    result1 = cpssPxPortAttributesOnPortGet(devNum, portNum,
                                    &portAttributSetArray);


    result3 = cpssPxPortFlowControlEnableGet(devNum,portNum,&flowCtrlEnable);
    if (portNum != CPSS_CPU_PORT_NUM_CNS)
    {
        result4 = cpssPxPortSpeedAutoNegEnableGet(devNum,portNum,&speedAutoNegEnable);
        result5 = cpssPxPortDuplexAutoNegEnableGet(devNum,portNum,&duplexAutoNegEnable);
        result6 = cpssPxPortFlowCntrlAutoNegEnableGet(devNum,portNum,
                                                            &flowCtrlAutoNegEnable,
                                                            &pauseAdvertise);
    }
    else
    {
        speedAutoNegEnable = GT_FALSE;
        result4 = GT_OK;

        duplexAutoNegEnable = GT_FALSE;
        result5 = GT_OK;

        flowCtrlAutoNegEnable = GT_FALSE;
        pauseAdvertise = GT_FALSE;
        result6 = GT_OK;
    }

    result7 = cpssPxPortMruGet(devNum,portNum,&mruSize);
    if(result1 == GT_NOT_APPLICABLE_DEVICE)
    {
        galtisOutput(outArgs, result1, "%d", -1);
        return GT_NOT_APPLICABLE_DEVICE;
    }

    flag1=(result1 != GT_OK)||(result2 != GT_OK)||(result3 != GT_OK)||(result7 != GT_OK);
    flag2=(result4 != GT_OK)||(result5 != GT_OK)||(result6 != GT_OK);

    if (flag1 || (flag2 && portNum != CPSS_CPU_PORT_NUM_CNS))
    {
        galtisOutput(outArgs, result1, "%d", -1);
        return GT_OK;
    }

    inFields[0] = portAttributSetArray.portLinkUp;
    inFields[1] = portAttributSetArray.portSpeed;
    inFields[2] = portAttributSetArray.portDuplexity;
    inFields[3] = ifMode;
    inFields[4] = flowCtrlEnable;
    inFields[5] = speedAutoNegEnable;
    inFields[6] = duplexAutoNegEnable;
    inFields[7] = flowCtrlAutoNegEnable;
    inFields[8] = pauseAdvertise;
    inFields[9] = mruSize;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d",  inFields[0],  inFields[1],  inFields[2], inFields[3], inFields[4],
                               inFields[5],  inFields[6],inFields[7],  inFields[8],inFields[9]);

    galtisOutput(outArgs, result1, "%f");

    return GT_OK;
}

/**
* @internal wrCpssPxPortCrcNumBytesSet function
* @endinternal
*
* @brief   Set CRC num of bytes.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortCrcNumBytesSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32                crcNumBytes;
    CPSS_PORT_DIRECTION_ENT portDirection;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_PORT_DIRECTION_ENT)inArgs[2];
    crcNumBytes = (GT_32)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCrcNumBytesSet(devNum, portNum, portDirection, crcNumBytes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortCrcNumBytesGet function
* @endinternal
*
* @brief   Get CRC num of bytes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssPxPortCrcNumBytesGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_SW_DEV_NUM        devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32                crcNumBytes;
    CPSS_PORT_DIRECTION_ENT portDirection;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_PORT_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortCrcNumBytesGet(devNum, portNum, portDirection, &crcNumBytes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", crcNumBytes);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSyncEtherRecoveryClkConfigSet function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on bad state
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType;
    GT_BOOL     enable;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      laneNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    recoveryClkType = (CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT)inArgs[1];
    enable  = (GT_BOOL)inArgs[2];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    laneNum = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssPxPortSyncEtherRecoveryClkConfigSet(devNum, recoveryClkType, enable, portNum, laneNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}



/**
* @internal wrCpssPxPortSyncEtherRecoveryClkConfigGet function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
*/
static CMD_STATUS wrCpssPxPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM       devNum;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType;
    GT_BOOL     enable;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      laneNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    recoveryClkType = (CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxPortSyncEtherRecoveryClkConfigGet(devNum, recoveryClkType, &enable, &portNum, &laneNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", enable, portNum, laneNum);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortSyncEtherRecoveryClkDividerValueSet function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      laneNum;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];
    clockSelect = (CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT)inArgs[3];
    value = (CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssPxPortSyncEtherRecoveryClkDividerValueSet(devNum, portNum, laneNum, clockSelect, value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxPortSyncEtherRecoveryClkDividerValueGet function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssPxPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      laneNum;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];
    clockSelect = (CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssPxPortSyncEtherRecoveryClkDividerValueGet(devNum, portNum, laneNum, clockSelect, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /* commandName                                   funcReference                  funcArgs  funcFields */
    { "cpssPxPortForceLinkPassEnableSet",
      &wrCpssPxPortForceLinkPassEnableSet,
      3, 0 },
    { "cpssPxPortForceLinkPassEnableGet",
      &wrCpssPxPortForceLinkPassEnableGet,
      2, 0 },
    { "cpssPxPortForceLinkDownEnableSet",
      &wrCpssPxPortForceLinkDownEnableSet,
      3, 0 },
    { "cpssPxPortForceLinkDownEnableGet",
      &wrCpssPxPortForceLinkDownEnableGet,
      2, 0 },
    { "cpssPxPortMruSet",
      &wrCpssPxPortMruSet,
      3, 0 },
    { "cpssPxPortMruGet",
      &wrCpssPxPortMruGet,
      2, 0 },
    { "cpssPxPortLinkStatusGet",
      &wrCpssPxPortLinkStatusGet,
      2, 0 },
    { "cpssPxPortInternalLoopbackEnableSet",
      &wrCpssPxPortInternalLoopbackEnableSet,
      3, 0 },
    { "cpssPxPortInternalLoopbackEnableGet",
      &wrCpssPxPortInternalLoopbackEnableGet,
      2, 0 },
    { "cpssPxPortModeSpeedSet",
      &wrCpssPxPortModeSpeedSet,
      5, 0 },
    { "cpssPxPortEnableSet",
      &wrCpssPxPortEnableSet,
      3, 0 },
    { "cpssPxPortEnableGet",
      &wrCpssPxPortEnableGet,
      2, 0 },
    { "cpssPxPortSerdesAutoTune",
      &wrCpssPxPortSerdesAutoTune,
      3, 0},
    { "cpssPxPortSerdesAutoTuneExt",
      &wrCpssPxPortSerdesAutoTuneExt,
      4, 0},
    { "cpssPxPortSerdesAutoTuneOptAlgGet",
      &wrCpssPxPortSerdesAutoTuneOptAlgGet,
      2, 0},
    { "cpssPxPortSerdesAutoTuneOptAlgSet",
      &wrCpssPxPortSerdesAutoTuneOptAlgSet,
      3, 0},
    { "cpssPxPortSerdesAutoTuneStatusGet",
      &wrCpssPxPortSerdesAutoTuneStatusGet,
      2, 0},
    { "cpssPxPortSerdesLanePolarityGet",
      &wrCpssPxPortSerdesLanePolarityGet,
      3, 0},
    { "cpssPxPortSerdesLanePolaritySet",
      &wrCpssPxPortSerdesLanePolaritySet,
      5, 0},
    { "cpssPxPortSerdesLaneTuningEntryGetFirst",
      &wrCpssPxPortSerdesLaneTuningGet,
      4, 0},
        { "cpssPxPortSerdesLaneTuningEntryGetNext",
      &wrCpssPxGetNext,
      4, 0},
    { "cpssPxPortSerdesLaneTuningEntrySet",
      &wrCpssPxPortSerdesLaneTuningSet,
      4, 9},
    { "cpssPxPortSerdesLoopbackModeGet",
      &wrCpssPxPortSerdesLoopbackModeGet,
      3, 0},
    { "cpssPxPortSerdesLoopbackModeSet",
      &wrCpssPxPortSerdesLoopbackModeSet,
      4, 0},
    { "cpssPxPortSerdesManualRxConfigGet",
      &wrCpssPxPortSerdesManualRxConfigGet,
      3, 0},
    { "cpssPxPortSerdesManualRxConfigSet",
      &wrCpssPxPortSerdesManualRxConfigSet,
      9, 0},
    { "cpssPxPortSerdesManualTxConfigGet",
      &wrCpssPxPortSerdesManualTxConfigGet,
      3, 0},
    { "cpssPxPortSerdesManualTxConfigSet",
      &wrCpssPxPortSerdesManualTxConfigSet,
      6, 0},
    { "cpssPxPortSerdesPolarityGet",
      &wrCpssPxPortSerdesPolarityGet,
      3, 0},
    { "cpssPxPortSerdesPolaritySet",
      &wrCpssPxPortSerdesPolaritySet,
      5, 0},
    { "cpssPxPortSerdesPowerStatusSet",
      &wrCpssPxPortSerdesPowerStatusSet,
      3, 0},
    { "cpssPxPortSerdesResetStateSet",
       &wrCpssPxPortSerdesResetStateSet,
       3, 0},
    { "cpssPxPortSerdesTuningEntryGetFirst",
      &wrCpssPxPortSerdesTuningGet,
      4, 0},
    { "cpssPxPortSerdesTuningEntryGetNext",
      &wrCpssPxGetNext,
      4, 0},
    { "cpssPxPortSerdesTuningEntrySet",
      &wrCpssPxPortSerdesTuningSet,
      4, 9},
    { "cpssPxPortSerdesTxEnableGet",
      &wrCpssPxPortSerdesTxEnableGet,
      2, 0},
    { "cpssPxPortSerdesTxEnableSet",
       &wrCpssPxPortSerdesTxEnableSet,
       3, 0},
    { "cpssPxPortMacResetStateSet",
      &wrCpssPxPortMacResetStateSet,
      3, 0},
    { "cpssPxPortInBandAutoNegBypassEnableSet",
      &wrCpssPxPortInBandAutoNegBypassEnableSet,
      3, 0},
    { "cpssPxPortInBandAutoNegBypassEnableGet",
      &wrCpssPxPortInBandAutoNegBypassEnableGet,
      2, 0},
    { "cpssPxPortExcessiveCollisionDropEnableSet",
      &wrCpssPxPortExcessiveCollisionDropEnableSet,
      3, 0},
    { "cpssPxPortExcessiveCollisionDropEnableGet",
      &wrCpssPxPortExcessiveCollisionDropEnableGet,
      2, 0},
    { "cpssPxPortCrcCheckEnableSet",
      &wrCpssPxPortCrcCheckEnableSet,
      3, 0},
    { "cpssPxPortCrcCheckEnableGet",
      &wrCpssPxPortCrcCheckEnableGet,
      2, 0},
    { "cpssPxPortDuplexAutoNegEnableSet",
      &wrCpssPxPortDuplexAutoNegEnableSet,
      3, 0},
    { "cpssPxPortDuplexAutoNegEnableGet",
      &wrCpssPxPortDuplexAutoNegEnableGet,
      2, 0},
    { "cpssPxPortFlowCntrlAutoNegEnableSet",
      &wrCpssPxPortFlowCntrlAutoNegEnableSet,
      4, 0},
    { "cpssPxPortFlowCntrlAutoNegEnableGet",
      &wrCpssPxPortFlowCntrlAutoNegEnableGet,
      2, 0},
    { "cpssPxPortFlowControlEnableGet",
      &wrCpssPxPortFlowControlEnableGet,
      2, 0},
    { "cpssPxPortFlowControlEnableSet",
      &wrCpssPxPortFlowControlEnableSet,
      3, 0},
    { "cpssPxPortBackPressureEnableSet",
      &wrCpssPxPortBackPressureEnableSet,
      3, 0},
    { "cpssPxPortBackPressureEnableGet",
      &wrCpssPxPortBackPressureEnableGet,
      2, 0},
    { "cpssPxPortSpeedAutoNegEnableSet",
      &wrCpssPxPortSpeedAutoNegEnableSet,
      3, 0},
    { "cpssPxPortSpeedAutoNegEnableGet",
      &wrCpssPxPortSpeedAutoNegEnableGet,
      2, 0},
    { "cpssPxPortDuplexModeSet",
      &wrCpssPxPortDuplexModeSet,
      3, 0},
    { "cpssPxPortDuplexModeGet",
      &wrCpssPxPortDuplexModeGet,
      2, 0},
    { "cpssPxPortFlowControlModeSet",
      &wrCpssPxPortFlowControlModeSet,
      3, 0},
    { "cpssPxPortFlowControlModeGet",
      &wrCpssPxPortFlowControlModeGet,
      2, 0},
    { "cpssPxPortAttributesGetFirst",
      &wrCpssPxPortAttributesGetFirst,
      0, 0},
    { "cpssPxPortAttributesGetNext",
     &wrCpssPxPortAttributesGetNext,
     0, 0},
    { "cpssPxPortForward802_3xEnableSet",
      &wrCpssPxPortForward802_3xEnableSet,
      3, 0},
    { "cpssPxPortForward802_3xEnableGet",
      &wrCpssPxPortForward802_3xEnableGet,
      2, 0},
    { "cpssPxPortForwardUnknownMacControlFramesEnSet",
      &wrCpssPxPortForwardUnknownMacControlFramesEnableSet,
      3, 0},
    { "cpssPxPortForwardUnknownMacControlFramesEnGet",
      &wrCpssPxPortForwardUnknownMacControlFramesEnableGet,
      2, 0},
    { "cpssPxPortIpgBaseSet",
      &wrCpssPxPortIpgBaseSet,
      3, 0},
    { "cpssPxortIpgBaseGet",
      &wrCpssPxPortIpgBaseGet,
      2, 0},
    { "cpssPxPortIpgSet",
      &wrCpssPxPortIpgSet,
      3, 0},
    { "cpssPxPortIpgGet",
      &wrCpssPxPortIpgGet,
      2, 0},
    { "cpssPxPortInbandAutoNegEnableSet",
      &wrCpssPxPortInbandAutoNegEnableSet,
      3, 0},
    { "cpssPxPortInbandAutoNegEnableGet",
      &wrCpssPxPortInbandAutoNegEnableGet,
      2, 0},
    { "cpssPxPortPeriodicFcEnableSet",
      &wrCpssPxPortPeriodicFcEnableSet,
      3, 0},
    { "cpssPxPortPeriodicFcEnableGet",
      &wrCpssPxPortPeriodicFcEnableGet,
      2, 0},
    { "cpssPxPortPreambleLengthSet",
      &wrCpssPxPortPreambleLengthSet,
      4, 0},
    { "cpssPxPortPreambleLengthGet",
      &wrCpssPxPortPreambleLengthGet,
      3, 0},
    { "cpssPxPortSerdesEnhancedAutoTune",
       &wrCpssPxPortSerdesEnhancedAutoTuneSet,
       4, 0},
    { "cpssPxPortFecModeSet",
      &wrCpssPxPortFecModeSet,
      3, 0},
    { "cpssPxPortFecModeGet",
      &wrCpssPxPortFecModeGet,
      2, 0},
    { "cpssPxPortPeriodicFlowControlCounterSet",
        &wrCpssPxPortPeriodicFlowControlCounterSet,
        3, 0},
    { "cpssPxPortPeriodicFlowControlCounterGet",
        &wrCpssPxPortPeriodicFlowControlCounterGet,
        2, 0},
    {"cpssPxPortMacSaLsbSet",
        &wrCpssPxPortMacSaLsbSet,
        3, 0},
    {"cpssPxPortMacSaLsbGet",
        &wrCpssPxPortMacSaLsbGet,
        2, 0},
    {"cpssPxPortMacSaBaseSet",
        &wrCpssPxPortMacSaBaseSet,
        2, 0},
    {"cpssPxPortMacSaBaseGet",
        &wrCpssPxPortMacSaBaseGet,
        1, 0},
    {"cpssPxPortXGmiiModeSet",
        &wrCpssPxPortXGmiiModeSet,
        3, 0},
    {"cpssPxPortXGmiiModeGet",
         &wrCpssPxPortXGmiiModeGet,
         2, 0},
    {"cpssPxPortExtraIpgSet",
        &wrCpssPxPortExtraIpgSet,
        3, 0},
    {"cpssPxPortExtraIpgGet",
         &wrCpssPxPortExtraIpgGet,
         2, 0},
    {"cpssPxPortXgmiiLocalFaultGet",
        &wrCpssPxPortXgmiiLocalFaultGet,
        2, 0},
    {"cpssPxPortXgmiiRemoteFaultGet",
        &wrCpssPxPortXgmiiRemoteFaultGet,
        2, 0},
    {"cpssPxPortMacStatusGetFirst",
        &wrCpssPxPortMacStatusGetFirst,
        1, 0},
    {"cpssPxPortMacStatusGetNext",
        &wrCpssPxPortMacStatusGetNext,
        1, 0},
    {"cpssPxPortPaddingEnableSet",
         &wrCpssPxPortPaddingEnableSet,
         3, 0},
    {"cpssPxPortPaddingEnableGet",
         &wrCpssPxPortPaddingEnableGet,
         2, 0},
    {"cpssPxPortPcsLoopbackModeSet",
        &wrCpssPxPortPcsLoopbackModeSet,
        3, 0},
    {"cpssPxPortPcsLoopbackModeGet",
        &wrCpssPxPortPcsLoopbackModeGet,
        2, 0},
    {"cpssPxPortXgLanesSwapEnableSet",
         &wrCpssPxPortXgLanesSwapEnableSet,
         3, 0},
    {"cpssPxPortXgLanesSwapEnableGet",
         &wrCpssPxPortXgLanesSwapEnableGet,
         2, 0},
    {"cpssPxPortXgPscLanesSwapSet",
         &wrCpssPxPortXgPscLanesSwapSet,
         10, 0},
    {"cpssPxPortXgPscLanesSwapGet",
         &wrCpssPxPortXgPscLanesSwapGet,
         2, 0},
    { "cpssPxPortAutoNegAdvertismentConfigGet",
         &wrCpssPxPortAutoNegAdvertismentConfigGet,
         2, 0},
    { "cpssPxPortAutoNegAdvertismentConfigSet",
         &wrCpssPxPortAutoNegAdvertismentConfigSet,
         5, 0},
    { "cpssPxPortAutoNegMasterModeEnableGet",
         &wrCpssPxPortAutoNegMasterModeEnableGet,
         2, 0},
    { "cpssPxPortAutoNegMasterModeEnableSet",
         &wrCpssPxPortAutoNegMasterModeEnableSet,
         3, 0},
    { "cpssPxPortRefClockSourceOverrideEnableGet",
         &wrCpssPxPortRefClockSourceOverrideEnableGet,
         2, 0},
    { "cpssPxPortRefClockSourceOverrideEnableSet",
         &wrCpssPxPortRefClockSourceOverrideEnableSet,
         4, 0},
    { "cpssPxPortAttributesOnPortGetFirst",
        &wrCpssPxPortAttributesOnPortGet,
        2, 0},
        { "cpssPxPortAttributesOnPortGetNext",
        &wrCpssPxGetNext,
        2, 0},
    { "cpssPxPortCrcNumBytesSet",
         &wrCpssPxPortCrcNumBytesSet,
         4, 0},
    { "cpssPxPortCrcNumBytesGet",
        &wrCpssPxPortCrcNumBytesGet,
        3, 0},
    {"cpssPxPortSyncEtherRecoveryClkConfigSet",
        &wrCpssPxPortSyncEtherRecoveryClkConfigSet,
        5, 0},
    {"cpssPxPortSyncEtherRecoveryClkConfigGet",
        &wrCpssPxPortSyncEtherRecoveryClkConfigGet,
        2, 0},
    {"cpssPxPortSyncEtherRecoveryClkDividerValueSet",
        &wrCpssPxPortSyncEtherRecoveryClkDividerValueSet,
        5, 0},
    {"cpssPxPortSyncEtherRecoveryClkDividerValueGet",
        &wrCpssPxPortSyncEtherRecoveryClkDividerValueGet,
        4, 0},
    {"cpssPxPortPcsResetSet",
        &wrCpssPxPortPcsResetSet,
        4, 0}
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortCtrl function
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
GT_STATUS cmdLibInitCpssPxPortCtrl
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


