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
* @file wrapCpssPxPortStat.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortStat.h API's
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
#include <cpss/px/port/cpssPxPortStat.h>

/**
* @internal util_wrCpssPxPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets ethernet MAC counters on current port and print them.
*
* @note   APPLICABLE DEVICES:      All Pipe devices
* @param[in] devNum                   - physical device number
* @param[in,out] portNumPtr               - (pointer to) physical port number
*
* @param[out] inFields[CMD_MAX_FIELDS] - memory of in fields used as temporary
* @param[out] outArgs[CMD_MAX_BUFFER]  - output memory
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS  util_wrCpssPxPortMacCountersOnPortGet
(
    IN     GT_SW_DEV_NUM            devNum,
    INOUT  GT_PHYSICAL_PORT_NUM     *portNumPtr,
    OUT    GT_UINTPTR   inFields[CMD_MAX_FIELDS],
    OUT    GT_8         outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    CPSS_PX_PORT_MAC_COUNTER_ENT    macCounterIndex;
    CPSS_PX_PORT_MAC_COUNTERS_STC   portMacCounterSetArray;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          actualNumFields;
    GT_U32                          i;

    while (1)
    {
        portNum = *portNumPtr;

        /* Override Device and Port */
        CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

        if (portNum >= PRV_CPSS_PX_GOP_PORTS_NUM_CNS)
        {
            /* All ports already treated - end of table */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return GT_OK;
        }

        /* call cpss api function */
        result = cpssPxPortMacCountersOnPortGet(
            devNum, portNum, &portMacCounterSetArray);
        if (result == GT_OK)
        {
            /* success, increment port number for the next calls */
            (*portNumPtr)++;
            break;
        }

        /* This port failed, try the next port */
        (*portNumPtr)++;
    }

    /* Starts from first MAC counter in enum */
    macCounterIndex = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;

    /* Starts from field[0]*/
    i = 0;
    /* Port number */
    inFields[i++] = portNum;

    while (macCounterIndex != CPSS_PX_PORT_MAC_COUNTER____LAST____E)
    {
        /* Low word */
        inFields[i++] = portMacCounterSetArray.mibCounter[macCounterIndex].l[0];
        /* High word */
        inFields[i++] = portMacCounterSetArray.mibCounter[macCounterIndex].l[1];
        /* Increment enum */
        macCounterIndex++;
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

/* port used in Mac Counters tables */
static GT_PHYSICAL_PORT_NUM portForMacCountersGet;
/**
* @internal wrCpssPxPortMacCountersTableGetFirst function
* @endinternal
*
* @brief   Gets ethernet MAC counters for the first port.
*
* @note   APPLICABLE DEVICES:      All Pipe devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS  wrCpssPxPortMacCountersTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portForMacCountersGet = 0;

    util_wrCpssPxPortMacCountersOnPortGet(
        devNum, &portForMacCountersGet,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersTableGetNext function
* @endinternal
*
* @brief   Gets ethernet MAC counters for the current port.
*
* @note   APPLICABLE DEVICES:      All Pipe devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS  wrCpssPxPortMacCountersTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    util_wrCpssPxPortMacCountersOnPortGet(
        devNum, &portForMacCountersGet,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCounterGet function
* @endinternal
*
* @brief   Gets specific MAC MIB counter of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacCounterGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PX_PORT_MAC_COUNTER_ENT    cntrName;
    GT_U64                          cntrValuePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum   = (GT_SW_DEV_NUM) inArgs[0];
    portNum  = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    cntrName = (CPSS_PX_PORT_MAC_COUNTER_ENT) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCounterGet(devNum, portNum, cntrName, &cntrValuePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%l", cntrValuePtr);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets ALL MAC MIB counters of a specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacCountersOnPortGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          portNum;
    CPSS_PX_PORT_MAC_COUNTER_ENT    macCounterIndex;
    CPSS_PX_PORT_MAC_COUNTERS_STC   portMacCounterSetArray;
    GT_U32                          i;
    GT_U32                          actualNumFields;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_U32)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersOnPortGet(devNum, portNum, &portMacCounterSetArray);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* Starts from field[0 ]*/
    i = 0;
    /* Starts from first MAC counter in enum */
    macCounterIndex = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;

    while (macCounterIndex != CPSS_PX_PORT_MAC_COUNTER____LAST____E)
    {
        /* Low word */
        inFields[i++] = portMacCounterSetArray.mibCounter[macCounterIndex].l[0];
        /* High word */
        inFields[i++] = portMacCounterSetArray.mibCounter[macCounterIndex].l[1];
        /* Increment enum */
        macCounterIndex++;
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

/**
* @internal wrCpssPxPortMacOversizedPacketsCounterModeSet function
* @endinternal
*
* @brief   Sets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacOversizedPacketsCounterModeSet
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
    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    counterMode = (CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT) inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacOversizedPacketsCounterModeSet(devNum, portNum, counterMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacOversizedPacketsCounterModeGet function
* @endinternal
*
* @brief   Gets MIB counters behavior for oversized packets on given port,
*         that relate to counters:
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E
*         CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacOversizedPacketsCounterModeGet
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
    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT counterMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacOversizedPacketsCounterModeGet(devNum, portNum, &counterMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counterMode);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters 'Clear on read' on the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacCountersClearOnReadSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    enable  = (GT_BOOL) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersClearOnReadSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get 'Clear on read' status of MAC Counters on the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacCountersClearOnReadGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersClearOnReadGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersEnableSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters update for this port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacCountersEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    enable  = (GT_BOOL) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disable) of MAC Counters update
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical
*                                       port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
CMD_STATUS wrCpssPxPortMacCountersEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCounterCaptureGet function
* @endinternal
*
* @brief   Gets the captured Ethernet MAC counter for a specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacCounterCaptureGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PX_PORT_MAC_COUNTER_ENT    cntrName;
    GT_U64                          cntrValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum   = (GT_SW_DEV_NUM) inArgs[0];
    portNum  = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    cntrName = (CPSS_PX_PORT_MAC_COUNTER_ENT) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCounterCaptureGet(devNum, portNum, cntrName, &cntrValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%l", cntrValue);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
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
CMD_STATUS wrCpssPxPortMacCountersCaptureOnPortGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          portNum;
    CPSS_PX_PORT_MAC_COUNTERS_STC   portMacCounterSetArray;
    CPSS_PX_PORT_MAC_COUNTER_ENT    macCounterIndex;
    GT_U32                          i;
    GT_U32                          actualNumFields;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_U32)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersCaptureOnPortGet(devNum, portNum, &portMacCounterSetArray);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }


    /* Starts from field[0 ]*/
    i = 0;
    /* Starts from first MAC counter in enum */
    macCounterIndex = CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E;

    while (macCounterIndex != CPSS_PX_PORT_MAC_COUNTER____LAST____E)
    {
        /* Low word */
        inFields[i++] = portMacCounterSetArray.mibCounter[macCounterIndex].l[0];
        /* High word */
        inFields[i++] = portMacCounterSetArray.mibCounter[macCounterIndex].l[1];
        /* Increment enum */
        macCounterIndex++;
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

/**
* @internal wrCpssPxPortMacCountersCaptureTriggerSet function
* @endinternal
*
* @brief   The function triggers a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
CMD_STATUS wrCpssPxPortMacCountersCaptureTriggerSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersCaptureTriggerSet(devNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersCaptureTriggerGet function
* @endinternal
*
* @brief   The function gets status of a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the counters, see:
*       cpssPxPortMacCountersCaptureOnPortGet,
*       cpssPxPortMacCounterCaptureGet.
*
*/
CMD_STATUS wrCpssPxPortMacCountersCaptureTriggerGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               captureIsDone;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersCaptureTriggerGet(devNum, portNum, &captureIsDone);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", captureIsDone);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersOnCgPortGet function
* @endinternal
*
* @brief   Gets the CG mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
CMD_STATUS wrCpssPxPortMacCountersOnCgPortGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_SW_DEV_NUM                       devNum;
    GT_U32                              portNum;
    CPSS_PORT_MAC_CG_COUNTER_SET_STC    cgMibStc;
    GT_U32                              i;
    GT_U32                              macCounterIndex;
    GT_U32                              actualNumFields;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_U32)inArgs[1];


    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersOnCgPortGet(devNum, portNum, &cgMibStc);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    /* Starts from field[0 ]*/
    i = 0;
    /*
        Convert struct with 50+ GT_U64 counters to (GT_U64*)
     */
    for (macCounterIndex = 0;
         macCounterIndex < (sizeof(CPSS_PORT_MAC_CG_COUNTER_SET_STC) / sizeof(GT_U64));
         macCounterIndex++)
    {
        /* Low word */
        inFields[i++] = ((GT_U64*) (&cgMibStc))[macCounterIndex].l[0];
        /* High word */
        inFields[i++] = ((GT_U64*) (&cgMibStc))[macCounterIndex].l[1];
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

/**
* @internal wrCpssPxPortStatTxDebugCountersGet function
* @endinternal
*
* @brief   Gets egress TxQ drop counters.
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
CMD_STATUS wrCpssPxPortStatTxDebugCountersGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;
    CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC  dropCntrStc;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];

    /* call cpss api function */
    result = cpssPxPortStatTxDebugCountersGet(devNum, &dropCntrStc);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput("%d%d%d%d%d",
                dropCntrStc.egrMirrorDropCntr,
                dropCntrStc.egrStcDropCntr,
                dropCntrStc.egrQcnDropCntr,
                dropCntrStc.dropPfcEventsCntr,
                dropCntrStc.clearPacketsDroppedCounter);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified egress
*         counters mode on specified device.
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
*
* @note Supported DP levels:
*       CPSS_DP_GREEN_E,
*       CPSS_DP_RED_E.
*
*/
CMD_STATUS wrCpssPxPortEgressCntrModeSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          cntrSetNum;
    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          tc;
    CPSS_DP_LEVEL_ENT               dpLevel;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM) inArgs[0];
    cntrSetNum = (GT_U32) inArgs[1];
    setModeBmp = (CPSS_PORT_EGRESS_CNT_MODE_ENT) inArgs[2];
    portNum    = (GT_PHYSICAL_PORT_NUM) inArgs[3];
    tc         = (GT_U32) inArgs[4];
    dpLevel    = (CPSS_DP_LEVEL_ENT) inArgs[5];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortEgressCntrModeSet(devNum, cntrSetNum, setModeBmp,
                                         portNum, tc, dpLevel);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of egress counters set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortEgressCntrModeGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          cntrSetNum;
    CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          tc;
    CPSS_DP_LEVEL_ENT               dpLevel;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM) inArgs[0];
    cntrSetNum = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortEgressCntrModeGet(devNum, cntrSetNum, &setModeBmp,
                                         &portNum, &tc, &dpLevel);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", setModeBmp, portNum, tc, dpLevel);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
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
CMD_STATUS wrCpssPxPortEgressCntrsGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          cntrSetNum;
    CPSS_PX_PORT_EGRESS_CNTR_STC    egrCntr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM) inArgs[0];
    cntrSetNum = (GT_U32) inArgs[1];

    /* call cpss api function */
    result = cpssPxPortEgressCntrsGet(devNum, cntrSetNum, &egrCntr);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput("%d%d", egrCntr.outFrames, egrCntr.txqFilterDisc);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersRxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device or physical port number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
