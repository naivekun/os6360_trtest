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
* @file wrapCpssPxCos.c
*
* @brief Wrapper functions for
* cpss/Px/cos/cpssPxCos.h API's
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
#include <cpss/px/cos/cpssPxCos.h>

static GT_U32 currentIndex;
static GT_U32 lastIndex;

CMD_STATUS wrCpssPxCosFormatEntryTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_SW_DEV_NUM                devNum;
    CPSS_PX_PACKET_TYPE          packetType;
    CPSS_PX_COS_FORMAT_ENTRY_STC cosFormatEntry = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    packetType                              = (CPSS_PX_PACKET_TYPE)inArgs[1];
    /* map input fields to locals */
    cosFormatEntry.cosMode                                 = (CPSS_PX_COS_MODE_ENT)inFields[0];
    cosFormatEntry.cosByteOffset                           = (GT_U32)inFields[1];
    cosFormatEntry.cosBitOffset                            = (GT_U32)inFields[2];
    cosFormatEntry.cosNumOfBits                            = (GT_U32)inFields[3];
    cosFormatEntry.cosAttributes.trafficClass              = (GT_U32)inFields[4];
    cosFormatEntry.cosAttributes.dropPrecedence            = (CPSS_DP_LEVEL_ENT)inFields[5];
    cosFormatEntry.cosAttributes.userPriority              = (GT_U32)inFields[6];
    cosFormatEntry.cosAttributes.dropEligibilityIndication = (GT_U32)inFields[7];

    /* call cpss api function */
    result = cpssPxCosFormatEntrySet(devNum, packetType, &cosFormatEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosFormatEntryTableGetNext
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

CMD_STATUS wrCpssPxCosFormatEntryTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_SW_DEV_NUM                devNum;
    CPSS_PX_PACKET_TYPE          packetType;
    CPSS_PX_COS_FORMAT_ENTRY_STC cosFormatEntry = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    packetType                              = (CPSS_PX_PACKET_TYPE)inArgs[1];
    /* map input fields to locals */
    cosFormatEntry.cosMode                                 = (CPSS_PX_COS_MODE_ENT)inFields[0];
    cosFormatEntry.cosByteOffset                           = (GT_U32)inFields[1];
    cosFormatEntry.cosBitOffset                            = (GT_U32)inFields[2];
    cosFormatEntry.cosNumOfBits                            = (GT_U32)inFields[3];
    cosFormatEntry.cosAttributes.trafficClass              = (GT_U32)inFields[4];
    cosFormatEntry.cosAttributes.dropPrecedence            = (CPSS_DP_LEVEL_ENT)inFields[5];
    cosFormatEntry.cosAttributes.userPriority              = (GT_U32)inFields[6];
    cosFormatEntry.cosAttributes.dropEligibilityIndication = (GT_U32)inFields[7];

    /* call cpss api function */
    result = cpssPxCosFormatEntryGet(devNum, packetType, &cosFormatEntry);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d%d%d%d",
        cosFormatEntry.cosMode,
        cosFormatEntry.cosByteOffset,
        cosFormatEntry.cosBitOffset,
        cosFormatEntry.cosNumOfBits,
        cosFormatEntry.cosAttributes.trafficClass,
        cosFormatEntry.cosAttributes.dropPrecedence,
        cosFormatEntry.cosAttributes.userPriority,
        cosFormatEntry.cosAttributes.dropEligibilityIndication);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosPortAttributesTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inFields[0];
    cosAttributes.trafficClass              = (GT_U32)inFields[1];
    cosAttributes.dropPrecedence            = (CPSS_DP_LEVEL_ENT)inFields[2];
    cosAttributes.userPriority              = (GT_U32)inFields[3];
    cosAttributes.dropEligibilityIndication = (GT_U32)inFields[4];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCosPortAttributesSet(devNum, portNum, &cosAttributes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosPortAttributesTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (currentIndex >= lastIndex)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)currentIndex;
    currentIndex ++;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCosPortAttributesGet(devNum, portNum, &cosAttributes);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d", portNum, cosAttributes.trafficClass, cosAttributes.dropPrecedence,
        cosAttributes.userPriority, cosAttributes.dropEligibilityIndication);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosPortAttributesTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* inArgs[0] is devNum used by colee */
    currentIndex = (GT_U32)inArgs[1];
    lastIndex = (currentIndex + (GT_U32)inArgs[2]);

    return wrCpssPxCosPortAttributesTableGetNext(
        inArgs, inFields, numFields, outArgs);
}

