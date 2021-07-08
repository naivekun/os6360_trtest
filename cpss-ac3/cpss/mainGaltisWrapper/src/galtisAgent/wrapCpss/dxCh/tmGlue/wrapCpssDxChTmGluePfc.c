/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file wrapCpssDxChTmGluePfc.c
*
* @brief Wrapper functions for TM PFC API.
*
* @version   3
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>

static GT_U32 tcIndex;

/**
* @internal wrCpssDxChTmGluePfcTmTcPort2CNodeSet function
* @endinternal
*
* @brief   Sets C node value for given pfc port and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To enable PFC response by the TM, the Timer values of the received PFC frames must be
*       redirected to the TM unit (using cpssDxChTmGluePfcResponseModeSet).
*       C-nodes must be allocated so that a C-node serves a specific (port, priority).
*       For example, to support 8 PFC priorities on a specific port, 8 C-Nodes must be associated with that port.
*       Queues that serve a specific port and a PFC priority must be associated with the respective C-node.
*       64 ingress ports (pfcPort) are available to redirect the Timer values of the received
*       PFC frames to the TM unit, this API is used along with cpssDxChTmGluePfcPortMappingSet
*       to map physical port to its pfcPort.
*       Specific CNode must have one instance in TcPortToCnode Table,
*       CNodes are switched by the API to have such behaviour.
*
*/
static CMD_STATUS wrCpssDxChTmGluePfcTmTcPort2CNodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          tc;
    GT_U32                          cNodeValue;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    tc = (GT_U8)inFields[0];
    cNodeValue = (GT_BOOL)inFields[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChTmGluePfcTmTcPort2CNodeSet(devNum, portNum, tc, cNodeValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGluePfcTmTcPort2CNodeGetEntry function
* @endinternal
*
* @brief   Gets C node for given traffic class and pfc port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, C node,
*                                       traffic class or port.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChTmGluePfcTmTcPort2CNodeGetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          cNodeValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (tcIndex >= CPSS_TC_RANGE_CNS)
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChTmGluePfcTmTcPort2CNodeGet(devNum, portNum, tcIndex, &cNodeValue);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = tcIndex;
    inFields[1] = cNodeValue;

    fieldOutput("%d%d", inFields[0], inFields[1]);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;

}

/***************************************************************************************************/
static CMD_STATUS wrCpssDxChTmGluePfcTmTcPort2CNodeGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    tcIndex = 0;

    return wrCpssDxChTmGluePfcTmTcPort2CNodeGetEntry(inArgs, inFields,
                                                     numFields, outArgs);
}


/***************************************************************************************************/
static CMD_STATUS wrCpssDxChTmGluePfcTmTcPort2CNodeGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    tcIndex++;

    return wrCpssDxChTmGluePfcTmTcPort2CNodeGetEntry(inArgs, inFields,
                                                     numFields, outArgs);
}

/**
* @internal wrCpssDxChTmGluePfcResponseModeSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) support with Traffic Manager (TM).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or PFC response mode.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Determine whether Timer values of the received PFC frames are redirected to be responded
*       by the TM unit.(used along with cpssDxChTmGluePfcTmTcPort2CNodeSet)
*       for Bobcat2 configuration is applied at Device level (portNum is ignored).
*       for Caelum configuration is applied at portNum only. (used along with cpssDxChTmGluePfcPortMappingSet)
*
*/
static CMD_STATUS wrCpssDxChTmGluePfcResponseModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    portNum;
    CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U32)inArgs[1];
    responseMode    = (CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChTmGluePfcResponseModeSet(devNum, portNum, responseMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGluePfcResponseModeGet function
* @endinternal
*
* @brief   Get PFC (Priority Flow Control) for TM support status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number number.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_STATE             - on wrong response mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Get whether Timer values of the received PFC frames are redirected to be
*       responded by the TM unit.
*       for Bobcat2 configuration is retrived by Device level (portNum is ignored).
*       for Caelum configuration is retrieved by portNum.
*
*/
static CMD_STATUS wrCpssDxChTmGluePfcResponseModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    portNum;
    CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChTmGluePfcResponseModeGet(devNum, portNum, &responseMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", responseMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGluePfcPortMappingSet function
* @endinternal
*
* @brief   Map physical port to pfc port, used to map physical ports 0..255
*         to pfc ports 0..63.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 64 ingress ports (pfcPort) are available to redirect the Timer values of the received
*       PFC frames to the TM unit, this API is used along with cpssDxChTmGluePfcTmTcPort2CNodeSet
*       to map physical port to its pfcPort.
*
*/
static CMD_STATUS wrCpssDxChTmGluePfcPortMappingSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            pfcPortNum;
    GT_PHYSICAL_PORT_NUM            portNum;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inFields[0];
    pfcPortNum = (GT_PHYSICAL_PORT_NUM)inFields[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, pfcPortNum);
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChTmGluePfcPortMappingSet(devNum, portNum, pfcPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChTmGluePfcPortMappingGetEntry function
* @endinternal
*
* @brief   Get physical port to pfc port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChTmGluePfcPortMappingGetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL getFirst /* GT_TRUE - first, GT_FALSE - next */
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            pfcPortNum;
    static GT_PHYSICAL_PORT_NUM     portIndex = 0;
    static GT_U32                   numOfPorts = 0;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    if (getFirst == GT_TRUE)
    {
        portIndex   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
        numOfPorts  = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    }
    else
    {
        portIndex++;
        numOfPorts--;
    }

    if (numOfPorts == 0)
    {
        /* no more entries */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    if (portIndex > 255)
    {
        galtisOutput(outArgs, CMD_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portIndex);

    /* call cpss api function */
    result = cpssDxChTmGluePfcPortMappingGet(devNum, portIndex, &pfcPortNum);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput("%d%d", portIndex, pfcPortNum);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/***************************************************************************************************/
static CMD_STATUS wrCpssDxChTmGluePfcPortMappingGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChTmGluePfcPortMappingGetEntry(inArgs, inFields,
                                                  numFields, outArgs, GT_TRUE);
}


/***************************************************************************************************/
static CMD_STATUS wrCpssDxChTmGluePfcPortMappingGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChTmGluePfcPortMappingGetEntry(inArgs, inFields,
                                                  numFields, outArgs, GT_FALSE);
}


/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChTmGluePfcTmTcPort2CNodeSet",
        &wrCpssDxChTmGluePfcTmTcPort2CNodeSet,
        2, 2},

    {"cpssDxChTmGluePfcTmTcPort2CNodeGetFirst",
        &wrCpssDxChTmGluePfcTmTcPort2CNodeGetFirst,
        2, 0},

    {"cpssDxChTmGluePfcTmTcPort2CNodeGetNext",
        &wrCpssDxChTmGluePfcTmTcPort2CNodeGetNext,
        2, 0},

    {"cpssDxChTmGluePfcResponseModeSet",
        &wrCpssDxChTmGluePfcResponseModeSet,
        3, 0},

    {"cpssDxChTmGluePfcResponseModeGet",
        &wrCpssDxChTmGluePfcResponseModeGet,
        2, 0},

    {"cpssDxChTmGluePfcPortMappingSet",
        &wrCpssDxChTmGluePfcPortMappingSet,
        1, 2},

    {"cpssDxChTmGluePfcPortMappingGetFirst",
        &wrCpssDxChTmGluePfcPortMappingGetFirst,
        3, 0},

    {"cpssDxChTmGluePfcPortMappingGetNext",
        &wrCpssDxChTmGluePfcPortMappingGetNext,
        3, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChTmGluePfc function
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
GT_STATUS cmdLibInitCpssDxChTmGluePfc
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