CMD_STATUS wrCpssPxPortMacCountersRxHistogramEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    enable  = (GT_BOOL) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersRxHistogramEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersTxHistogramEnableSet function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram counters for
*         transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortMacCountersTxHistogramEnableSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    enable  = (GT_BOOL) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersTxHistogramEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersRxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for received packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
CMD_STATUS wrCpssPxPortMacCountersRxHistogramEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersRxHistogramEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortMacCountersTxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram counters
*         for transmitted packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
CMD_STATUS wrCpssPxPortMacCountersTxHistogramEnableGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_SW_DEV_NUM         devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM) inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortMacCountersTxHistogramEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/* wrapper for cpssPxPortMacCountersOnPortGet */
CMD_STATUS wrCpssPxPortMacCountersOnPortGetFirst
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

    return wrCpssPxPortMacCountersOnPortGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxPortMacCountersCaptureOnPortGet */
CMD_STATUS wrCpssPxPortMacCountersCaptureOnPortGetFirst
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

    return wrCpssPxPortMacCountersCaptureOnPortGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxPortMacCountersCaptureOnPortGet */
CMD_STATUS wrCpssPxPortMacCountersCaptureOnPortGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxPortMacCountersCaptureOnPortGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxPortMacCountersOnCgPortGet */
CMD_STATUS wrCpssPxPortMacCountersOnCgPortGetFirst
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

    return wrCpssPxPortMacCountersOnCgPortGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxPortMacCountersOnCgPortGet */
