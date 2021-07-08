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
* @file wrapCpssDxChVnt.c
*
* @brief TODO: Add proper description of this file here
*
* @version   6
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>

/**
* @internal wrCpssDxChVntOamPortLoopBackModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable 802.3ah Loopback mode on the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         Loopback Mode is enabled for the port,
*         - All traffic destined for this port but was not ingressed on this port
*         is discarded, except for FROM_CPU traffic - which is to allow the CPU
*         to send OAM control packets.
*         - Egress VLAN and spanning tree filtering is bypassed.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*
* @note In the Policy TCAM, after the existing rule which traps OAMPDUs,
*       add a rule to redirect all traffic received on
*       the port back to the same port for getting loopback.
*
*/
static CMD_STATUS wrCpssDxChVntOamPortLoopBackModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChVntOamPortLoopBackModeEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntOamPortLoopBackModeEnableGet function
* @endinternal
*
* @brief   Get 802.3ah Loopback mode on the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         Loopback Mode is enabled for the port,
*         - All traffic destined for this port but was not ingressed on this port
*         is discarded, except for FROM_CPU traffic - which is to allow the CPU
*         to send OAM control packets.
*         - Egress VLAN and spanning tree filtering is bypassed.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note In the Policy TCAM, after the existing rule which traps OAMPDUs,
*       add a rule to redirect all traffic received on
*       the port back to the same port for getting loopback.
*
*/
static CMD_STATUS wrCpssDxChVntOamPortLoopBackModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChVntOamPortLoopBackModeEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntOamPortUnidirectionalEnableSet function
* @endinternal
*
* @brief   Enable/Disable the port for unidirectional transmit.
*         If unidirectional transmit enabled, sending
*         OAM (Operation, Administration, and Maintenance) control packets
*         and data traffic over failed links (ports with link down) is allowed.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
static CMD_STATUS wrCpssDxChVntOamPortUnidirectionalEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChVntOamPortUnidirectionalEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntOamPortUnidirectionalEnableGet function
* @endinternal
*
* @brief   Gets the current status of unidirectional transmit for the port.
*         If unidirectional transmit enabled, sending
*         OAM (Operation, Administration, and Maintenance) control packets
*         and data traffic over failed links (ports with link down) is allowed.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChVntOamPortUnidirectionalEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChVntOamPortUnidirectionalEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntCfmEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CFM (Connectivity Fault Management) PDUs.
*         If the packet EtherType matches the CFM EtherType, the ingress Policy
*         key implicitly uses the three User-Defined-Byte (UDB0, UDB1, UDB2) to
*         contain the three CFM data fields, MD Level, Opcode, and Flags,
*         respectively.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong etherType values.
*/
static CMD_STATUS wrCpssDxChVntCfmEtherTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 etherType;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    etherType = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChVntCfmEtherTypeSet(devNum, etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntCfmEtherTypeGet function
* @endinternal
*
* @brief   Gets the current EtherType to identify CFM
*         (Connectivity Fault Management) PDUs.
*         If the packet EtherType matches the CFM EtherType, the ingress Policy
*         key implicitly uses the three User-Defined-Byte (UDB0, UDB1, UDB2) to
*         contain the three CFM data fields, MD Level, Opcode, and Flags,
*         respectively.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChVntCfmEtherTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 etherType;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChVntCfmEtherTypeGet(devNum, &etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", etherType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntCfmLbrOpcodeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, opcode.
* @retval GT_OUT_OF_RANGE          - on wrong opcode values.
*/
static CMD_STATUS wrCpssDxChVntCfmLbrOpcodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 opcode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    opcode = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChVntCfmLbrOpcodeSet(devNum, opcode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntCfmLbrOpcodeGet function
* @endinternal
*
* @brief   Gets the current CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChVntCfmLbrOpcodeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 opcode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChVntCfmLbrOpcodeGet(devNum, &opcode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", opcode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChVntLastReadTimeStampGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 timeStampValue;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChVntLastReadTimeStampGet(devNum, &timeStampValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", timeStampValue);

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntOamPortPduTrapEnableSet function
* @endinternal
*
* @brief   Enable/Disable trap to CPU of 802.3ah Link Layer Control protocol on
*         the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         PDU trap is enabled for the port:
*         - Packets identified as OAM-PDUs destined for this port are trapped to the CPU.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChVntOamPortPduTrapEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChVntOamPortPduTrapEnableSet(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntOamPortPduTrapEnableGet function
* @endinternal
*
* @brief   Get trap to CPU status(Enable/Disable) of 802.3ah Link Layer Control protocol on
*         the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         PDU trap is enabled for the port:
*         - Packets identified as OAM-PDUs are trapped to the CPU.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on bad pointer of enablePtr
*/
static CMD_STATUS wrCpssDxChVntOamPortPduTrapEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;
    GT_PORT_NUM port;
    
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    
    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChVntOamPortPduTrapEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntCfmReplyModeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChVntCfmReplyModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChVntCfmReplyModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChVntCfmReplyModeGet function
* @endinternal
*
* @brief   Gets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - output parameter is NULL pointer.
*/
static CMD_STATUS wrCpssDxChVntCfmReplyModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChVntCfmReplyModeGet(devNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**** database initialization **************************************/



static CMD_COMMAND dbCommands[] =
{
        {"cpssDxChVntOamPortLoopBackModeEnableSet",
         &wrCpssDxChVntOamPortLoopBackModeEnableSet,
         3, 0},
        {"cpssDxChVntOamPortLoopBackModeEnableGet",
         &wrCpssDxChVntOamPortLoopBackModeEnableGet,
         2, 0},
        {"cpssDxChVntOamPortUnidirectionalEnableSet",
         &wrCpssDxChVntOamPortUnidirectionalEnableSet,
         3, 0},
        {"cpssDxChVntOamPortUnidirectionalEnableGet",
         &wrCpssDxChVntOamPortUnidirectionalEnableGet,
         2, 0},
        {"cpssDxChVntCfmEtherTypeSet",
         &wrCpssDxChVntCfmEtherTypeSet,
         2, 0},
        {"cpssDxChVntCfmEtherTypeGet",
         &wrCpssDxChVntCfmEtherTypeGet,
         1, 0},
        {"cpssDxChVntCfmLbrOpcodeSet",
         &wrCpssDxChVntCfmLbrOpcodeSet,
         2, 0},
        {"cpssDxChVntCfmLbrOpcodeGet",
         &wrCpssDxChVntCfmLbrOpcodeGet,
         1, 0},
        {"cpssDxChVntLastReadTimeStampGet",
         &wrCpssDxChVntLastReadTimeStampGet,
         1, 0},
        {"cpssDxChVntOamPortPduTrapEnableSet",
         &wrCpssDxChVntOamPortPduTrapEnableSet,
         3, 0},
        {"cpssDxChVntOamPortPduTrapEnableGet",
         &wrCpssDxChVntOamPortPduTrapEnableGet,
         2, 0},

        {"cpssDxChVntCfmReplyModeSet",
         &wrCpssDxChVntCfmReplyModeSet,
         2, 0},
        {"cpssDxChVntCfmReplyModeGet",
         &wrCpssDxChVntCfmReplyModeGet,
         1, 0},

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChVnt function
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
GT_STATUS cmdLibInitCpssDxChVnt
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