CMD_STATUS wrCpssPxCosPortL2MappingTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32                      l2Index;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    /* map input fields to locals */
    l2Index                                 = (GT_U32)inFields[0];
    cosAttributes.trafficClass              = (GT_U32)inFields[1];
    cosAttributes.dropPrecedence            = (CPSS_DP_LEVEL_ENT)inFields[2];
    cosAttributes.userPriority              = (GT_U32)inFields[3];
    cosAttributes.dropEligibilityIndication = (GT_U32)inFields[4];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCosPortL2MappingSet(devNum, portNum, l2Index, &cosAttributes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosPortL2MappingTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32                      l2Index;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (currentIndex >= lastIndex)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    l2Index                                 = (GT_U32)currentIndex;
    currentIndex ++;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxCosPortL2MappingGet(devNum, portNum, l2Index, &cosAttributes);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d", l2Index, cosAttributes.trafficClass, cosAttributes.dropPrecedence,
        cosAttributes.userPriority, cosAttributes.dropEligibilityIndication);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosPortL2MappingTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* inArgs[0] is devNum used by colee  */
    /* inArgs[1] is portNum used by colee */
    currentIndex = (GT_U32)inArgs[2];
    lastIndex = (currentIndex + (GT_U32)inArgs[3]);

    return wrCpssPxCosPortL2MappingTableGetNext(
        inArgs, inFields, numFields, outArgs);
}

CMD_STATUS wrCpssPxCosL3MappingTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_U32                      l3Index;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    l3Index                                 = (GT_U32)inFields[0];
    cosAttributes.trafficClass              = (GT_U32)inFields[1];
    cosAttributes.dropPrecedence            = (CPSS_DP_LEVEL_ENT)inFields[2];
    cosAttributes.userPriority              = (GT_U32)inFields[3];
    cosAttributes.dropEligibilityIndication = (GT_U32)inFields[4];

    /* call cpss api function */
    result = cpssPxCosL3MappingSet(devNum, l3Index, &cosAttributes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosL3MappingTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_U32                      l3Index;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (currentIndex >= lastIndex)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    l3Index                                 = (GT_U32)currentIndex;
    currentIndex ++;

    /* call cpss api function */
    result = cpssPxCosL3MappingGet(devNum, l3Index, &cosAttributes);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d", l3Index, cosAttributes.trafficClass, cosAttributes.dropPrecedence,
        cosAttributes.userPriority, cosAttributes.dropEligibilityIndication);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosL3MappingTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* inArgs[0] is devNum used by colee  */
    currentIndex = (GT_U32)inArgs[1];
    lastIndex = (currentIndex + (GT_U32)inArgs[2]);

    return wrCpssPxCosL3MappingTableGetNext(
        inArgs, inFields, numFields, outArgs);
}

CMD_STATUS wrCpssPxCosMplsMappingTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_U32                      mplsIndex;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    mplsIndex                               = (GT_U32)inFields[0];
    cosAttributes.trafficClass              = (GT_U32)inFields[1];
    cosAttributes.dropPrecedence            = (CPSS_DP_LEVEL_ENT)inFields[2];
    cosAttributes.userPriority              = (GT_U32)inFields[3];
    cosAttributes.dropEligibilityIndication = (GT_U32)inFields[4];

    /* call cpss api function */
    result = cpssPxCosMplsMappingSet(devNum, mplsIndex, &cosAttributes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosMplsMappingTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_U32                      mplsIndex;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (currentIndex >= lastIndex)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    mplsIndex                               = (GT_U32)currentIndex;
    currentIndex ++;

    /* call cpss api function */
    result = cpssPxCosMplsMappingGet(devNum, mplsIndex, &cosAttributes);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d", mplsIndex, cosAttributes.trafficClass, cosAttributes.dropPrecedence,
        cosAttributes.userPriority, cosAttributes.dropEligibilityIndication);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosMplsMappingTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* inArgs[0] is devNum used by colee  */
    currentIndex = (GT_U32)inArgs[1];
    lastIndex = (currentIndex + (GT_U32)inArgs[2]);

    return wrCpssPxCosMplsMappingTableGetNext(
        inArgs, inFields, numFields, outArgs);
}

