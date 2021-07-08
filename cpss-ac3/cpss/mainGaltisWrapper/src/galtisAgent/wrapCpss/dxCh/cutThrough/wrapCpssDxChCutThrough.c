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
* @file wrapCpssDxChCutThrough.c
*
* @brief Wrapper functions for Cut Through cpss.dxCh functions
*
* @version   9
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>

/**
* @internal wrCpssDxChCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssDxChCutThroughUpEnableSet.
*
*/
static CMD_STATUS wrCpssDxChCutThroughPortEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL   enable;
    GT_BOOL   untaggedEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    untaggedEnable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCutThroughPortEnableSet(devNum, portNum, enable, untaggedEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughPortEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode on the specified port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughPortEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;
    GT_BOOL untaggedEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCutThroughPortEnableGet(devNum, portNum,
                                             &enable, &untaggedEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, untaggedEnable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssDxChCutThroughPortEnableSet for it.
*
*/
static CMD_STATUS wrCpssDxChCutThroughUpEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U8     up;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    up = (GT_U8)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChCutThroughUpEnableSet(devNum, up, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughUpEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode for tagged packets
*         with the specified UP.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or up
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughUpEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U8     up;
    GT_BOOL   enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    up = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCutThroughUpEnableGet(devNum, up, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughVlanEthertypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    etherType0;
    GT_U32    etherType1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    etherType0 = (GT_U32)inArgs[1];
    etherType1 = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChCutThroughVlanEthertypeSet(devNum, etherType0, etherType1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughVlanEthertypeGet function
* @endinternal
*
* @brief   Get VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughVlanEthertypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    etherType0;
    GT_U32    etherType1;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCutThroughVlanEthertypeGet(devNum, &etherType0, &etherType1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", etherType0, etherType1);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughMinimalPacketSizeSet function
* @endinternal
*
* @brief   Set minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When using cut-through to 1G port, and bypassing
*       of Router And Ingress Policer engines is disabled,
*       the minimal packet size should be 512 bytes.
*       When bypassing of Router And Ingress Policer engines is enabled,
*       the minimal cut-through packet size should be:
*       - for 10G or faster ports - at least 257 bytes.
*       - for ports slower than 10 G - at least 513 bytes
*
*/
static CMD_STATUS wrCpssDxChCutThroughMinimalPacketSizeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    size;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    size = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCutThroughMinimalPacketSizeSet(devNum, size);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughMinimalPacketSizeGet function
* @endinternal
*
* @brief   Get minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughMinimalPacketSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    size;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCutThroughMinimalPacketSizeGet(devNum, &size);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", size);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughMemoryRateLimitSet function
* @endinternal
*
* @brief   Set rate limiting of read operations from the memory
*         per target port in Cut Through mode according to the port speed.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or portSpeed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Rate limit is recommended be enabled
*       on all egress ports for cut-through traffic.
*       Rate limit to the CPU port should be configured as a 1G port.
*
*/
static CMD_STATUS wrCpssDxChCutThroughMemoryRateLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL   enable;
    CPSS_PORT_SPEED_ENT  portSpeed;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    portSpeed = (CPSS_PORT_SPEED_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCutThroughMemoryRateLimitSet(devNum, portNum, enable, portSpeed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughMemoryRateLimitGet function
* @endinternal
*
* @brief   Get rate limiting of read operations from the memory
*         per target port in Cut Through mode.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughMemoryRateLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;
    CPSS_PORT_SPEED_ENT  portSpeed;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCutThroughMemoryRateLimitGet(devNum, portNum,
                                             &enable, &portSpeed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, portSpeed);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughBypassRouterAndPolicerEnableSet function
* @endinternal
*
* @brief   Enable / Disable bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughBypassRouterAndPolicerEnableSet
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
    result = cpssDxChCutThroughBypassRouterAndPolicerEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughBypassRouterAndPolicerEnableGet function
* @endinternal
*
* @brief   Get mode of bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughBypassRouterAndPolicerEnableGet
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


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCutThroughBypassRouterAndPolicerEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughBypassModeSet function
* @endinternal
*
* @brief   Set Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, Bypass Ingress PCL mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughBypassModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC  bypassMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* Fill bypassMode with current values */
    result = cpssDxChCutThroughBypassModeGet(devNum, &bypassMode);
    if(GT_OK != result)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    bypassMode.bypassRouter = (GT_BOOL)inArgs[1];
    bypassMode.bypassIngressPolicerStage0 = (GT_BOOL)inArgs[2];
    bypassMode.bypassIngressPolicerStage1 = (GT_BOOL)inArgs[3];
    bypassMode.bypassEgressPolicer = (GT_BOOL)inArgs[4];
    bypassMode.bypassEgressPcl = (GT_BOOL)inArgs[5];
    bypassMode.bypassIngressPcl = (CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT)inArgs[6];

    /* call cpss api function */
    result = cpssDxChCutThroughBypassModeSet(devNum, &bypassMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughBypassModeSet_1 function
* @endinternal
*
* @brief   Set Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, Bypass Ingress PCL mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughBypassModeSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC  bypassMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    bypassMode.bypassRouter = (GT_BOOL)inArgs[1];
    bypassMode.bypassIngressPolicerStage0 = (GT_BOOL)inArgs[2];
    bypassMode.bypassIngressPolicerStage1 = (GT_BOOL)inArgs[3];
    bypassMode.bypassEgressPolicer = (GT_BOOL)inArgs[4];
    bypassMode.bypassEgressPcl = (GT_BOOL)inArgs[5];
    bypassMode.bypassIngressPcl = (CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT)inArgs[6];
    bypassMode.bypassIngressOam = (GT_BOOL)inArgs[7];
    bypassMode.bypassEgressOam = (GT_BOOL)inArgs[8];
    bypassMode.bypassMll = (GT_BOOL)inArgs[9];

    /* call cpss api function */
    result = cpssDxChCutThroughBypassModeSet(devNum, &bypassMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughBypassModeGet function
* @endinternal
*
* @brief   Get Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughBypassModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC  bypassMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCutThroughBypassModeGet(devNum, &bypassMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d", bypassMode.bypassRouter,
                                        bypassMode.bypassIngressPolicerStage0,
                                        bypassMode.bypassIngressPolicerStage1,
                                        bypassMode.bypassEgressPolicer,
                                        bypassMode.bypassEgressPcl,
                                        bypassMode.bypassIngressPcl);

    return CMD_OK;

}

/**
* @internal wrCpssDxChCutThroughBypassModeGet_1 function
* @endinternal
*
* @brief   Get Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughBypassModeGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC  bypassMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCutThroughBypassModeGet(devNum, &bypassMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d",
                                    bypassMode.bypassRouter,
                                    bypassMode.bypassIngressPolicerStage0,
                                    bypassMode.bypassIngressPolicerStage1,
                                    bypassMode.bypassEgressPolicer,
                                    bypassMode.bypassEgressPcl,
                                    bypassMode.bypassIngressPcl,
                                    bypassMode.bypassIngressOam,
                                    bypassMode.bypassEgressOam,
                                    bypassMode.bypassMll);

    return CMD_OK;

}

/**
* @internal wrCpssDxChCutThroughPortGroupMaxBuffersLimitSet function
* @endinternal
*
* @brief   Set max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong maxSharedBuffersLimit
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
static CMD_STATUS wrCpssDxChCutThroughPortGroupMaxBuffersLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS          result;
    GT_U8              devNum;
    GT_PORT_GROUPS_BMP portGroupsBmp;
    GT_BOOL            buffersLimitEnable;
    GT_U32             buffersLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum             = (GT_U8)inArgs[0];
    portGroupsBmp      = (GT_PORT_GROUPS_BMP)inArgs[1];
    buffersLimitEnable = (GT_BOOL)inArgs[2];
    buffersLimit       = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChCutThroughPortGroupMaxBuffersLimitSet(
        devNum, portGroupsBmp, buffersLimitEnable, buffersLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCutThroughPortGroupMaxBuffersLimitGet function
* @endinternal
*
* @brief   Get max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
static CMD_STATUS wrCpssDxChCutThroughPortGroupMaxBuffersLimitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS          result;
    GT_U8              devNum;
    GT_PORT_GROUPS_BMP portGroupsBmp;
    GT_BOOL            buffersLimitEnable;
    GT_U32             buffersLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum             = (GT_U8)inArgs[0];
    portGroupsBmp      = (GT_PORT_GROUPS_BMP)inArgs[1];
    buffersLimitEnable = 0;
    buffersLimit       = 0;

    /* call cpss api function */
    result = cpssDxChCutThroughPortGroupMaxBuffersLimitGet(
        devNum, portGroupsBmp, &buffersLimitEnable, &buffersLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", buffersLimitEnable, buffersLimit);

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughUdeCfgSet function
* @endinternal
*
* @brief   Set Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[in] udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[in] udeCutThroughEnable         - GT_TRUE/GT_FALSE enable/disable Cut Through mode for UDE packets.
* @param[in] udeByteCount                - default CT Byte Count for UDE packets.
*                                          (APPLICABLE RANGES: 0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range udeByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChCutThroughUdeCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      udeIndex;
    GT_BOOL     udeCutThroughEnable;
    GT_U32      udeByteCount;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum               = (GT_U8)inArgs[0];
    udeIndex             = (GT_U32)inArgs[1];
    udeCutThroughEnable  = (GT_BOOL)inArgs[2];
    udeByteCount         = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChCutThroughUdeCfgSet(
        devNum, udeIndex, udeCutThroughEnable, udeByteCount);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughUdeCfgGet function
* @endinternal
*
* @brief   Get Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                      - device number
* @param[in]  udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[out] udeCutThroughEnablePtr      - pointer to GT_TRUE/GT_FALSE
*                                           enable/disable Cut Through mode for UDE packets.
* @param[out] udeByteCountPtr             - pointer to default CT Byte Count for UDE packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughUdeCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      udeIndex;
    GT_BOOL     udeCutThroughEnable;
    GT_U32      udeByteCount;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum               = (GT_U8)inArgs[0];
    udeIndex             = (GT_U32)inArgs[1];
    udeCutThroughEnable  = GT_FALSE;
    udeByteCount         = 0;

    /* call cpss api function */
    result = cpssDxChCutThroughUdeCfgGet(
        devNum, udeIndex, &udeCutThroughEnable, &udeByteCount);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", udeCutThroughEnable, udeByteCount);

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughPassengerParsingModeSet function
* @endinternal
*
* @brief   Set Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] parseMode              - passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChCutThroughPassengerParsingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                          result;
    GT_U8                                              devNum;
    CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    parsingMode = (CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCutThroughPassengerParsingModeSet(
        devNum, parsingMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughPassengerParsingModeGet function
* @endinternal
*
* @brief   Get Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                 - device number
* @param[out] parseModePtr           - pointer to passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChCutThroughPassengerParsingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                          result;
    GT_U8                                              devNum;
    CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCutThroughPassengerParsingModeGet(
        devNum, &parsingMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", parsingMode);

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughByteCountExtractFailsCounterGet function
* @endinternal
*
* @brief   Get counter of fails extracting CT Packet Byte Count by packet header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum    - device number
* @param[out] countPtr  - pointer to count of packet Byte Count extracting fails.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughByteCountExtractFailsCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32     counter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    counter     = 0;

    /* call cpss api function */
    result = cpssDxChCutThroughByteCountExtractFailsCounterGet(
        devNum, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counter);

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet function
* @endinternal
*
* @brief   Set configuration Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum - device number
* @param[in] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChCutThroughPacketHdrIntegrityCheckCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                                 result;
    GT_U8                                                     devNum;
    CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  cfg;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                              = (GT_U8)inArgs[0];
    cfg.minByteCountBoundary            = (GT_U32)inArgs[1];
    cfg.maxByteCountBoundary            = (GT_U32)inArgs[2];
    cfg.enableByteCountBoundariesCheck  = (GT_BOOL)inArgs[3];
    cfg.enableIpv4HdrCheckByChecksum    = (GT_BOOL)inArgs[4];
    cfg.hdrIntergrityExceptionPktCmd    = (CPSS_PACKET_CMD_ENT)inArgs[5];
    cfg.hdrIntergrityExceptionCpuCode   = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[6];

    /* call cpss api function */
    result = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(
        devNum, &cfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet function
* @endinternal
*
* @brief   Get configuration of Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum  - device number
* @param[out] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughPacketHdrIntegrityCheckCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                                 result;
    GT_U8                                                     devNum;
    CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  cfg;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                              = (GT_U8)inArgs[0];
    cpssOsMemSet(&cfg, 0, sizeof(cfg));

    /* call cpss api function */
    result = cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet(
        devNum, &cfg);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d%d%d",
        cfg.minByteCountBoundary, cfg.maxByteCountBoundary,
        cfg.enableByteCountBoundariesCheck, cfg.enableIpv4HdrCheckByChecksum,
        cfg.hdrIntergrityExceptionPktCmd, cfg.hdrIntergrityExceptionCpuCode);

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughPortByteCountUpdateSet function
* @endinternal
*
* @brief   Set configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] portNum                   - physical port number including CPU port.
* @param[in] toSubtractOrToAdd         - GT_TRUE - to subtract, GT_FALSE to add.
* @param[in] subtractedOrAddedValue    - value to subtract or to add to packet length
*                                        (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on out of range subtractedOrAddedValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChCutThroughPortByteCountUpdateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_U8                 devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               toSubtractOrToAdd;
    GT_U32                subtractedOrAddedValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                 = (GT_U8)inArgs[0];
    portNum                = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    toSubtractOrToAdd      = (GT_BOOL)inArgs[2];
    subtractedOrAddedValue = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCutThroughPortByteCountUpdateSet(
        devNum, portNum, toSubtractOrToAdd, subtractedOrAddedValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal cpssDxChCutThroughPortByteCountUpdateGet function
* @endinternal
*
* @brief   Get configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                    - device number
* @param[in]  portNum                   - physical port number including CPU port.
* @param[out] toSubtractOrToAddPtr      - pointer to GT_TRUE - to subtract, GT_FALSE to add.
* @param[out] subtractedOrAddedValuePtr - pointer to value to subtract or to add to packet length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCutThroughPortByteCountUpdateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS             result;
    GT_U8                 devNum;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_BOOL               toSubtractOrToAdd;
    GT_U32                subtractedOrAddedValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                 = (GT_U8)inArgs[0];
    portNum                = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    toSubtractOrToAdd      = GT_FALSE;
    subtractedOrAddedValue = 0;

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCutThroughPortByteCountUpdateGet(
        devNum, portNum, &toSubtractOrToAdd, &subtractedOrAddedValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", toSubtractOrToAdd, subtractedOrAddedValue);

    return CMD_OK;
}

/**** database initialization **************************************/


static CMD_COMMAND dbCommands[] =
{
        {"cpssDxChCutThroughPortEnableSet",
         &wrCpssDxChCutThroughPortEnableSet,
         4, 0},
        {"cpssDxChCutThroughPortEnableGet",
         &wrCpssDxChCutThroughPortEnableGet,
         2, 0},
        {"cpssDxChCutThroughUpEnableSet",
         &wrCpssDxChCutThroughUpEnableSet,
         3, 0},
        {"cpssDxChCutThroughUpEnableGet",
         &wrCpssDxChCutThroughUpEnableGet,
         2, 0},
        {"cpssDxChCutThroughVlanEthertypeSet",
         &wrCpssDxChCutThroughVlanEthertypeSet,
         3, 0},
        {"cpssDxChCutThroughVlanEthertypeGet",
         &wrCpssDxChCutThroughVlanEthertypeGet,
         1, 0},
        {"cpssDxChCutThroughMinimalPacketSizeSet",
         &wrCpssDxChCutThroughMinimalPacketSizeSet,
         2, 0},
        {"cpssDxChCutThroughMinimalPacketSizeGet",
         &wrCpssDxChCutThroughMinimalPacketSizeGet,
         1, 0},
        {"cpssDxChCutThroughMemoryRateLimitSet",
         &wrCpssDxChCutThroughMemoryRateLimitSet,
         4, 0},
        {"cpssDxChCutThroughMemoryRateLimitGet",
         &wrCpssDxChCutThroughMemoryRateLimitGet,
         2, 0},
        {"cpssDxChCutThroughBypassRouterAndPolicerEnableSet",
         &wrCpssDxChCutThroughBypassRouterAndPolicerEnableSet,
         2, 0},
        {"cpssDxChCutThroughBypassRouterAndPolicerEnableGet",
         &wrCpssDxChCutThroughBypassRouterAndPolicerEnableGet,
         1, 0},
        {"cpssDxChCutThroughBypassModeSet",
         &wrCpssDxChCutThroughBypassModeSet,
         7, 0},
        {"cpssDxChCutThroughBypassModeSet_1",
         &wrCpssDxChCutThroughBypassModeSet_1,
         10, 0},
        {"cpssDxChCutThroughBypassModeGet",
         &wrCpssDxChCutThroughBypassModeGet,
         1, 0},
        {"cpssDxChCutThroughBypassModeGet_1",
         &wrCpssDxChCutThroughBypassModeGet_1,
         1, 0},
        {"cpssDxChCutThroughPortGroupMaxBuffersLimitSet",
         &wrCpssDxChCutThroughPortGroupMaxBuffersLimitSet,
         4, 0},
        {"cpssDxChCutThroughPortGroupMaxBuffersLimitGet",
         &wrCpssDxChCutThroughPortGroupMaxBuffersLimitGet,
         2, 0},
        {"cpssDxChCutThroughUdeCfgSet",
         &wrCpssDxChCutThroughUdeCfgSet,
         4, 0},
        {"cpssDxChCutThroughUdeCfgGet",
         &wrCpssDxChCutThroughUdeCfgGet,
         2, 0},
        {"cpssDxChCutThroughPassengerParsingModeSet",
         &wrCpssDxChCutThroughPassengerParsingModeSet,
         2, 0},
        {"cpssDxChCutThroughPassengerParsingModeGet",
         &wrCpssDxChCutThroughPassengerParsingModeGet,
         1, 0},
        {"cpssDxChCutThroughByteCountExtractFailsCounterGet",
         &wrCpssDxChCutThroughByteCountExtractFailsCounterGet,
         1, 0},
        {"cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet",
         &wrCpssDxChCutThroughPacketHdrIntegrityCheckCfgSet,
         7, 0},
        {"cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet",
         &wrCpssDxChCutThroughPacketHdrIntegrityCheckCfgGet,
         1, 0},
        {"cpssDxChCutThroughPortByteCountUpdateSet",
         &wrCpssDxChCutThroughPortByteCountUpdateSet,
         4, 0},
        {"cpssDxChCutThroughPortByteCountUpdateGet",
         &wrCpssDxChCutThroughPortByteCountUpdateGet,
         2, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChCutThrough function
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
GT_STATUS cmdLibInitCpssDxChCutThrough
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


