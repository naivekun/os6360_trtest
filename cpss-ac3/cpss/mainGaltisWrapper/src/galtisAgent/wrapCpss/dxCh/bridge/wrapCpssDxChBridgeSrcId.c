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
* wrapBridgeSrcIdCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for BridgeSrcId cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>

/* DxCh max number of entries to be read from tables\classes */
#define  CPSS_DXCH_SOURCE_ID_MAX_ENTRY_CNS 4096
/**
* @internal wrCpssDxChBrgSrcIdGroupPortAdd function
* @endinternal
*
* @brief   Add a port to Source ID group. Packets assigned with this Source ID
*         may be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGroupPortAdd

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_U32     sourceId;
    GT_PHYSICAL_PORT_NUM      portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    sourceId = (GT_U32)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGroupPortAdd(devNum, sourceId, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdGroupPortDelete function
* @endinternal
*
* @brief   Delete a port from Source ID group. Packets assigned with this
*         Source ID will not be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGroupPortDelete

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_U32     sourceId;
    GT_PHYSICAL_PORT_NUM      portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    sourceId = (GT_U32)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGroupPortDelete(devNum, sourceId, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}




/*cpssDxChBrgSrcIdGroupEntry Table*/
static GT_U32   sourceIdCnt;

/**
* @internal wrCpssDxChBrgSrcIdGroupEntrySet function
* @endinternal
*
* @brief   Set entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId, ports bitmap value
* @retval GT_BAD_PTR               - portsMembersPtr is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGroupEntrySet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;

    GT_U8                            devNum;
    GT_U32                           sourceId;
    GT_BOOL                          cpuSrcIdMember;
    CPSS_PORTS_BMP_STC               portsMembers;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    sourceId = (GT_U32)inFields[0];
    cpuSrcIdMember = (GT_BOOL)inFields[1];

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    portsMembers.ports[0] = (GT_U32)inFields[2];
    portsMembers.ports[1] = (GT_U32)inFields[3];
    CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembers);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGroupEntrySet(devNum, sourceId, cpuSrcIdMember,
                                                                &portsMembers);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdGroupEntryGetFirst function
* @endinternal
*
* @brief   Get entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGroupEntryGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                            devNum;
    GT_BOOL                          cpuSrcIdMemberPtr;
    CPSS_PORTS_BMP_STC               portsMembers;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    sourceIdCnt = 0;

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGroupEntryGet(devNum, sourceIdCnt,
                                            &cpuSrcIdMemberPtr,
                                              &portsMembers);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }


    if(cpuSrcIdMemberPtr)
    {
        CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembers);
        inFields[0] = sourceIdCnt;
        inFields[1] = cpuSrcIdMemberPtr;
        inFields[2] = portsMembers.ports[0];
        inFields[3] = portsMembers.ports[1];

        /* pack and output table fields */
        fieldOutput("%d%d%d%d", inFields[0], inFields[1],
                                inFields[2], inFields[3]);

        galtisOutput(outArgs, GT_OK, "%f");
    }
    else
    {
       galtisOutput(outArgs, result, "");
    }
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgSrcIdGroupEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                            devNum;
    GT_BOOL                          cpuSrcIdMemberPtr;
    CPSS_PORTS_BMP_STC               portsMembers;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    sourceIdCnt++;

    if(sourceIdCnt >= CPSS_DXCH_SOURCE_ID_MAX_ENTRY_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGroupEntryGet(devNum, sourceIdCnt,
                                            &cpuSrcIdMemberPtr,
                                              &portsMembers);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if(cpuSrcIdMemberPtr)
    {
        CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembers);
        inFields[0] = sourceIdCnt;
        inFields[1] = cpuSrcIdMemberPtr;
        inFields[2] = portsMembers.ports[0];
        inFields[3] = portsMembers.ports[1];

        /* pack and output table fields */
        fieldOutput("%d%d%d%d", inFields[0], inFields[1],
                                inFields[2], inFields[3]);

        galtisOutput(outArgs, GT_OK, "%f");
    }
    else
    {
       galtisOutput(outArgs, result, "");
    }
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSrcIdPortDefaultSrcIdSet function
* @endinternal
*
* @brief   Configure Port's Default Source ID.
*         The Source ID is used for source based egress filtering.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortDefaultSrcIdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_PORT_NUM      portNum;
    GT_U32     defaultSrceId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    defaultSrceId = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortDefaultSrcIdSet(devNum, portNum,
                                                   defaultSrceId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdPortDefaultSrcIdGet function
* @endinternal
*
* @brief   Get configuration of Port's Default Source ID.
*         The Source ID is used for Source based egress filtering.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortDefaultSrcIdGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_PORT_NUM      portNum;
    GT_U32     defaultSrceIdPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortDefaultSrcIdGet(devNum, portNum,
                                               &defaultSrceIdPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", defaultSrceIdPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSrcIdGlobalUcastEgressFilterSet function
* @endinternal
*
* @brief   Enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGlobalUcastEgressFilterSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdGlobalUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGlobalUcastEgressFilterGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;

    GT_U8      devNum;
    GT_BOOL    enablePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGlobalUcastEgressFilterGet(devNum, &enablePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enablePtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode for non-DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      All DXCH devices exclude DXCH2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mode
* @retval GT_NOT_SUPPORTED         - this request isn't supported
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeGet function
* @endinternal
*
* @brief   Get Source ID Assignment mode for non-DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      All DXCH devices exclude DXCH2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_SUPPORTED         - this request isn't supported
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT    modePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(devNum, &modePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", modePtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdPortSrcIdAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode for non-DSA tagged packets
*         per Ingress Port.
*
* @note   APPLICABLE DEVICES:      All DXCH2 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, mode
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortSrcIdAssignModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    GT_PORT_NUM                              portNum;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT    mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    mode = (CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortSrcIdAssignModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdPortSrcIdAssignModeGet function
* @endinternal
*
* @brief   Get Source ID Assignment mode for non-DSA tagged packets
*         per Ingress Port.
*
* @note   APPLICABLE DEVICES:      All DXCH2 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortSrcIdAssignModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    GT_PORT_NUM                              portNum;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT    modePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortSrcIdAssignModeGet(devNum, portNum, &modePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", modePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSrcIdPortUcastEgressFilterSet function
* @endinternal
*
* @brief   Per Egress Port enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      All DXCH2 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortUcastEgressFilterSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    GT_PHYSICAL_PORT_NUM                              portNum;
    GT_BOOL                            enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortUcastEgressFilterSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgSrcIdPortUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Per Egress Port Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      All DXCH2 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortUcastEgressFilterGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    GT_PHYSICAL_PORT_NUM                              portNum;
    GT_BOOL                            enablePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortUcastEgressFilterGet(devNum, portNum,
                                                           &enablePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enablePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSrcIdPortSrcIdForceEnableSet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortSrcIdForceEnableSet
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
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSrcIdPortSrcIdForceEnableGet function
* @endinternal
*
* @brief   Get Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgSrcIdPortSrcIdForceEnableGet
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
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSrcIdEtagTypeLocationSet function
* @endinternal
*
* @brief   Set the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] bitLocation              - bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bitLocation
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgSrcIdEtagTypeLocationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      bitLocation;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    bitLocation = (GT_U32)inArgs[1];


    /* call cpss api function */
    result = cpssDxChBrgSrcIdEtagTypeLocationSet(devNum, bitLocation);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgSrcIdEtagTypeLocationGet function
* @endinternal
*
* @brief   Get the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @param[out] bitLocationPtr          - (pointer to) bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgSrcIdEtagTypeLocationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      bitLocation;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgSrcIdEtagTypeLocationGet(devNum, &bitLocation);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", bitLocation);
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChBrgSrcIdGroupPortAdd",
        &wrCpssDxChBrgSrcIdGroupPortAdd,
        3, 0},

    {"cpssDxChBrgSrcIdGroupPortDelete",
        &wrCpssDxChBrgSrcIdGroupPortDelete,
        3, 0},

    {"cpssDxChBrgSrcIdGroupEntrySet",
        &wrCpssDxChBrgSrcIdGroupEntrySet,
        1, 4},

    {"cpssDxChBrgSrcIdGroupEntryGetFirst",
        &wrCpssDxChBrgSrcIdGroupEntryGetFirst,
        1, 0},

    {"cpssDxChBrgSrcIdGroupEntryGetNext",
        &wrCpssDxChBrgSrcIdGroupEntryGetNext,
        1, 0},

    {"cpssDxChBrgSrcIdPortDefaultSrcIdSet",
        &wrCpssDxChBrgSrcIdPortDefaultSrcIdSet,
        3, 0},

    {"cpssDxChBrgSrcIdPortDefaultSrcIdGet",
        &wrCpssDxChBrgSrcIdPortDefaultSrcIdGet,
        2, 0},

    {"cpssDxChBrgSrcIdGlobalUcastEgressFilterSet",
        &wrCpssDxChBrgSrcIdGlobalUcastEgressFilterSet,
        2, 0},

    {"cpssDxChBrgSrcIdGlobalUcastEgressFilterGet",
        &wrCpssDxChBrgSrcIdGlobalUcastEgressFilterGet,
        1, 0},

    {"cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet",
        &wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeSet,
        2, 0},

    {"cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet",
        &wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeGet,
        1, 0},

    {"cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet1",
        &wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeSet,
        2, 0},

    {"cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet1",
        &wrCpssDxChBrgSrcIdGlobalSrcIdAssignModeGet,
        1, 0},

    {"cpssDxChBrgSrcIdPortSrcIdAssignModeSet",
        &wrCpssDxChBrgSrcIdPortSrcIdAssignModeSet,
        3, 0},

    {"cpssDxChBrgSrcIdPortSrcIdAssignModeGet",
        &wrCpssDxChBrgSrcIdPortSrcIdAssignModeGet,
        2, 0},

    {"cpssDxChBrgSrcIdPortUcastEgressFilterSet",
        &wrCpssDxChBrgSrcIdPortUcastEgressFilterSet,
        3, 0},

    {"cpssDxChBrgSrcIdPortUcastEgressFilterGet",
        &wrCpssDxChBrgSrcIdPortUcastEgressFilterGet,
        2, 0},

    {"cpssDxChBrgSrcIdPortSrcIdForceEnableSet",
        &wrCpssDxChBrgSrcIdPortSrcIdForceEnableSet,
        3, 0},

    {"cpssDxChBrgSrcIdPortSrcIdForceEnableGet",
        &wrCpssDxChBrgSrcIdPortSrcIdForceEnableGet,
        2, 0},

    {"cpssDxChBrgSrcIdEtagTypeLocationSet",
        &wrCpssDxChBrgSrcIdEtagTypeLocationSet,
        2, 0},

    {"cpssDxChBrgSrcIdEtagTypeLocationGet",
        &wrCpssDxChBrgSrcIdEtagTypeLocationGet,
        1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChBridgeSrcId function
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
GT_STATUS cmdLibInitCpssDxChBridgeSrcId
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}




