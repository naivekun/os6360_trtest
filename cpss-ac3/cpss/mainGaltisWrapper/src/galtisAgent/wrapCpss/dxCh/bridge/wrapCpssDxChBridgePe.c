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
* wrapBridgePeCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for BridgePe cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPe.h>


/**
* @internal wrCpssDxChBrgPeEnableSet function
* @endinternal
*
* @brief   Enables globally Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPeEnableSet
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
    result = cpssDxChBrgPeEnableSet(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPeEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) global enabling status per port.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPeEnableGet
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
    result = cpssDxChBrgPeEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPePortEnableSet function
* @endinternal
*
* @brief   Enables per port Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPePortEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM          portNum;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, portNum);

    /* call cpss api function */
    result = cpssDxChBrgPePortEnableSet(dev, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPePortEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) enabling status per port.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPePortEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgPePortEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgPePortPcidMcFilterEnableSet function
* @endinternal
*
* @brief   Configures <port,PCID> pair for downsteam multicast source filtering.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_OUT_OF_RANGE          - PCID value out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPePortPcidMcFilterEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM          portNum;
    GT_U32                        pcid;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    pcid = (GT_U32)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, portNum);

    /* call cpss api function */
    result = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPePortPcidMcFilterEnableGet function
* @endinternal
*
* @brief   Gets downsteam multicast source filtering enabling status for <port,PCID> pair.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_OUT_OF_RANGE          - PCID value out of range
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPePortPcidMcFilterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  pcid;
    GT_BOOL                 enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    pcid = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgPePortPcidMcFilterEnableGet(devNum, portNum, pcid, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet function
* @endinternal
*
* @brief   Associate the cascade port (not DSA cascade port) with a trunkId.
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum , trunkId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         dev;
    GT_PHYSICAL_PORT_NUM          portNum;
    GT_BOOL                  enable;
    GT_TRUNK_ID              trunkId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    trunkId = (GT_TRUNK_ID)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, portNum);

    /* call cpss api function */
    result = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable, trunkId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet function
* @endinternal
*
* @brief   Get the associate trunkId with the cascade port (not DSA cascade port) .
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - inconsistency in HW regarding value of the 'associated trunkId'
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                  enable;
    GT_TRUNK_ID              trunkId;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(devNum, portNum, &enable , &trunkId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable,trunkId);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChBrgPeEnableSet",
        &wrCpssDxChBrgPeEnableSet,
        2, 0},

    {"cpssDxChBrgPeEnableGet",
        &wrCpssDxChBrgPeEnableGet,
        1, 0},

    {"cpssDxChBrgPePortEnableSet",
        &wrCpssDxChBrgPePortEnableSet,
        3, 0},

    {"cpssDxChBrgPePortEnableGet",
        &wrCpssDxChBrgPePortEnableGet,
        2, 0},

    {"cpssDxChBrgPePortPcidMcFilterEnableSet",
        &wrCpssDxChBrgPePortPcidMcFilterEnableSet,
        4, 0},

    {"cpssDxChBrgPePortPcidMcFilterEnableGet",
        &wrCpssDxChBrgPePortPcidMcFilterEnableGet,
        3, 0},
    /* remove prefix "cpssDxChBrg" due to GALTIS DB limitations */
    {"PeCascadePortAssociateToUplinkTrunkEnableSet",
        &wrCpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet,
        4, 0},

    {"PeCascadePortAssociateToUplinkTrunkEnableGet",
        &wrCpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet,
        2, 0},

 };

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChBridgePe function
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
GT_STATUS cmdLibInitCpssDxChBridgePe
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


