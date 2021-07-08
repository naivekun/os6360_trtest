/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file wrapCpssDxChBridgeFdbRouting.c
*
* @brief Wrapper functions for cpssDxChBrgFdbRouting.c dxCh functions
*
* @version   5
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>

extern GT_STATUS appDemoBrgFdbIpv6UcUpdateAddrDataLinkedList
(
    IN  GT_U8           devNum,
    IN  GT_U32          addrIndex,
    IN  GT_U32          dataIndex,
    IN  GT_BOOL         deleteElement
);

extern GT_VOID appDemoBrgFdbIpv6AddrDataLinkedListDump
(
    GT_VOID
);

extern GT_STATUS appDemoBrgFdbIpv6UcFreeAddrDataLinkedList
(
    IN GT_U8  devNum
);

/**
* @internal wrCpssDxChBrgFdbRoutingPortIpUcEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingPortIpUcEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        dev;
    GT_PORT_NUM                  port;
    CPSS_IP_PROTOCOL_STACK_ENT   protocol;
    GT_BOOL                      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev      = (GT_U8)inArgs[0];
    port     = (GT_PORT_NUM)inArgs[1];
    protocol = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    enable   = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingPortIpUcEnableSet(dev, port, protocol, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingPortIpUcEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingPortIpUcEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        dev;
    GT_PORT_NUM                  port;
    CPSS_IP_PROTOCOL_STACK_ENT   protocol;
    GT_BOOL                      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev      = (GT_U8)inArgs[0];
    port     = (GT_PORT_NUM)inArgs[1];
    protocol = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingPortIpUcEnableGet(dev, port, protocol, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcRefreshEnableSet function
* @endinternal
*
* @brief   Enable/Disable refresh for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcRefreshEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcRefreshEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcRefreshEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable refresh status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcRefreshEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcRefreshEnableGet(devNum, &enable);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgFdbRoutingUcAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable aging for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcAgingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcAgingEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcAgingEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable aging status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcAgingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcAgingEnableGet(devNum, &enable);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcTransplantEnableSet function
* @endinternal
*
* @brief   Enable/Disable transplanting for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcTransplantEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcTransplantEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcTransplantEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable transplanting status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcTransplantEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcTransplantEnableGet(devNum, &enable);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcDeleteEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcDeleteEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcDeleteEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcDeleteEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable delete status for UC Routing entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcDeleteEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcDeleteEnableGet(devNum, &enable);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcAAandTAToCpuSet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting
*         mechanism. When an entry is aged out or is transplanted, a message can
*         be forwarded to the CPU.
*         This field enables/disables forwarding address aged out messages and
*         transplanted address messages to the Host CPU for UC route FDB entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcAAandTAToCpuSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingUcAAandTAToCpuGet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting mechanism
*         When an entry is aged out or is transplanted, a message can be forwarded
*         to the CPU.
*         This field get the enables/disable status of forwarding address aged out
*         messages and transplanted address messages to the Host CPU for UC route FDB entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingUcAAandTAToCpuGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(devNum, &enable);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgFdbRoutingNextHopPacketCmdSet function
* @endinternal
*
* @brief   Set Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingNextHopPacketCmdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     nhPacketCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    nhPacketCmd = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(devNum, nhPacketCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgFdbRoutingNextHopPacketCmdGet function
* @endinternal
*
* @brief   Get Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_VALUE             - wrong hw value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgFdbRoutingNextHopPacketCmdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     nhPacketCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(devNum, &nhPacketCmd);
    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", nhPacketCmd);

    return CMD_OK;
}

/**
* @internal wrAppDemoBrgFdbIpv6AddrDataLinkedListDump function
* @endinternal
*
* @brief   Dump FDB IPv6 UC Route entries linked list,
*         that holds AddressIndex to DataIndex mapping
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrAppDemoBrgFdbIpv6AddrDataLinkedListDump
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result=GT_OK;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

   /* call cpss api function */
    appDemoBrgFdbIpv6AddrDataLinkedListDump();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrAppDemoBrgFdbIpv6UcFreeAddrDataLinkedList function