CMD_STATUS wrCpssPxCosDsaMappingTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_U32                      dsaIndex;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    dsaIndex                                = (GT_U32)inFields[0];
    cosAttributes.trafficClass              = (GT_U32)inFields[1];
    cosAttributes.dropPrecedence            = (CPSS_DP_LEVEL_ENT)inFields[2];
    cosAttributes.userPriority              = (GT_U32)inFields[3];
    cosAttributes.dropEligibilityIndication = (GT_U32)inFields[4];

    /* call cpss api function */
    result = cpssPxCosDsaMappingSet(devNum, dsaIndex, &cosAttributes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosDsaMappingTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_SW_DEV_NUM               devNum;
    GT_U32                      dsaIndex;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (currentIndex >= lastIndex)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    dsaIndex                                = (GT_U32)currentIndex;
    currentIndex ++;

    /* call cpss api function */
    result = cpssPxCosDsaMappingGet(devNum, dsaIndex, &cosAttributes);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    fieldOutput(
        "%d%d%d%d%d", dsaIndex, cosAttributes.trafficClass, cosAttributes.dropPrecedence,
        cosAttributes.userPriority, cosAttributes.dropEligibilityIndication);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

CMD_STATUS wrCpssPxCosDsaMappingTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* inArgs[0] is devNum used by colee  */
    currentIndex = (GT_U32)inArgs[1];
    lastIndex = (currentIndex + (GT_U32)inArgs[2]);

    return wrCpssPxCosDsaMappingTableGetNext(
        inArgs, inFields, numFields, outArgs);
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName,                            funcReference                             funcArgs funcFields */
    { "cpssPxCosFormatEntryTableSet",          &wrCpssPxCosFormatEntryTableSet,          2, 8},
    { "cpssPxCosFormatEntryTableGetNext",      &wrCpssPxCosFormatEntryTableGetNext,      2, 0},
    { "cpssPxCosFormatEntryTableGetFirst",     &wrCpssPxCosFormatEntryTableGetFirst,     2, 0},
    { "cpssPxCosPortAttributesTableSet",       &wrCpssPxCosPortAttributesTableSet,       3, 5},
    { "cpssPxCosPortAttributesTableGetNext",   &wrCpssPxCosPortAttributesTableGetNext,   3, 0},
    { "cpssPxCosPortAttributesTableGetFirst",  &wrCpssPxCosPortAttributesTableGetFirst,  3, 0},
    { "cpssPxCosPortL2MappingTableSet",        &wrCpssPxCosPortL2MappingTableSet,        4, 5},
    { "cpssPxCosPortL2MappingTableGetNext",    &wrCpssPxCosPortL2MappingTableGetNext,    4, 0},
    { "cpssPxCosPortL2MappingTableGetFirst",   &wrCpssPxCosPortL2MappingTableGetFirst,   4, 0},
    { "cpssPxCosL3MappingTableSet",            &wrCpssPxCosL3MappingTableSet,            3, 5},
    { "cpssPxCosL3MappingTableGetNext",        &wrCpssPxCosL3MappingTableGetNext,        3, 0},
    { "cpssPxCosL3MappingTableGetFirst",       &wrCpssPxCosL3MappingTableGetFirst,       3, 0},
    { "cpssPxCosMplsMappingTableSet",          &wrCpssPxCosMplsMappingTableSet,          3, 5},
    { "cpssPxCosMplsMappingTableGetNext",      &wrCpssPxCosMplsMappingTableGetNext,      3, 0},
    { "cpssPxCosMplsMappingTableGetFirst",     &wrCpssPxCosMplsMappingTableGetFirst,     3, 0},
    { "cpssPxCosDsaMappingTableSet",           &wrCpssPxCosDsaMappingTableSet,           3, 5},
    { "cpssPxCosDsaMappingTableGetNext",       &wrCpssPxCosDsaMappingTableGetNext,       3, 0},
    { "cpssPxCosDsaMappingTableGetFirst",      &wrCpssPxCosDsaMappingTableGetFirst,      3, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxCos function
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
GT_STATUS cmdLibInitCpssPxCos
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



