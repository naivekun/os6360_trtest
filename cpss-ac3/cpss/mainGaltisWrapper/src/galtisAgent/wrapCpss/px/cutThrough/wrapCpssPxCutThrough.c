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
* @file wrapCpssPxCutThrough.c
*
* @brief Wrapper functions for
* cpss/Px/cutThrough/wrapCpssPxCutThrough.h API's
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
#include <cpss/px/cutThrough/cpssPxCutThrough.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


/**
* @internal wrCpssPxCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssPxCutThroughUpEnableSet.
*
*/
CMD_STATUS wrCpssPxCutThroughPortEnableSet
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
    GT_BOOL                          untaggedEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable          = (GT_BOOL)inArgs[2];
    untaggedEnable  = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCutThroughPortEnableSet(
        devNum, portNum, enable, untaggedEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCutThroughPortEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode on the specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxCutThroughPortEnableGet
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
    GT_BOOL                          untaggedEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable          = GT_FALSE;
    untaggedEnable  = GT_FALSE;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCutThroughPortEnableGet(
        devNum, portNum, &enable, &untaggedEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, untaggedEnable);

    return CMD_OK;
}

/**
* @internal wrCpssPxCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssPxCutThroughPortEnableSet for it.
*
*/
CMD_STATUS wrCpssPxCutThroughUpEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_U32                           up;
    GT_BOOL                          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    up              = (GT_U32)inArgs[1];
    enable          = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxCutThroughUpEnableSet(
        devNum, up, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCutThroughUpEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode for tagged packets
*         with the specified UP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or up
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxCutThroughUpEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_U32                           up;
    GT_BOOL                          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    up              = (GT_U32)inArgs[1];
    enable          = GT_FALSE;

    /* call cpss api function */
    result = cpssPxCutThroughUpEnableGet(
        devNum, up, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssPxCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxCutThroughVlanEthertypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_U32                           etherType0;
    GT_U32                           etherType1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    etherType0      = (GT_U32)inArgs[1];
    etherType1      = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssPxCutThroughVlanEthertypeSet(
        devNum, etherType0, etherType1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCutThroughVlanEthertypeGet function
* @endinternal
*
* @brief   Get VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxCutThroughVlanEthertypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_SW_DEV_NUM                    devNum;
    GT_U32                           etherType0;
    GT_U32                           etherType1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    etherType0      = 0;
    etherType1      = 0;

    /* call cpss api function */
    result = cpssPxCutThroughVlanEthertypeGet(
        devNum, &etherType0, &etherType1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", etherType0, etherType1);

    return CMD_OK;
}

/**
* @internal wrCpssPxCutThroughMaxBuffersLimitSet function
* @endinternal
*
* @brief   Set max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong maxSharedBuffersLimit
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxCutThroughMaxBuffersLimitSet
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
    GT_U32                           buffersLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    buffersLimit    = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCutThroughMaxBuffersLimitSet(
        devNum, portNum, buffersLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxCutThroughMaxBuffersLimitGet function
* @endinternal
*
* @brief   Get max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxCutThroughMaxBuffersLimitGet
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
    GT_U32                           buffersLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    buffersLimit    = 0;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCutThroughMaxBuffersLimitGet(
        devNum, portNum, &buffersLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", buffersLimit);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName,                                    funcReference                            funcArgs funcFields */
    { "cpssPxCutThroughPortEnableSet",                 &wrCpssPxCutThroughPortEnableSet,        4, 0},
    { "cpssPxCutThroughPortEnableGet",                 &wrCpssPxCutThroughPortEnableGet,        2, 0},
    { "cpssPxCutThroughUpEnableSet",                   &wrCpssPxCutThroughUpEnableSet,          3, 0},
    { "cpssPxCutThroughUpEnableGet",                   &wrCpssPxCutThroughUpEnableGet,          2, 0},
    { "cpssPxCutThroughVlanEthertypeSet",              &wrCpssPxCutThroughVlanEthertypeSet,     3, 0},
    { "cpssPxCutThroughVlanEthertypeGet",              &wrCpssPxCutThroughVlanEthertypeGet,     1, 0},
    { "cpssPxCutThroughMaxBuffersLimitSet",            &wrCpssPxCutThroughMaxBuffersLimitSet,   3, 0},
    { "cpssPxCutThroughMaxBuffersLimitGet",            &wrCpssPxCutThroughMaxBuffersLimitGet,   2, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxCutThrough function
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
GT_STATUS cmdLibInitCpssPxCutThrough
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



