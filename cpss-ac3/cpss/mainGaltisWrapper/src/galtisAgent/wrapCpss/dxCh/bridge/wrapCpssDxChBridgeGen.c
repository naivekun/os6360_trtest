/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
* wrapBridgeGenCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for BridgeGen cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 33 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
/**
* @internal wrCpssDxChBrgGenIgmpSnoopEnable function
* @endinternal
*
* @brief   Enable/disable trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgGenIgmpSnoopEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_PORT_NUM      port;
    GT_BOOL    status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    status = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenIgmpSnoopEnable(dev, port, status);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIgmpSnoopEnableGet function
* @endinternal
*
* @brief   Get status of trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIgmpSnoopEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_PORT_NUM      port;
    GT_BOOL    status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenIgmpSnoopEnableGet(dev, port, &status);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", status);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenDropIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgGenDropIpMcEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_BOOL    status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    status = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenDropIpMcEnable(dev, status);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenDropIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
static CMD_STATUS wrCpssDxChBrgGenDropIpMcEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_BOOL    status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenDropIpMcEnableGet(dev, &status);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", status);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenDropNonIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgGenDropNonIpMcEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_BOOL    status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    status = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenDropNonIpMcEnable(dev, status);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenDropNonIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
static CMD_STATUS wrCpssDxChBrgGenDropNonIpMcEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_BOOL    status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenDropNonIpMcEnableGet(dev, &status);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", status);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenDropInvalidSaEnable function
* @endinternal
*
* @brief   Drop all Ethernet packets with MAC SA that are Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgGenDropInvalidSaEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenDropInvalidSaEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenDropInvalidSaEnableGet function
* @endinternal
*
* @brief   Get status of droping all Ethernet packets with MAC SA that are
*         Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
static CMD_STATUS wrCpssDxChBrgGenDropInvalidSaEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenDropInvalidSaEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenUcLocalSwitchingEnable function
* @endinternal
*
* @brief   Enable/disable local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device or port number
*
* @note If the packet is received from a cascade port, the packet’s source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*
*/
static CMD_STATUS wrCpssDxChBrgGenUcLocalSwitchingEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_PORT_NUM      port;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenUcLocalSwitchingEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenUcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get status of local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*
* @note 1. If the packet is received from a cascade port, the packet’s source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*       2. For xCat and above devices to enable local switching of known
*       Unicast traffic, both ingress port configuration and VLAN entry (by
*       function cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
static CMD_STATUS wrCpssDxChBrgGenUcLocalSwitchingEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      dev;
    GT_PORT_NUM      port;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenUcLocalSwitchingEnableGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenIgmpSnoopModeSet function
* @endinternal
*
* @brief   Set global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on on bad device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgGenIgmpSnoopModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    CPSS_IGMP_SNOOP_MODE_ENT      mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    mode = (CPSS_IGMP_SNOOP_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenIgmpSnoopModeSet(dev, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenIgmpSnoopModeGet function
* @endinternal
*
* @brief   Get status of global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgGenIgmpSnoopModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    CPSS_IGMP_SNOOP_MODE_ENT      mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenIgmpSnoopModeGet(dev, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenArpBcastToCpuCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs,
*         with with IPv4/IPv6 Control Traffic To CPU Enable set by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or control packet command
*/
static CMD_STATUS wrCpssDxChBrgGenArpBcastToCpuCmdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenArpBcastToCpuCmdGet function
* @endinternal
*
* @brief   Get status of CPU ARP Broadcast packets for all VLANs
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
static CMD_STATUS wrCpssDxChBrgGenArpBcastToCpuCmdGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenArpBcastToCpuCmdSet1 function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs,
*         with with IPv4/IPv6 Control Traffic To CPU Enable set by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or control packet command
*/
static CMD_STATUS wrCpssDxChBrgGenArpBcastToCpuCmdSet1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    cmdMode = (CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT)inArgs[1];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenArpBcastToCpuCmdSet(dev, cmdMode, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenArpBcastToCpuCmdGet1 function
* @endinternal
*
* @brief   Get status of CPU ARP Broadcast packets for all VLANs
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
static CMD_STATUS wrCpssDxChBrgGenArpBcastToCpuCmdGet1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    cmdMode = (CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenArpBcastToCpuCmdGet(dev, cmdMode, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenRipV1MirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
*/
static CMD_STATUS wrCpssDxChBrgGenRipV1MirrorToCpuEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenRipV1MirrorToCpuEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenRipV1MirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgGenRipV1MirrorToCpuEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenRipV1MirrorToCpuEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenIeeeReservedMcastTrapEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
*/
static CMD_STATUS wrCpssDxChBrgGenIeeeReservedMcastTrapEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenIeeeReservedMcastTrapEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIeeeReservedMcastTrapEnableGet function
* @endinternal
*
* @brief   Get status of trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgGenIeeeReservedMcastTrapEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenIeeeReservedMcastProtCmdSet1 function
* @endinternal
*
* @brief   Enables forwarding, trapping, or mirroring to the CPU any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, protocol or control packet command
*
* @note BPDU packets with MAC DA = 01-80-C2-00-00-00 are not affect
*       by this mechanism. BPDUs are implicitly trapped to the CPU
*       if the ingress port span tree is not disabled.
*
*/
static CMD_STATUS wrCpssDxChBrgGenIeeeReservedMcastProtCmdSet1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;
    GT_U8                    dev;
    GT_U32                   profileIndex;
    GT_U8                    protocol;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    profileIndex = (GT_U32)inArgs[1];
    protocol = (GT_U8)inArgs[2];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, profileIndex, protocol, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

static GT_U32 xCatProfileIndex = 0; /* for DxCh1, DxCh2, DxCh3 - 0 is used */
                                    /* for DxChXcat and above - use wrapper
           wrCpssDxChBrgGenIeeeReservedMcastProtProfileSet to configure it */

/**
* @internal wrCpssDxChBrgGenIeeeReservedMcastProtCmdSet function
* @endinternal
*
* @brief   Enables forwarding, trapping, or mirroring to the CPU any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, protocol or control packet command
*
* @note BPDU packets with MAC DA = 01-80-C2-00-00-00 are not affect
*       by this mechanism. BPDUs are implicitly trapped to the CPU
*       if the ingress port span tree is not disabled.
*
*/
static CMD_STATUS wrCpssDxChBrgGenIeeeReservedMcastProtCmdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_U8                    protocol;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    protocol = (GT_U8)inArgs[1];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev, xCatProfileIndex, protocol, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIeeeReservedMcastProtCmdGet function
* @endinternal
*
* @brief   Gets command (forwarding, trapping, or mirroring to the CPU) any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, protocol or control packet command
* @retval GT_OUT_OF_RANGE          - for profileIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIeeeReservedMcastProtCmdGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;
    GT_U8                    dev;
    GT_U8                    protocol;
    GT_U32                   profileIndex;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    profileIndex = (GT_U32)inArgs[1];
    protocol = (GT_U8)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(dev, profileIndex, protocol, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet function
* @endinternal
*
* @brief   Select the IEEE Reserved Multicast profile (table) associated with port
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev, port
* @retval GT_OUT_OF_RANGE          - for profileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                dev;
    GT_PORT_NUM                port;
    GT_U32               profileIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    profileIndex = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet function
* @endinternal
*
* @brief   Get the IEEE Reserved Multicast profile (table) associated with port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev, port or profileId
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                dev;
    GT_PORT_NUM                port;
    GT_U32               profileIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(dev, port, &profileIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileIndex);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIeeeReservedMcastProtProfileSet function
* @endinternal
*
* @brief   Sets profile index for wrCpssDxChBrgGenIeeeReservedMcastProtCmdSet
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssDxChBrgGenIeeeReservedMcastProtProfileSet

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

    /* map input arguments to global parameter */
    xCatProfileIndex = (GT_U32)inArgs[0];

     /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenCiscoL2ProtCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or control packet command
*/
static CMD_STATUS wrCpssDxChBrgGenCiscoL2ProtCmdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenCiscoL2ProtCmdSet(dev, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenCiscoL2ProtCmdGet function
* @endinternal
*
* @brief   Get status (trapping or mirroring) of the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BADPTR                - on NULL Ptr
*/
static CMD_STATUS wrCpssDxChBrgGenCiscoL2ProtCmdGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenCiscoL2ProtCmdGet(dev, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpV6IcmpTrapEnable function
* @endinternal
*
* @brief   Enable/disable IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
*/
static CMD_STATUS wrCpssDxChBrgGenIpV6IcmpTrapEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_PORT_NUM                    port;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenIpV6IcmpTrapEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpV6IcmpTrapEnableGet function
* @endinternal
*
* @brief   Gets status of IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIpV6IcmpTrapEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_PORT_NUM                    port;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenIpV6IcmpTrapEnableGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIcmpv6MsgTypeSet function
* @endinternal
*
* @brief   Set trap/mirror/forward command for specified ICMP message type.
*         To enable ICMPv6 trapping/mirroring on a VLAN interface,
*         set the <IPv6ICMP ToCPUEn> bit in the VLAN<n> Entry by function
*         cpssDxChBrgVlanIpV6IcmpToCpuEnable
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or control packet command
*                                       or index > 7
*/
static CMD_STATUS wrCpssDxChBrgGenIcmpv6MsgTypeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_U32                   index;
    GT_U8                    msgType;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    msgType = (GT_U8)inArgs[2];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgGenIcmpv6MsgTypeSet(dev, index, msgType, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIcmpv6MsgTypeGet function
* @endinternal
*
* @brief   Get status of trap/mirror/forward command for specified ICMP message type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
*                                       or index > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
static CMD_STATUS wrCpssDxChBrgGenIcmpv6MsgTypeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_U32                   index;
    GT_U8                    msgType;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenIcmpv6MsgTypeGet(dev, index, &msgType, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", msgType, cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpLinkLocalMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*         INPUTS:
*         dev      - device number
*         protocolStack - IPv4 or IPv6 protocol version
*         enable     - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP protocol version
*/
static CMD_STATUS wrCpssDxChBrgGenIpLinkLocalMirrorToCpuEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           dev;
    CPSS_IP_PROTOCOL_STACK_ENT      protocolStack;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(dev, protocolStack,
                                                                    enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of mirroring to CPU
*         for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*         INPUTS:
*         devNum     - device number
*         protocolStack - IPv4 or IPv6 protocol version
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or IP protocol version
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           dev;
    CPSS_IP_PROTOCOL_STACK_ENT      protocolStack;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(dev, protocolStack,
                                                                    &enable);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      Relevant for 98DX167/167R/247/253/263/273 devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, IP protocol version or
*                                       Link Local IP protocol
*/
static CMD_STATUS wrCpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           dev;
    CPSS_IP_PROTOCOL_STACK_ENT      protocolStack;
    GT_U8                           protocol;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    protocol = (GT_U8)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(dev, protocolStack,
                                                              protocol, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, IP protocol version or
*                                       Link Local IP protocol
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           dev;
    CPSS_IP_PROTOCOL_STACK_ENT      protocolStack;
    GT_U8                           protocol;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    protocol = (GT_U8)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(dev, protocolStack,
                                                              protocol, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpV6SolicitedCmdSet function
* @endinternal
*
* @brief   Set trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or packet control command
*/
static CMD_STATUS wrCpssDxChBrgGenIpV6SolicitedCmdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenIpV6SolicitedCmdSet(dev, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpV6SolicitedCmdGet function
* @endinternal
*
* @brief   Get status of trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or packet control command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgGenIpV6SolicitedCmdGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    CPSS_PACKET_CMD_ENT      cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenIpV6SolicitedCmdGet(dev, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet function
* @endinternal
*
* @brief   Set IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_IPV6ADDR              addressPtr;
    GT_IPV6ADDR              maskPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    galtisIpv6Addr(&addressPtr, (GT_U8*)inArgs[1]);
    galtisIpv6Addr(&maskPtr, (GT_U8*)inArgs[2]);

    /* call cpss api function */
    result = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet(dev, &addressPtr,
                                                                   &maskPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet function
* @endinternal
*
* @brief   Get IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_IPV6ADDR              addressPtr;
    GT_IPV6ADDR              maskPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet(dev, &addressPtr,
                                                                   &maskPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%16B%16B", addressPtr.arIP , maskPtr.arIP);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenRateLimitGlobalCfgSet function
* @endinternal
*
* @brief   Configures global ingress rate limit parameters - rate limiting mode,
*         Four configurable global time window periods are assigned to ports as
*         a function of their operating speed:
*         10 Gbps, 1 Gbps, 100 Mbps, and 10 Mbps.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Supported windows for DxCh devices:
*       1000 Mbps: range - 0-16128 uSec  granularity - 256 uSec
*       100 Mbps: range - 0-130816 uSec  granularity - 256 uSec
*       10  Mbps: range - 0-1048576 uSec granularity - 256 uSec
*       10  Gbps: range - 0-104857.6 uSec granularity - 25.6 uSec
*       Time ranges and granularity specified for clock 200 Mhrz for
*       they changed linearly with the clock value
*
*/
static CMD_STATUS wrCpssDxChBrgGenRateLimitGlobalCfgSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               dev;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    brgRateLimitPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    brgRateLimitPtr.dropMode = (CPSS_DROP_MODE_TYPE_ENT)inFields[0];
    brgRateLimitPtr.rMode = (CPSS_RATE_LIMIT_MODE_ENT)inFields[1];
    brgRateLimitPtr.win10Mbps = (GT_U32)inFields[2];
    brgRateLimitPtr.win100Mbps = (GT_U32)inFields[3];
    brgRateLimitPtr.win1000Mbps = (GT_U32)inFields[4];
    brgRateLimitPtr.win10Gbps = (GT_U32)inFields[5];

    /* call cpss api function */
    result = cpssDxChBrgGenRateLimitGlobalCfgSet(dev, &brgRateLimitPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenRateLimitGlobalCfgGet function
* @endinternal
*
* @brief   Get global ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgGenRateLimitGlobalCfgGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                         result;

    GT_U8                             dev;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC  brgRateLimitPtr;
    static GT_U32                     isNext = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(isNext == 1)
    {
        isNext = 0;
        /* there is no GetNext for wrapper */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    isNext = 1;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenRateLimitGlobalCfgGet(dev, &brgRateLimitPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = brgRateLimitPtr.dropMode;
    inFields[1] = brgRateLimitPtr.rMode;
    inFields[2] = brgRateLimitPtr.win10Mbps;
    inFields[3] = brgRateLimitPtr.win100Mbps;
    inFields[4] = brgRateLimitPtr.win1000Mbps;
    inFields[5] = brgRateLimitPtr.win10Gbps;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                inFields[3], inFields[4], inFields[5]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

static GT_BOOL  isMcSeparate; /* if GT_TRUE-this is wrapper for for xCat2 API
                               GT_FALSE-otherwise */
/**
* @internal wrCpssDxChBrgGenPortRateLimitSet function
* @endinternal
*
* @brief   Configures port ingress rate limit parameters
*         Each port maintains rate limits for unknown unicast packets,
*         known unicast packets, multicast packets and broadcast packets,
*         single configurable limit threshold value, and a single internal counter.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_OUT_OF_RANGE          - on out of range rate limit values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;

    GT_U8                                    dev;
    GT_PHYSICAL_PORT_NUM                                    port;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC    portGfgPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    portGfgPtr.enableBc = (GT_BOOL)inFields[0];
    portGfgPtr.enableMc = (GT_BOOL)inFields[1];
    portGfgPtr.enableUcUnk = (GT_BOOL)inFields[2];
    portGfgPtr.enableUcKnown = (GT_BOOL)inFields[3];
    portGfgPtr.rateLimit = (GT_U32)inFields[4];


    /* Check that the API was called from xCat2 wrapper */
    if (isMcSeparate == GT_TRUE)
    {
        portGfgPtr.enableMcReg = (GT_BOOL)inFields[5];

        isMcSeparate = GT_FALSE;
    }
    else
    {
        portGfgPtr.enableMcReg = portGfgPtr.enableMc;
    }

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitSet(dev, port, &portGfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortRateLimitGet function
* @endinternal
*
* @brief   Get port ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on bad device or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                              result;
    GT_U8                                  dev;
    GT_PHYSICAL_PORT_NUM                                  port;
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC  portGfgPtr;
    static GT_U32                          isNext = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(isNext == 1)
    {
        isNext = 0;
        /* there is no GetNext for wrapper */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    isNext = 1;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitGet(dev, port, &portGfgPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = portGfgPtr.enableBc;
    inFields[1] = portGfgPtr.enableMc;
    inFields[2] = portGfgPtr.enableUcUnk;
    inFields[3] = portGfgPtr.enableUcKnown;
    inFields[4] = portGfgPtr.rateLimit;


    if (isMcSeparate == GT_TRUE)
    {
        inFields[5] = portGfgPtr.enableMcReg;
        isMcSeparate = GT_FALSE;
    }
    else
    {
        inFields[5]=portGfgPtr.enableMc;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d", inFields[0], inFields[1],
                inFields[2], inFields[3], inFields[4],
                inFields[5]);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}
/****************************ch3 ext********************************************/
/**
* @internal wrCpssDxChBrgGenPortRateLimitExtSet function
* @endinternal
*
* @brief   Configures port ingress rate limit parameters
*         Each port maintains rate limits for unknown unicast packets,
*         known unicast packets, multicast packets and broadcast packets,
*         single configurable limit threshold value, and a single internal counter.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_OUT_OF_RANGE          - on out of range rate limit values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note wrappers for Ch3
*
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitExtSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    isMcSeparate = GT_TRUE;

    return wrCpssDxChBrgGenPortRateLimitSet(inArgs,inFields,numFields,outArgs);
}


/**
* @internal wrCpssDxChBrgGenPortRateLimitExtGet function
* @endinternal
*
* @brief   Get port ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on bad device or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note wrappers for Ch3
*
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitExtGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    isMcSeparate = GT_TRUE;

    return wrCpssDxChBrgGenPortRateLimitGet(inArgs,inFields,numFields,outArgs);
}


/**
* @internal wrCpssDxChBrgGenPortRateLimitTcpSynSet function
* @endinternal
*
* @brief   Enable or disable TCP SYN packets' rate limit for a port.
*         Value of rate limit is configured by cpssDxChBrgGenPortRateLimitSet and
*         cpssDxChBrgGenRateLimitGlobalCfgSet
*
* @note   APPLICABLE DEVICES:      All CH2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on bad device or port number
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitTcpSynSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM                    port;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitTcpSynSet(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenPortRateLimitTcpSynGet function
* @endinternal
*
* @brief   Get the status of TCP SYN packets' rate limit for a port (enable/disable).
*         Value of rate limit can be read by cpssDxChBrgGenPortRateLimitGet and
*         cpssDxChBrgGenRateLimitGlobalCfgGet.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitTcpSynGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM                    port;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitTcpSynGet(dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,  "%d", enable);
    return CMD_OK;
}






/**
* @internal wrCpssDxChBrgGenUdpBcDestPortCfgSet function
* @endinternal
*
* @brief   Configure UDP Broadcast Destination Port configuration table.
*         It's possible to configure several UDP destination ports with their Trap
*         or Mirror to CPU command and CPU code. See datasheet of device for maximal
*         number of UDP destination ports, which may be configured. The feature may
*         be enabled or disabled per VLAN by cpssDxChBrgVlanUdpBcPktsToCpuEnable
*
* @note   APPLICABLE DEVICES:      All CH2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad device or CPU code or cmd or
*                                       entryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenUdpBcDestPortCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_U32                        entryIndex;
    GT_U16                        udpPortNum;
    CPSS_NET_RX_CPU_CODE_ENT      cpuCode;
    CPSS_PACKET_CMD_ENT           cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];
    udpPortNum = (GT_U16)inArgs[2];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[3];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChBrgGenUdpBcDestPortCfgSet(dev, entryIndex, udpPortNum,
                                                              cpuCode, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenUdpBcDestPortCfgGet function
* @endinternal
*
* @brief   Get UDP Broadcast Destination Port configuration table.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad device or entryIndex > 11
* @retval GT_HW_ERROR              - failed to read from hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenUdpBcDestPortCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_U32                        entryIndex;
    GT_BOOL                       valid;
    GT_U16                        udpPortNum;
    CPSS_NET_RX_CPU_CODE_ENT      cpuCode;
    CPSS_PACKET_CMD_ENT           cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenUdpBcDestPortCfgGet(dev, entryIndex,
                                               &valid, &udpPortNum,
                                               &cpuCode, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", valid, udpPortNum,
                                              cpuCode, cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenUdpBcDestPortCfgInvalidate function
* @endinternal
*
* @brief   Invalidate UDP Broadcast Destination Port configuration table.
*         All fields in entry will be reset.
*
* @note   APPLICABLE DEVICES:      All CH2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad device or intryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
*/
static CMD_STATUS wrCpssDxChBrgGenUdpBcDestPortCfgInvalidate

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_U32                        entryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenUdpBcDestPortCfgInvalidate(dev, entryIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenBpduTrapEnableSet function
* @endinternal
*
* @brief   Enable or disable trapping of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      All CH2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
*/
static CMD_STATUS wrCpssDxChBrgGenBpduTrapEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenBpduTrapEnableSet(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenBpduTrapEnableGet function
* @endinternal
*
* @brief   Get trapping status of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenBpduTrapEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenBpduTrapEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenArpTrapEnable function
* @endinternal
*
* @brief   Enable/disable trapping ARP Broadcast packets to the CPU.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
*/
static CMD_STATUS wrCpssDxChBrgGenArpTrapEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_PORT_NUM                         port;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenArpTrapEnable(dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenArpTrapEnableGet function
* @endinternal
*
* @brief   Get trapping ARP Broadcast packets to the CPU status.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenArpTrapEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PORT_NUM     port;
    GT_BOOL   enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgGenArpTrapEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;

}

/**
* @internal wrCpssDxChBrgGenDropIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
static CMD_STATUS wrCpssDxChBrgGenDropIpMcModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    CPSS_DROP_MODE_TYPE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    mode = (CPSS_DROP_MODE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenDropIpMcModeSet(dev, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenDropIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
static CMD_STATUS wrCpssDxChBrgGenDropIpMcModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    CPSS_DROP_MODE_TYPE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenDropIpMcModeGet(dev, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgGenDropNonIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
static CMD_STATUS wrCpssDxChBrgGenDropNonIpMcModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    CPSS_DROP_MODE_TYPE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    mode = (CPSS_DROP_MODE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenDropNonIpMcModeSet(dev, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenDropNonIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
static CMD_STATUS wrCpssDxChBrgGenDropNonIpMcModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    CPSS_DROP_MODE_TYPE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenDropNonIpMcModeGet(dev, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenCfiRelayEnableSet function
* @endinternal
*
* @brief   Enable / Disable CFI bit relay.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenCfiRelayEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenCfiRelayEnableSet(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenCfiRelayEnableGet function
* @endinternal
*
* @brief   Get CFI bit relay status.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenCfiRelayEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenCfiRelayEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet function
* @endinternal
*
* @brief   Enable/disable dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PORT_NUM port;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev    = (GT_U8)inArgs[0];
    port   = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet function
* @endinternal
*
* @brief   Get Enable/disable status of dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 dev;
    GT_PORT_NUM port;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev  = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenRateLimitSpeedCfgEnableSet function
* @endinternal
*
* @brief   Enable/Disable (per network 1Gbps/100Mbps/10Mbps port) the port speed
*         configuration that will be used by the Bridge ingress rate limiter,
*         regardless of the actual speed in which the network port actually works.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above exclude Lion
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The feature is not relevant for Flex link ports.
*
*/
static CMD_STATUS wrCpssDxChBrgGenRateLimitSpeedCfgEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   dev;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev    = (GT_U8)inArgs[0];
    enable   = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenRateLimitSpeedCfgEnableSet(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenRateLimitSpeedCfgEnableGet function
* @endinternal
*
* @brief   Get the status of the port speed configuration (Enabled/Disabled) that
*         will be used by the Bridge ingress rate limiter, regardless of the actual
*         speed in which the network port actually works.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above exclude Lion
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The feature is not relevant for Flex link ports.
*
*/
static CMD_STATUS wrCpssDxChBrgGenRateLimitSpeedCfgEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   dev;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev    = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenRateLimitSpeedCfgEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortRateLimitSpeedGranularitySet function
* @endinternal
*
* @brief   Configure 100Mbps/10Mbps/1Gbps port speed for the use of the bridge
*         ingress rate limit calculations.
*         If port speed for Bridge ingress rate limit calculations is enabled,
*         cpssDxChBrgGenRateLimitSpeedCfgEnableSet() API, then this configuration
*         is used.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above exclude Lion
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum or speedGranularity
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The feature is not relevant for Flex link ports.
*
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitSpeedGranularitySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_PORT_SPEED_ENT     speedGranularity;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev    = (GT_U8)inArgs[0];
    port   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    speedGranularity = (CPSS_PORT_SPEED_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitSpeedGranularitySet(
        dev, port, speedGranularity);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenPortRateLimitSpeedGranularityGet function
* @endinternal
*
* @brief   Get port speed configuration. This configuration is used for bridge
*         ingress rate limit calculations.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above exclude Lion
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The feature is not relevant for Flex link ports.
*
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitSpeedGranularityGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_PORT_SPEED_ENT     speedGranularity;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev  = (GT_U8)inArgs[0];
    port = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitSpeedGranularityGet(
        dev, port, &speedGranularity);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", speedGranularity);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet function
* @endinternal
*
* @brief   Enable / Disable excluding unregistered IPv4 / IPv6 Link Local Multicast
*         packets from the per VLAN Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        devNum;
    CPSS_IP_PROTOCOL_STACK_ENT   protocolStack;
    GT_BOOL                      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    protocolStack   = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet(
        devNum, protocolStack, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet function
* @endinternal
*
* @brief   Get enable / disable status for excluding unregistered IPv4 / IPv6
*         Link Local Multicast packets from the per VLAN
*         Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        devNum;
    CPSS_IP_PROTOCOL_STACK_ENT   protocolStack;
    GT_BOOL                      enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    protocolStack   = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet(
        devNum, protocolStack, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PORT_NUM       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];
    enable    = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(devNum,
                                                                  portNum,
                                                                  enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet function
* @endinternal
*
* @brief   Get status of MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PORT_NUM       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet(devNum,
                                                                  portNum,
                                                                  &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortRateLimitDropCntrEnableSet function
* @endinternal
*
* @brief   Enable/Disable port for counting packets dropped due to ingress rate
*         limiting by the Ingress Rate Limit Drop Counter
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitDropCntrEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable    = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitDropCntrEnableSet(devNum,
                                                          portNum,
                                                          enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortRateLimitDropCntrEnableGet function
* @endinternal
*
* @brief   Get port status (enabled/disabled) for counting packets dropped due
*         to ingress rate limiting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortRateLimitDropCntrEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortRateLimitDropCntrEnableGet(devNum,
                                                          portNum,
                                                          &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenRateLimitDropCntrSet function
* @endinternal
*
* @brief   Set rate limit drop counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range drop counter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenRateLimitDropCntrSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U64      value;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum      = (GT_U8)inArgs[0];
    value.l[0]  = (GT_U32)inArgs[1];
    value.l[1]  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenRateLimitDropCntrSet(devNum, value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenRateLimitDropCntrGet function
* @endinternal
*
* @brief   Get rate limit drop counter.
*         The counter counts packets or bytes according to ingress rate limit mode
*         set by cpssDxChBrgGenRateLimitGlobalCfgSet.
*         The counter counts packets/bytes that are dropped due to ingress rate
*         limitting on all ports, where the drop rate limit counting is enabled
*         by cpssDxChBrgGenPortRateLimitDropCntrEnableSet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenRateLimitDropCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U64      value;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenRateLimitDropCntrGet(devNum, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d", value.l[0], value.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortArpMacSaMismatchDropEnable function
* @endinternal
*
* @brief   Enable/disable configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The global ARP MAC SA configuration must remain. When both global
*       configuration is enabled and per port configuration is enabled the
*       ARP MAC SA mismatch check is performed and if fails packet is dropped.
*
*/
static CMD_STATUS wrCpssDxChBrgGenPortArpMacSaMismatchDropEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PORT_NUM portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];
    enable    = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(
        devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortArpMacSaMismatchDropEnableGet function
* @endinternal
*
* @brief   Get configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortArpMacSaMismatchDropEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PORT_NUM portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet(
        devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PORT_NUM          portNum;
    CPSS_PACKET_CMD_ENT  cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];
    cmd       = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet(
        devNum, portNum, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PORT_NUM          portNum;
    CPSS_PACKET_CMD_ENT  cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet(
        devNum, portNum, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PORT_NUM          portNum;
    CPSS_PACKET_CMD_ENT  cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];
    cmd       = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet(
        devNum, portNum, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PORT_NUM          portNum;
    CPSS_PACKET_CMD_ENT  cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet(
        devNum, portNum, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIngressPortBcFilterDaCommandSet function
* @endinternal
*
* @brief   Set BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIngressPortBcFilterDaCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PORT_NUM          portNum;
    CPSS_PACKET_CMD_ENT  cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];
    cmd       = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenIngressPortBcFilterDaCommandSet(
        devNum, portNum, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIngressPortBcFilterDaCommandGet function
* @endinternal
*
* @brief   Get BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIngressPortBcFilterDaCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PORT_NUM          portNum;
    CPSS_PACKET_CMD_ENT  cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenIngressPortBcFilterDaCommandGet(
        devNum, portNum, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortIpControlTrapEnableSet function
* @endinternal
*
* @brief   Set IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortIpControlTrapEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    GT_PORT_NUM                          portNum;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT       ipCntrlType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum       = (GT_U8)inArgs[0];
    portNum      = (GT_PORT_NUM)inArgs[1];
    ipCntrlType  = (CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortIpControlTrapEnableSet(
        devNum, portNum, ipCntrlType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortIpControlTrapEnableGet function
* @endinternal
*
* @brief   Get IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortIpControlTrapEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    GT_PORT_NUM                          portNum;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT       ipCntrlType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortIpControlTrapEnableGet(
        devNum, portNum, &ipCntrlType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ipCntrlType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortBcUdpTrapMirrorEnableSet function
* @endinternal
*
* @brief   Set BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortBcUdpTrapMirrorEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_PORT_NUM     portNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum       = (GT_U8)inArgs[0];
    portNum      = (GT_PORT_NUM)inArgs[1];
    enable       = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(
        devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortBcUdpTrapMirrorEnableGet function
* @endinternal
*
* @brief   Get BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenPortBcUdpTrapMirrorEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            devNum;
    GT_PORT_NUM      portNum;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(
        devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet function
* @endinternal
*
* @brief   Set the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable/disable filtering of untagged packets with length bigger than
*       1518 bytes on ePort, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
*
*/
static CMD_STATUS wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    CPSS_PACKET_CMD_ENT      command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum       = (GT_U8)inArgs[0];
    command      = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet(devNum, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet function
* @endinternal
*
* @brief   Get the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the filtering status on ePort of untagged packets with length bigger
*       than 1518 bytes, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
*
*/
static CMD_STATUS wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    CPSS_PACKET_CMD_ENT      command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet(devNum, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenBypassModeSet function
* @endinternal
*
* @brief   Set Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
static CMD_STATUS wrCpssDxChBrgGenBypassModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               dev;
    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenBypassModeSet(dev, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenBypassModeGet function
* @endinternal
*
* @brief   Get Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
static CMD_STATUS wrCpssDxChBrgGenBypassModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               dev;
    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenBypassModeGet(dev, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenMtuConfigSet function
* @endinternal
*
* @brief   cpssDxChBrgGenMtuConfigSet
*         DESCRIPTION:
*         Enable/disable Target ePort MTU check feature.
*         and configure the following parameters:
*         - the target ePort MTU check feature:
*         GT_TRUE = enabled / GT_FALSE = disabled
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenMtuConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                    result;
    GT_U8                                        devNum;
    GT_BOOL                                      mtuCheckEnable;
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode;
    CPSS_PACKET_CMD_ENT                          exceptionCommand;
    CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    mtuCheckEnable  = (GT_BOOL)inArgs[1];
    byteCountMode   = (CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT)inArgs[2];
    exceptionCommand= (CPSS_PACKET_CMD_ENT)inArgs[3];
    exceptionCpuCode= (CPSS_NET_RX_CPU_CODE_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChBrgGenMtuConfigSet(devNum, mtuCheckEnable,
                                        byteCountMode, exceptionCommand,
                                        exceptionCpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenMtuConfigGet function
* @endinternal
*
* @brief   Get the target ePort MTU feature parameters:
*         - the enable/disable target ePort MTU check feature.
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong HW value
*/
static CMD_STATUS wrCpssDxChBrgGenMtuConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                    result;
    GT_U8                                        devNum;
    GT_BOOL                                      mtuCheckEnable;
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode;
    CPSS_PACKET_CMD_ENT                          exceptionCommand;
    CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenMtuConfigGet(devNum, &mtuCheckEnable,
                                        &byteCountMode, &exceptionCommand,
                                        &exceptionCpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", mtuCheckEnable,
                 byteCountMode, exceptionCommand,
                 exceptionCpuCode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenMtuExceedCntrGet function
* @endinternal
*
* @brief   Get target ePort MTU exceeded counter.
*         The number of packets that triggered the Target ePort MTU
*         exceeded exception check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenMtuExceedCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   value;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenMtuExceedCntrGet(devNum, &value);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", value);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenMtuProfileSet function
* @endinternal
*
* @brief   Set an MTU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range mtuSize
*/
static CMD_STATUS wrCpssDxChBrgGenMtuProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32   profile;
    GT_U32   mtuSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    profile  = (GT_U32)inArgs[1];
    mtuSize  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgGenMtuProfileSet(devNum, profile, mtuSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenMtuProfileGet function
* @endinternal
*
* @brief   Get an MTU size for ePort profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenMtuProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32   profile;
    GT_U32   mtuSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    profile  = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenMtuProfileGet(devNum, profile, &mtuSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mtuSize);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenMtuPortProfileIdxSet function
* @endinternal
*
* @brief   Set MTU profile index for the given target ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_OUT_OF_RANGE     – mtuProfileIdx out of range
*/
static CMD_STATUS wrCpssDxChBrgGenMtuPortProfileIdxSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_PORT_NUM  portNum;
    GT_U32   profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum  = (GT_PORT_NUM)inArgs[1];
    profile  = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenMtuPortProfileIdxSet(devNum, portNum, profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenMtuPortProfileIdxGet function
* @endinternal
*
* @brief   Get MTU profile index for the given ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenMtuPortProfileIdxGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PORT_NUM portNum;
    GT_U32    profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U32)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpmClassificationModeSet function
* @endinternal
*
* @brief   Configue IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIpmClassificationModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           dev;
    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgGenIpmClassificationModeSet(dev, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgGenIpmClassificationModeGet function
* @endinternal
*
* @brief   Get IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgGenIpmClassificationModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           dev;
    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgGenIpmClassificationModeGet(dev, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChBrgGenIgmpSnoopEnable",
        &wrCpssDxChBrgGenIgmpSnoopEnable,
        3, 0},

    {"cpssDxChBrgGenIgmpSnoopEnableGet",
        &wrCpssDxChBrgGenIgmpSnoopEnableGet,
        2, 0},

    {"cpssDxChBrgGenDropIpMcEnable",
        &wrCpssDxChBrgGenDropIpMcEnable,
        2, 0},

    {"cpssDxChBrgGenDropIpMcEnableGet",
        &wrCpssDxChBrgGenDropIpMcEnableGet,
        1, 0},

    {"cpssDxChBrgGenDropNonIpMcEnable",
        &wrCpssDxChBrgGenDropNonIpMcEnable,
        2, 0},

    {"cpssDxChBrgGenDropNonIpMcEnableGet",
        &wrCpssDxChBrgGenDropNonIpMcEnableGet,
        1, 0},

    {"cpssDxChBrgGenDropInvalidSaEnable",
        &wrCpssDxChBrgGenDropInvalidSaEnable,
        2, 0},

    {"cpssDxChBrgGenDropInvalidSaEnableGet",
        &wrCpssDxChBrgGenDropInvalidSaEnableGet,
        1, 0},

    {"cpssDxChBrgGenUcLocalSwitchingEnable",
        &wrCpssDxChBrgGenUcLocalSwitchingEnable,
        3, 0},

    {"cpssDxChBrgGenUcLocalSwitchingEnableGet",
        &wrCpssDxChBrgGenUcLocalSwitchingEnableGet,
        2, 0},

    {"cpssDxChBrgGenIgmpSnoopModeSet",
        &wrCpssDxChBrgGenIgmpSnoopModeSet,
        2, 0},

    {"cpssDxChBrgGenIgmpSnoopModeGet",
        &wrCpssDxChBrgGenIgmpSnoopModeGet,
        1, 0},

    {"cpssDxChBrgGenArpBcastToCpuCmdSet",
        &wrCpssDxChBrgGenArpBcastToCpuCmdSet,
        2, 0},

    {"cpssDxChBrgGenArpBcastToCpuCmdGet",
        &wrCpssDxChBrgGenArpBcastToCpuCmdGet,
        1, 0},

    {"cpssDxChBrgGenArpBcastToCpuCmdSet_1",
        &wrCpssDxChBrgGenArpBcastToCpuCmdSet1,
        3, 0},

    {"cpssDxChBrgGenArpBcastToCpuCmdGet_1",
        &wrCpssDxChBrgGenArpBcastToCpuCmdGet1,
        2, 0},

    {"cpssDxChBrgGenRipV1MirrorToCpuEnable",
        &wrCpssDxChBrgGenRipV1MirrorToCpuEnable,
        2, 0},

    {"cpssDxChBrgGenRipV1MirrorToCpuEnableGet",
        &wrCpssDxChBrgGenRipV1MirrorToCpuEnableGet,
        1, 0},

    {"cpssDxChBrgGenIeeeReservedMcastTrapEnable",
        &wrCpssDxChBrgGenIeeeReservedMcastTrapEnable,
        2, 0},

    {"cpssDxChBrgGenIeeeReservedMcastTrapEnableGet",
        &wrCpssDxChBrgGenIeeeReservedMcastTrapEnableGet,
        1, 0},

    {"cpssDxChBrgGenIeeeReservedMcastProtCmdSet",
        &wrCpssDxChBrgGenIeeeReservedMcastProtCmdSet,
        3, 0},

    {"cpssDxChBrgGenIeeeReservedMcastProtCmdGet",
        &wrCpssDxChBrgGenIeeeReservedMcastProtCmdGet,
        3, 0},

    {"cpssDxChBrgGenIeeeReservedMcastProtCmdSet1",
        &wrCpssDxChBrgGenIeeeReservedMcastProtCmdSet1,
        4, 0},

    {"cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet",
        &wrCpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet,
        3, 0},

    {"cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet",
        &wrCpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet,
        2, 0},

    {"cpssDxChBrgGenIeeeReservedMcastProtProfileSet",
        &wrCpssDxChBrgGenIeeeReservedMcastProtProfileSet,
        1, 0 },

    {"cpssDxChBrgGenCiscoL2ProtCmdSet",
        &wrCpssDxChBrgGenCiscoL2ProtCmdSet,
        2, 0},

    {"cpssDxChBrgGenCiscoL2ProtCmdGet",
        &wrCpssDxChBrgGenCiscoL2ProtCmdGet,
        1, 0},

    {"cpssDxChBrgGenIpV6IcmpTrapEnable",
        &wrCpssDxChBrgGenIpV6IcmpTrapEnable,
        3, 0},

    {"cpssDxChBrgGenIpV6IcmpTrapEnableGet",
        &wrCpssDxChBrgGenIpV6IcmpTrapEnableGet,
        2, 0},

    {"cpssDxChBrgGenIcmpv6MsgTypeSet",
        &wrCpssDxChBrgGenIcmpv6MsgTypeSet,
        4, 0},

    {"cpssDxChBrgGenIcmpv6MsgTypeGet",
        &wrCpssDxChBrgGenIcmpv6MsgTypeGet,
        2, 0},

    {"cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable",
        &wrCpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
        3, 0},

    {"cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet",
        &wrCpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet,
        2, 0},

    {"cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable",
        &wrCpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable,
        4, 0},

    {"cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet",
        &wrCpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet,
        3, 0},

    {"cpssDxChBrgGenIpV6SolicitedCmdSet",
        &wrCpssDxChBrgGenIpV6SolicitedCmdSet,
        2, 0},

    {"cpssDxChBrgGenIpV6SolicitedCmdGet",
        &wrCpssDxChBrgGenIpV6SolicitedCmdGet,
        1, 0},

    {"cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet",
        &wrCpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet,
        3, 0},

    {"cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet",
        &wrCpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet,
        1, 0},

    {"cpssDxChBrgGenRateLimitGlobalCfgSet",
        &wrCpssDxChBrgGenRateLimitGlobalCfgSet,
        1, 6},

    {"cpssDxChBrgGenRateLimitGlobalCfgGetFirst",
        &wrCpssDxChBrgGenRateLimitGlobalCfgGet,
        1, 0},

    {"cpssDxChBrgGenRateLimitGlobalCfgGetNext",
        &wrCpssDxChBrgGenRateLimitGlobalCfgGet,
        1, 0},

    {"cpssDxChBrgGenPortRateLimitSet",
        &wrCpssDxChBrgGenPortRateLimitSet,
        2, 5},

    {"cpssDxChBrgGenPortRateLimitGetFirst",
        &wrCpssDxChBrgGenPortRateLimitGet,
        2, 0},

    {"cpssDxChBrgGenPortRateLimitGetNext",
        &wrCpssDxChBrgGenPortRateLimitGet,
        2, 0},

    {"cpssDxCh3BrgGenPortRateLimitSet",
        &wrCpssDxChBrgGenPortRateLimitExtSet,
        2, 6},

    {"cpssDxCh3BrgGenPortRateLimitGetFirst",
        &wrCpssDxChBrgGenPortRateLimitExtGet,
        2, 0},

    {"cpssDxCh3BrgGenPortRateLimitGetNext",
        &wrCpssDxChBrgGenPortRateLimitExtGet,
        2, 0},

    {"cpssDxChBrgGenPortRateLimitTcpSynSet",
        &wrCpssDxChBrgGenPortRateLimitTcpSynSet,
        3, 0},

    {"cpssDxChBrgGenPortRateLimitTcpSynGet",
        &wrCpssDxChBrgGenPortRateLimitTcpSynGet,
        2, 0},

    {"cpssDxChBrgGenUdpBcDestPortCfgSet",
        &wrCpssDxChBrgGenUdpBcDestPortCfgSet,
        5, 0},

    {"cpssDxChBrgGenUdpBcDestPortCfgGet",
        &wrCpssDxChBrgGenUdpBcDestPortCfgGet,
        2, 0},

    {"cpssDxChBrgGenUdpBcDestPortCfgInvalidate",
        &wrCpssDxChBrgGenUdpBcDestPortCfgInvalidate,
        2, 0},

    {"cpssDxChBrgGenBpduTrapEnableSet",
        &wrCpssDxChBrgGenBpduTrapEnableSet,
        2, 0},

    {"cpssDxChBrgGenBpduTrapEnableGet",
        &wrCpssDxChBrgGenBpduTrapEnableGet,
        1, 0},

    {"cpssDxChBrgGenArpTrapEnable",
        &wrCpssDxChBrgGenArpTrapEnable,
        3, 0},

    {"cpssDxChBrgGenArpTrapEnableGet",
        &wrCpssDxChBrgGenArpTrapEnableGet,
        2, 0},

    {"cpssDxChBrgGenDropIpMcModeSet",
         &wrCpssDxChBrgGenDropIpMcModeSet,
         2, 0},
    {"cpssDxChBrgGenDropIpMcModeGet",
         &wrCpssDxChBrgGenDropIpMcModeGet,
         1, 0},
    {"cpssDxChBrgGenDropNonIpMcModeSet",
         &wrCpssDxChBrgGenDropNonIpMcModeSet,
         2, 0},
    {"cpssDxChBrgGenDropNonIpMcModeGet",
         &wrCpssDxChBrgGenDropNonIpMcModeGet,
         1, 0},
    {"cpssDxChBrgGenCfiRelayEnableSet",
         &wrCpssDxChBrgGenCfiRelayEnableSet,
         2, 0},
    {"cpssDxChBrgGenCfiRelayEnableGet",
         &wrCpssDxChBrgGenCfiRelayEnableGet,
         1, 0},

    {"cpssDxChBrgGenPortOversizUntagPktFilterEnableSet",
         &wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet,
         3, 0},
    {"cpssDxChBrgGenPortOversizUntagPktFilterEnableGet",
         &wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet,
         2, 0},

    {"cpssDxChBrgGenRateLimitSpeedCfgEnableSet",
         &wrCpssDxChBrgGenRateLimitSpeedCfgEnableSet,
         2, 0},

    {"cpssDxChBrgGenRateLimitSpeedCfgEnableGet",
         &wrCpssDxChBrgGenRateLimitSpeedCfgEnableGet,
         1, 0},

    {"cpssDxChBrgGenPortRateLimitSpeedGranularitySet",
         &wrCpssDxChBrgGenPortRateLimitSpeedGranularitySet,
         3, 0},

    {"cpssDxChBrgGenPortRateLimitSpeedGranularityGet",
         &wrCpssDxChBrgGenPortRateLimitSpeedGranularityGet,
         2, 0},

    {"cpssDxChBrgGenExcludeLLMcFromUnregMcFilterEnSet",
         &wrCpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet,
         3, 0},

    {"cpssDxChBrgGenExcludeLLMcFromUnregMcFilterEnGet",
         &wrCpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet,
         2, 0},

    {"cpssDxChBrgGenPortIeeeReservedMcastLearnEnSet",
         &wrCpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet,
         3, 0},

    {"cpssDxChBrgGenPortIeeeReservedMcastLearnEnGet",
         &wrCpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet,
         2, 0},

    {"cpssDxChBrgGenPortRateLimitDropCntrEnableSet",
         &wrCpssDxChBrgGenPortRateLimitDropCntrEnableSet,
         3, 0},

    {"cpssDxChBrgGenPortRateLimitDropCntrEnableGet",
         &wrCpssDxChBrgGenPortRateLimitDropCntrEnableGet,
         2, 0},

    {"cpssDxChBrgGenRateLimitDropCntrSet",
         &wrCpssDxChBrgGenRateLimitDropCntrSet,
         3, 0},

    {"cpssDxChBrgGenRateLimitDropCntrGet",
         &wrCpssDxChBrgGenRateLimitDropCntrGet,
         1, 0},

    {"cpssDxChBrgGenPortArpMacSaMismatchDropEnable",
         &wrCpssDxChBrgGenPortArpMacSaMismatchDropEnable,
         3, 0},

    {"cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet",
         &wrCpssDxChBrgGenPortArpMacSaMismatchDropEnableGet,
         2, 0},

    {"cpssDxChBrgGenIngressPortUnknownUcFilterDaCmdSet",
         &wrCpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet,
         3, 0},

    {"cpssDxChBrgGenIngressPortUnknownUcFilterDaCmdGet",
         &wrCpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet,
         2, 0},

    {"cpssDxChBrgGenIngressPortUnregisMcFilterDaCmdSet",
         &wrCpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet,
         3, 0},

    {"cpssDxChBrgGenIngressPortUnregisMcFilterDaCmdGet",
         &wrCpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet,
         2, 0},

    {"cpssDxChBrgGenIngressPortBcFilterDaCommandSet",
         &wrCpssDxChBrgGenIngressPortBcFilterDaCommandSet,
         3, 0},

    {"cpssDxChBrgGenIngressPortBcFilterDaCommandGet",
         &wrCpssDxChBrgGenIngressPortBcFilterDaCommandGet,
         2, 0},

    {"cpssDxChBrgGenPortIpControlTrapEnableSet",
         &wrCpssDxChBrgGenPortIpControlTrapEnableSet,
         3, 0},

    {"cpssDxChBrgGenPortIpControlTrapEnableGet",
         &wrCpssDxChBrgGenPortIpControlTrapEnableGet,
         2, 0},

    {"cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet",
         &wrCpssDxChBrgGenPortBcUdpTrapMirrorEnableSet,
         3, 0},

    {"cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet",
         &wrCpssDxChBrgGenPortBcUdpTrapMirrorEnableGet,
         2, 0},

    {"cpssDxChBrgGenPortOversizeUntaggedPckFilterCmdSet",
        &wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet,
        2, 0},

    {"cpssDxChBrgGenPortOversizeUntaggedPckFilterCmdGet",
        &wrCpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet,
        1, 0},

    {"cpssDxChBrgGenBypassModeSet",
        &wrCpssDxChBrgGenBypassModeSet,
        2, 0},

    {"cpssDxChBrgGenBypassModeGet",
        &wrCpssDxChBrgGenBypassModeGet,
        1, 0},

    {"cpssDxChBrgGenMtuConfigSet",
        &wrCpssDxChBrgGenMtuConfigSet,
        5, 0},

    {"cpssDxChBrgGenMtuConfigGet",
        &wrCpssDxChBrgGenMtuConfigGet,
        1, 0},

    {"cpssDxChBrgGenMtuExceedCntrGet",
        &wrCpssDxChBrgGenMtuExceedCntrGet,
        1, 0},

    {"cpssDxChBrgGenMtuProfileSet",
        &wrCpssDxChBrgGenMtuProfileSet,
        3, 0},

    {"cpssDxChBrgGenMtuProfileGet",
        &wrCpssDxChBrgGenMtuProfileGet,
        2, 0},

    {"cpssDxChBrgGenMtuPortProfileIdxSet",
        &wrCpssDxChBrgGenMtuPortProfileIdxSet,
        3, 0},

    {"cpssDxChBrgGenMtuPortProfileIdxGet",
        &wrCpssDxChBrgGenMtuPortProfileIdxGet,
        2, 0},

    {"cpssDxChBrgGenIpmClassificationModeSet",
        &wrCpssDxChBrgGenIpmClassificationModeSet,
        2, 0},

    {"cpssDxChBrgGenIpmClassificationModeGet",
        &wrCpssDxChBrgGenIpmClassificationModeGet,
        1, 0},
 };

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChBridgeGen function
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
GT_STATUS cmdLibInitCpssDxChBridgeGen
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