CMD_STATUS wrCpssPxPortMacCountersOnCgPortGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxPortMacCountersOnCgPortGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssPxGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      Pipe
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

/* wrapper for cpssPxPortStatTxDebugCountersGet */
CMD_STATUS wrCpssPxPortStatTxDebugCountersGetFirst
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

    return wrCpssPxPortStatTxDebugCountersGet(inArgs, inFields, numFields, outArgs);
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /* commandName                                   funcReference                  funcArgs  funcFields */
    { "cpssPxPortMacCounterGet",                     &wrCpssPxPortMacCounterGet,                     3, 0},
    { "cpssPxPortMacCountersGetFirst",
        &wrCpssPxPortMacCountersTableGetFirst,
        1, 0},
    { "cpssPxPortMacCountersGetNext",
        &wrCpssPxPortMacCountersTableGetNext,
        1, 0},
    { "cpssPxPortMacCountersOnPortGetFirst",
       &wrCpssPxPortMacCountersOnPortGetFirst,
       2, 0 },
    { "cpssPxPortMacCountersOnPortGetNext",
       &wrCpssPxGetNext,
       2, 0 },
    { "cpssPxPortMacOversizedPacketsCounterModeSet", &wrCpssPxPortMacOversizedPacketsCounterModeSet, 3, 0},
    { "cpssPxPortMacOversizedPacketsCounterModeGet", &wrCpssPxPortMacOversizedPacketsCounterModeGet, 3, 0},
    { "cpssPxPortMacCountersClearOnReadSet",         &wrCpssPxPortMacCountersClearOnReadSet,         3, 0},
    { "cpssPxPortMacCountersClearOnReadGet",         &wrCpssPxPortMacCountersClearOnReadGet,         3, 0},
    { "cpssPxPortMacCountersEnableSet",              &wrCpssPxPortMacCountersEnableSet,              3, 0},
    { "cpssPxPortMacCountersEnableGet",              &wrCpssPxPortMacCountersEnableGet,              3, 0},
    { "cpssPxPortMacCounterCaptureGet",              &wrCpssPxPortMacCounterCaptureGet,              4, 0},
    { "cpssPxPortMacCountersCaptureOnPortGetFirst",
       &wrCpssPxPortMacCountersCaptureOnPortGetFirst,
       2, 0},
    { "cpssPxPortMacCountersCaptureOnPortGetNext",
       &wrCpssPxGetNext,
       2, 0 },
    { "cpssPxPortMacCountersCaptureTriggerSet",      &wrCpssPxPortMacCountersCaptureTriggerSet,      2, 0},
    { "cpssPxPortMacCountersCaptureTriggerGet",      &wrCpssPxPortMacCountersCaptureTriggerGet,      3, 0},
    { "cpssPxPortMacCountersOnCgPortGetFirst",
       &wrCpssPxPortMacCountersOnCgPortGetFirst,
       2, 0},
    { "cpssPxPortMacCountersOnCgPortGetNext",
       &wrCpssPxGetNext,
       2, 0 },
    { "cpssPxPortStatTxDebugCountersGetFirst",
       &wrCpssPxPortStatTxDebugCountersGetFirst,
       1, 0},
    { "cpssPxPortStatTxDebugCountersGetNext",
       &wrCpssPxGetNext,
       1, 0},
    { "cpssPxPortEgressCntrModeSet",                 &wrCpssPxPortEgressCntrModeSet,                 6, 0},
    { "cpssPxPortEgressCntrModeGet",                 &wrCpssPxPortEgressCntrModeGet,                 2, 0},
    { "cpssPxPortEgressCntrsGetFirst",
       &wrCpssPxPortEgressCntrsGet,
       2, 0},
    {"cpssPxPortEgressCntrsGetNext",
       &wrCpssPxGetNext,
       2, 0},
    { "cpssPxPortMacCountersRxHistogramEnableSet",   &wrCpssPxPortMacCountersRxHistogramEnableSet,   3, 0},
    { "cpssPxPortMacCountersTxHistogramEnableSet",   &wrCpssPxPortMacCountersTxHistogramEnableSet,   3, 0},
    { "cpssPxPortMacCountersRxHistogramEnableGet",   &wrCpssPxPortMacCountersRxHistogramEnableGet,   3, 0},
    { "cpssPxPortMacCountersTxHistogramEnableGet",   &wrCpssPxPortMacCountersTxHistogramEnableGet,   3, 0},
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortStat function
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
GT_STATUS cmdLibInitCpssPxPortStat
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