* @endinternal
*
* @brief   Free FDB IPv6 UC Route entries linked list,
*         that holds AddressIndex to DataIndex mapping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Only elements that were deleted in flush process will be deleted
*       from the Linked List. If only part of the IPV6 UC entry was deleted
*       (address entry or data entry) then the Linked list will be updated
*       with value PRV_APPDEMO_INVALID_FDB_INDEX_VALUE_CNS indicating the
*       entry is not valid
*
*/
static CMD_STATUS wrAppDemoBrgFdbIpv6UcFreeAddrDataLinkedList
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = appDemoBrgFdbIpv6UcFreeAddrDataLinkedList(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrAppDemoBrgFdbIpv6UcUpdateAddrDataLinkedList function
* @endinternal
*
* @brief   Update FDB IPv6 UC Route entries linked list,
*         that holds AddressIndex to DataIndex mapping
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - on allocation fail
* @retval GT_FAIL                  - otherwise.
*
* @note adding a mapping of addrIndex and dataIndex to the Linked List must be
*       of valid entries that were added to HW.
*       if the address entry or the data entry is not added to HW this function will fail.
*       the Linked List should reflect the HW state
*
*/
static CMD_STATUS wrAppDemoBrgFdbIpv6UcUpdateAddrDataLinkedList
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  addrIndex;
    GT_U32                  dataIndex;
    GT_BOOL                 deleteElement;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    addrIndex = (GT_U32)inArgs[1];
    dataIndex = (GT_U32)inArgs[2];
    deleteElement = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = appDemoBrgFdbIpv6UcUpdateAddrDataLinkedList(devNum,addrIndex, dataIndex, deleteElement);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChBrgFdbRoutingPortIpUcEnableSet",
        &wrCpssDxChBrgFdbRoutingPortIpUcEnableSet,
        4, 0},

    {"cpssDxChBrgFdbRoutingPortIpUcEnableGet",
        &wrCpssDxChBrgFdbRoutingPortIpUcEnableGet,
        3, 0},

    {"cpssDxChBrgFdbRoutingUcRefreshEnableSet",
        &wrCpssDxChBrgFdbRoutingUcRefreshEnableSet,
        2, 0},

    {"cpssDxChBrgFdbRoutingUcRefreshEnableGet",
        &wrCpssDxChBrgFdbRoutingUcRefreshEnableGet,
        1, 0},

    {"cpssDxChBrgFdbRoutingUcAgingEnableSet",
        &wrCpssDxChBrgFdbRoutingUcAgingEnableSet,
        2, 0},

    {"cpssDxChBrgFdbRoutingUcAgingEnableGet",
        &wrCpssDxChBrgFdbRoutingUcAgingEnableGet,
        1, 0},

    {"cpssDxChBrgFdbRoutingUcTransplantEnableSet",
        &wrCpssDxChBrgFdbRoutingUcTransplantEnableSet,
        2, 0},

    {"cpssDxChBrgFdbRoutingUcTransplantEnableGet",
        &wrCpssDxChBrgFdbRoutingUcTransplantEnableGet,
        1, 0},

    {"cpssDxChBrgFdbRoutingUcDeleteEnableSet",
        &wrCpssDxChBrgFdbRoutingUcDeleteEnableSet,
        2, 0},

    {"cpssDxChBrgFdbRoutingUcDeleteEnableGet",
        &wrCpssDxChBrgFdbRoutingUcDeleteEnableGet,
        1, 0},

    {"cpssDxChBrgFdbRoutingUcAAandTAToCpuSet",
        &wrCpssDxChBrgFdbRoutingUcAAandTAToCpuSet,
        2, 0},

    {"cpssDxChBrgFdbRoutingUcAAandTAToCpuGet",
        &wrCpssDxChBrgFdbRoutingUcAAandTAToCpuGet,
        1, 0},

    {"cpssDxChBrgFdbRoutingNextHopPacketCmdSet",
        &wrCpssDxChBrgFdbRoutingNextHopPacketCmdSet,
        2, 0},

    {"cpssDxChBrgFdbRoutingNextHopPacketCmdGet",
        &wrCpssDxChBrgFdbRoutingNextHopPacketCmdGet,
        1, 0},

    {"appDemoDxChBrgFdbRouteIpv6UcAddrDataIndexDBDump",
        &wrAppDemoBrgFdbIpv6AddrDataLinkedListDump,
        0, 0},

     {"appDemoDxChBrgFdbRouteIpv6UcFreeAddrDataLinkedList",
        &wrAppDemoBrgFdbIpv6UcFreeAddrDataLinkedList,
        1, 0},

      {"appDemoDxChBrgFdbRouteIpv6UcUpdateAddrDataIndexDB",
        &wrAppDemoBrgFdbIpv6UcUpdateAddrDataLinkedList,
        4, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssDxChBridgeFdbRouting function
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
GT_STATUS cmdLibInitCpssDxChBridgeFdbRouting
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}




