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
* @file wrapCpssPxEgress.c
*
* @brief Wrapper functions for
* cpss/Px/egress/cpssPxEgress.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operation802_1BrTypeArr[] =
    {
        CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E

        /*802.1br IPL types*/
        ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E
        ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E

       /* General types */
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
    };
static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operation802_1BrExtTypeArr[] =
    {
        CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E
    };

static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationDsaTypeArr[] =
    {
        CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E

       /* General types */
       ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
    };

/**
* @internal wrCpssPxEgressBypassModeSet function
* @endinternal
*
* @brief   Enable/disable bypass of the egress processing and option to disable
*         PPA clock for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressBypassModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         phaBypassEnable;
    GT_BOOL                         ppaClockEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    phaBypassEnable                         = (GT_BOOL)inArgs[1];
    ppaClockEnable                          = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxEgressBypassModeSet(devNum, phaBypassEnable, ppaClockEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxEgressBypassModeGet function
* @endinternal
*
* @brief   Get bypass enable/disable of the egress processing and is the PPA clock
*         disable for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressBypassModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         phaBypassEnable;
    GT_BOOL                         ppaClockEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxEgressBypassModeGet(devNum, &phaBypassEnable, &ppaClockEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", phaBypassEnable, ppaClockEnable);

    return CMD_OK;
}


/**
* @internal wrCpssPxEgressTimestampModeSet function
* @endinternal
*
* @brief   Set if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressTimestampModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PACKET_TYPE             packetType;
    GT_BOOL                         useTod;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    packetType                              = (CPSS_PX_PACKET_TYPE)inFields[0];
    useTod                                  = (GT_BOOL)inFields[1];

    /* call cpss api function */
    result = cpssPxEgressTimestampModeSet(devNum, packetType, useTod);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static CPSS_PX_PACKET_TYPE                          gPacketType;
/**
* @internal wrCpssPxEgressTimestampModeGet function
* @endinternal
*
* @brief   Get if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressTimestampModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         useTod;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api get function */
    result = cpssPxEgressTimestampModeGet(devNum, gPacketType, &useTod);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d", gPacketType, useTod);
    fieldOutputSetAppendMode();

    /* Increment packet type before next read */
    gPacketType++;

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxEgressTimestampModeGet */
CMD_STATUS wrCpssPxEgressTimestampModeGetFirst
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

    gPacketType = 0;

    return wrCpssPxEgressTimestampModeGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxEgressTimestampModeGet */
CMD_STATUS wrCpssPxEgressTimestampModeGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxEgressTimestampModeGet(inArgs, inFields, numFields, outArgs);
}

/* Set DSA tag from input fields */
#define dsaDataSet(dsaForwardPtr, inFields) \
{\
    dsaForwardPtr->srcTagged = (GT_BOOL)inFields[i++]; \
    dsaForwardPtr->hwSrcDev = (GT_HW_DEV_NUM)inFields[i++]; \
    dsaForwardPtr->srcPortOrTrunk = (GT_U32)inFields[i++]; \
    dsaForwardPtr->srcIsTrunk = (GT_BOOL)inFields[i++]; \
    dsaForwardPtr->cfi = (GT_U32)inFields[i++]; \
    dsaForwardPtr->up = (GT_U32)inFields[i++]; \
    dsaForwardPtr->vid = (GT_U32)inFields[i++]; \
}

/* Get DSA tag from structure */
#define dsaDataGet(inFields, dsaForwardPtr) \
{ \
    inFields[i++] = (GT_BOOL)dsaForwardPtr->srcTagged; \
    inFields[i++] = (GT_HW_DEV_NUM)dsaForwardPtr->hwSrcDev; \
    inFields[i++] = (GT_U32)dsaForwardPtr->srcPortOrTrunk; \
    inFields[i++] = (GT_BOOL)dsaForwardPtr->srcIsTrunk; \
    inFields[i++] = (GT_U32)dsaForwardPtr->cfi; \
    inFields[i++] = (GT_U32)dsaForwardPtr->up; \
    inFields[i++] = (GT_U32)dsaForwardPtr->vid; \
}

/**
* @internal wrCpssPxEgressHeaderAlteration802_1BrEntrySet function
* @endinternal
*
* @brief   Set per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_PACKET_TYPE                                 packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT           operationType;
    CPSS_802_1BR_ETAG_STC                               *eTagPtr;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT          operationInfo;
    GT_U32                                              i;
    GT_U32                                              tabIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&operationInfo, 0, sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tabIndex                            = (CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT)inArgs[2];
    operationType                       = operation802_1BrTypeArr[tabIndex];

    i = 0;
    packetType                          = (GT_U32)inFields[i++];

    switch (operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E:
            eTagPtr = &operationInfo.info_802_1br_E2U.eTag;
            /* map input fields to locals */
            eTagPtr->TPID                       = (GT_U32)inFields[i++];
            eTagPtr->E_PCP                      = (GT_U32)inFields[i++];
            eTagPtr->E_DEI                      = (GT_U32)inFields[i++];
            eTagPtr->Ingress_E_CID_base         = (GT_U32)inFields[i++];
            eTagPtr->Direction                  = (GT_U32)inFields[i++];
            eTagPtr->Upstream_Specific          = (GT_U32)inFields[i++];
            eTagPtr->GRP                        = (GT_U32)inFields[i++];
            eTagPtr->E_CID_base                 = (GT_U32)inFields[i++];
            eTagPtr->Ingress_E_CID_ext          = (GT_U32)inFields[i++];
            eTagPtr->E_CID_ext                  = (GT_U32)inFields[i++];
        default:
            break;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxEgressHeaderAlterationEntrySet(devNum, portNum, packetType, operationType, &operationInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxEgressHeaderAlteration802_1BrEntryGet function
* @endinternal
*
* @brief   Get per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT           operationType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT          operationInfo;
    CPSS_802_1BR_ETAG_STC                               *eTagPtr;

    GT_U32                          actualNumFields;
    GT_U32                          i;
    GT_U32                          tabIndex;

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api get function */
    result = cpssPxEgressHeaderAlterationEntryGet(devNum, portNum, gPacketType, &operationType, &operationInfo);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    i = 0;
    inFields[i++] = gPacketType++;

    switch (operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
            /* get entry pointer */
            eTagPtr = &operationInfo.info_802_1br_E2U.eTag;
            /* set entry data */
            inFields[i++] = eTagPtr->TPID;
            inFields[i++] = eTagPtr->E_PCP;
            inFields[i++] = eTagPtr->E_DEI;
            inFields[i++] = eTagPtr->Ingress_E_CID_base;
            inFields[i++] = eTagPtr->Direction;
            inFields[i++] = eTagPtr->Upstream_Specific;
            inFields[i++] = eTagPtr->GRP;
            inFields[i++] = eTagPtr->E_CID_base;
            inFields[i++] = eTagPtr->Ingress_E_CID_ext;
            inFields[i++] = eTagPtr->E_CID_ext;
            tabIndex = 0;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E:
            tabIndex = 1;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E:
            tabIndex = 2;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E:
            tabIndex = 3;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E:
            /* get entry pointer */
            eTagPtr = &operationInfo.info_802_1br_E2U.eTag;
            /* set entry data */
            inFields[i++] = eTagPtr->TPID;
            inFields[i++] = eTagPtr->E_PCP;
            inFields[i++] = eTagPtr->E_DEI;
            inFields[i++] = eTagPtr->Ingress_E_CID_base;
            inFields[i++] = eTagPtr->Direction;
            inFields[i++] = eTagPtr->Upstream_Specific;
            inFields[i++] = eTagPtr->GRP;
            inFields[i++] = eTagPtr->E_CID_base;
            inFields[i++] = eTagPtr->Ingress_E_CID_ext;
            inFields[i++] = eTagPtr->E_CID_ext;
            tabIndex = 4;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E:
            tabIndex = 5;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E:
            tabIndex = 8;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E:
            tabIndex = 6;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E:
            tabIndex = 7;
            break;
        default:
            galtisOutput(outArgs, GT_OK, "");
            return CMD_OK;
    }

    actualNumFields = i;

    /* start from field 0 */
    i = 0;
    /* Tab-number, packet type */
    fieldOutput(
        "%d", inFields[i]);
    i++;
    fieldOutputSetAppendMode();

    /* pack and output table fields */
    for (; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%d%f", tabIndex);

    return CMD_OK;
}

/* wrapper for cpssPxEgressHeaderAlterationEntryGet */
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrEntryGetFirst
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

    gPacketType = 0;

    return wrCpssPxEgressHeaderAlteration802_1BrEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxEgressHeaderAlterationEntryGet */
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxEgressHeaderAlteration802_1BrEntryGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssPxEgressHeaderAlteration802_1BrExtEntrySet
*           function
* @endinternal
*
* @brief   Set per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrExtEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_PACKET_TYPE                                 packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT           operationType;
    CPSS_802_1BR_ETAG_STC                               *eTagPtr;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC  *pcidPtr;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT          operationInfo;
    GT_U32                                              i;
    GT_U32                                              tabIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&operationInfo, 0, sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tabIndex                            = (CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT)inArgs[2];
    operationType                       = operation802_1BrExtTypeArr[tabIndex];
    i = 0;
    packetType                          = (GT_U32)inFields[i++];

    switch (operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E:
            pcidPtr = &operationInfo.info_802_1br_U2E_MC;
            /* map input fields to locals */
            pcidPtr->pcid[0]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[1]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[2]         = (GT_U32)inFields[i++];
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E:
            pcidPtr = &operationInfo.info_802_1br_U2E_MC;
            /* map input fields to locals */
            pcidPtr->pcid[0]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[1]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[2]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[3]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[4]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[5]         = (GT_U32)inFields[i++];
            pcidPtr->pcid[6]         = (GT_U32)inFields[i++];
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E:
            eTagPtr = &operationInfo.info_802_1br_E2U.eTag;
            /* map input fields to locals */
            eTagPtr->E_PCP                      = (GT_U32)inFields[i++];
            eTagPtr->E_DEI                      = (GT_U32)inFields[i++];
            eTagPtr->Ingress_E_CID_base         = (GT_U32)inFields[i++];
            eTagPtr->GRP                        = (GT_U32)inFields[i++];
            eTagPtr->E_CID_base                 = (GT_U32)inFields[i++];
            eTagPtr->Ingress_E_CID_ext          = (GT_U32)inFields[i++];
            eTagPtr->E_CID_ext                  = (GT_U32)inFields[i++];
            operationInfo.info_802_1br_E2U.vlanTagTpid
                                                = (GT_U32)inFields[i++];
            break;
        default:
            break;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssPxEgressHeaderAlterationEntrySet(devNum, portNum, packetType, operationType, &operationInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxEgressHeaderAlteration802_1BrExtEntryGet
*           function
* @endinternal
*
* @brief   Get per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrExtEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT           operationType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT          operationInfo;
    CPSS_802_1BR_ETAG_STC                               *eTagPtr;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *pcidPtr;

    GT_U32                          actualNumFields;
    GT_U32                          i;
    GT_U32                          tabIndex;

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;



    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api get function */
    result = cpssPxEgressHeaderAlterationEntryGet(devNum, portNum, gPacketType, &operationType, &operationInfo);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    i = 0;
    inFields[i++] = gPacketType++;

    switch (operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E:
            pcidPtr = &operationInfo.info_802_1br_U2E_MC;
            /* set entry data */
            inFields[i++] = pcidPtr->pcid[0];
            inFields[i++] = pcidPtr->pcid[1];
            inFields[i++] = pcidPtr->pcid[2];
            tabIndex = 0;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E:
            pcidPtr = &operationInfo.info_802_1br_U2E_MC;
            /* set entry data */
            inFields[i++] = pcidPtr->pcid[0];
            inFields[i++] = pcidPtr->pcid[1];
            inFields[i++] = pcidPtr->pcid[2];
            inFields[i++] = pcidPtr->pcid[3];
            inFields[i++] = pcidPtr->pcid[4];
            inFields[i++] = pcidPtr->pcid[5];
            inFields[i++] = pcidPtr->pcid[6];
            tabIndex = 1;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E:
           /* get entry pointer */
            eTagPtr = &operationInfo.info_802_1br_E2U.eTag;
            /* set entry data */
            inFields[i++] = eTagPtr->E_PCP;
            inFields[i++] = eTagPtr->E_DEI;
            inFields[i++] = eTagPtr->Ingress_E_CID_base;
            inFields[i++] = eTagPtr->GRP;
            inFields[i++] = eTagPtr->E_CID_base;
            inFields[i++] = eTagPtr->Ingress_E_CID_ext;
            inFields[i++] = eTagPtr->E_CID_ext;
            inFields[i++] = operationInfo.info_802_1br_E2U.vlanTagTpid;
            tabIndex = 2;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E:
            tabIndex = 3;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E:
            tabIndex = 4;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E:
            tabIndex = 5;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E:
            tabIndex = 6;
            break;
        default:
            galtisOutput(outArgs, GT_OK, "");
            return CMD_OK;
    }

    actualNumFields = i;

    /* start from field 0 */
    i = 0;
    /* Tab-number, packet type */
    fieldOutput(
        "%d", inFields[i]);
    i++;
    fieldOutputSetAppendMode();

    /* pack and output table fields */
    for (; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%d%f", tabIndex);

    return CMD_OK;
}

/* wrapper for cpssPxEgressHeaderAlterationEntryGet */
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrExtEntryGetFirst
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

    gPacketType = 0;

    return wrCpssPxEgressHeaderAlteration802_1BrExtEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxEgressHeaderAlterationEntryGet */
CMD_STATUS wrCpssPxEgressHeaderAlteration802_1BrExtEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxEgressHeaderAlteration802_1BrExtEntryGet(inArgs, inFields, numFields, outArgs);
}
/**
* @internal wrCpssPxEgressHeaderAlterationDsaEntrySet function
* @endinternal
*
* @brief   Set per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlterationDsaEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_PACKET_TYPE                                 packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT           operationType;
    CPSS_PX_REGULAR_DSA_FORWARD_STC                     *dsaForwardPtr;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC   *dsaQcnPtr;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT          operationInfo;
    GT_ETHERADDR                                        mac;
    GT_U32                                              i;
    CPSS_PX_EXTENDED_DSA_FORWARD_STC                    *dsaExtFwrdPtr;
    GT_U32                                              tabIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&operationInfo, 0, sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    tabIndex                            = (CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT)inArgs[2];
    operationType                       = operationDsaTypeArr[tabIndex];

    i = 0;
    packetType                          = (GT_U32)inFields[i++];

    switch (operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E:
            dsaForwardPtr = &operationInfo.info_dsa_ET2U.dsaForward;
            dsaDataSet(dsaForwardPtr, inFields);
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
            dsaForwardPtr = &operationInfo.info_dsa_EU2U.dsaForward;
            dsaDataSet(dsaForwardPtr, inFields);
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E:
            dsaQcnPtr = &operationInfo.info_dsa_QCN;
            galtisMacAddr(&mac,  (GT_U8*)inFields[i++]);
            cmdOsMemCpy(dsaQcnPtr->macSa.arEther, mac.arEther, 6);
            dsaExtFwrdPtr = &dsaQcnPtr->dsaExtForward;
            dsaExtFwrdPtr->srcTagged        = (GT_BOOL)         inFields[i++];
            dsaExtFwrdPtr->hwSrcDev         = (GT_HW_DEV_NUM)   inFields[i++];
            dsaExtFwrdPtr->srcPortOrTrunk   = (GT_U32)          inFields[i++];
            dsaExtFwrdPtr->srcIsTrunk       = (GT_BOOL)         inFields[i++];
            dsaExtFwrdPtr->cfi              = (GT_U32)          inFields[i++];
            dsaExtFwrdPtr->up               = (GT_U32)          inFields[i++];
            dsaExtFwrdPtr->vid              = (GT_U32)          inFields[i++];

            dsaExtFwrdPtr->egrFilterRegistered  = (GT_BOOL)     inFields[i++];
            dsaExtFwrdPtr->dropOnSource     = (GT_BOOL)         inFields[i++];
            dsaExtFwrdPtr->packetIsLooped   = (GT_BOOL)         inFields[i++];
            dsaExtFwrdPtr->wasRouted        = (GT_BOOL)         inFields[i++];
            dsaExtFwrdPtr->srcId            = (GT_U32)          inFields[i++];
            dsaExtFwrdPtr->qosProfileIndex  = (GT_U32)          inFields[i++];

            dsaExtFwrdPtr->useVidx          = (GT_BOOL)         inFields[i++];
            dsaExtFwrdPtr->trgVidx          = (GT_U32)          inFields[i++];
            dsaExtFwrdPtr->trgPort          = (GT_U32)          inFields[i++];
            dsaExtFwrdPtr->hwTrgDev         = (GT_HW_DEV_NUM)   inFields[i++];

            dsaQcnPtr->cnmTpid          = (GT_U16)          inFields[i++];
            break;
        default:
            break;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxEgressHeaderAlterationEntrySet(devNum, portNum, packetType, operationType, &operationInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxEgressHeaderAlterationDsaEntryGet function
* @endinternal
*
* @brief   Get per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlterationDsaEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT           operationType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT          operationInfo;
    CPSS_PX_REGULAR_DSA_FORWARD_STC                     *dsaForwardPtr;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC   *dsaQcnPtr;
    CPSS_PX_EXTENDED_DSA_FORWARD_STC                    *dsaExtFwrdPtr;

    GT_U32                          actualNumFields;
    GT_U32                          i;
    GT_U32                          tabIndex;

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api get function */
    result = cpssPxEgressHeaderAlterationEntryGet(devNum, portNum, gPacketType, &operationType, &operationInfo);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    i = 0;
    inFields[i++] = gPacketType++;

    switch (operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E:
            /* get entry pointer */
            dsaForwardPtr = &operationInfo.info_dsa_ET2U.dsaForward;
            /* set entry data */
            dsaDataGet(inFields, dsaForwardPtr);
            tabIndex = 0;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
            /* get entry pointer */
            dsaForwardPtr = &operationInfo.info_dsa_EU2U.dsaForward;
            /* set entry data */
            dsaDataGet(inFields, dsaForwardPtr);
            tabIndex = 1;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E:
            tabIndex = 2;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E:
            tabIndex = 3;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E:
            tabIndex = 4;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E:
            /* get entry pointer */
            dsaQcnPtr = &operationInfo.info_dsa_QCN;
            dsaExtFwrdPtr = &dsaQcnPtr->dsaExtForward;
            /* Skip MAC field */
            i++;
            /* set entry data */
            inFields[i++] = dsaExtFwrdPtr->srcTagged;
            inFields[i++] = dsaExtFwrdPtr->hwSrcDev;
            inFields[i++] = dsaExtFwrdPtr->srcPortOrTrunk;
            inFields[i++] = dsaExtFwrdPtr->srcIsTrunk;
            inFields[i++] = dsaExtFwrdPtr->cfi;
            inFields[i++] = dsaExtFwrdPtr->up;
            inFields[i++] = dsaExtFwrdPtr->vid;
            inFields[i++] = dsaExtFwrdPtr->egrFilterRegistered;
            inFields[i++] = dsaExtFwrdPtr->dropOnSource;
            inFields[i++] = dsaExtFwrdPtr->packetIsLooped;
            inFields[i++] = dsaExtFwrdPtr->wasRouted;
            inFields[i++] = dsaExtFwrdPtr->srcId;
            inFields[i++] = dsaExtFwrdPtr->qosProfileIndex;
            inFields[i++] = dsaExtFwrdPtr->useVidx;
            inFields[i++] = dsaExtFwrdPtr->trgVidx;
            inFields[i++] = dsaExtFwrdPtr->trgPort;
            inFields[i++] = dsaExtFwrdPtr->hwTrgDev;
            inFields[i++] = dsaQcnPtr->cnmTpid;
            tabIndex = 5;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E:
            tabIndex = 6;
            break;
        default:
            galtisOutput(outArgs, GT_OK, "");
            return CMD_OK;
    }

    actualNumFields = i;

    /* start from field 0 */
    i = 0;
    /* Tab-number, packet type */
    fieldOutput(
        "%d", inFields[i]);
    i++;
    fieldOutputSetAppendMode();

    if (operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E)
    {
        fieldOutput("%s",
                    galtisByteArrayToString(dsaQcnPtr->macSa.arEther, 6));
        i++;
        fieldOutputSetAppendMode();
    }
    /* pack and output table fields */
    for (; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%d%f", tabIndex);

    return CMD_OK;
}

/* wrapper for cpssPxEgressHeaderAlterationEntryGet */
CMD_STATUS wrCpssPxEgressHeaderAlterationDsaEntryGetFirst
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

    gPacketType = 0;

    return wrCpssPxEgressHeaderAlterationDsaEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxEgressHeaderAlterationEntryGet */
CMD_STATUS wrCpssPxEgressHeaderAlterationDsaEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxEgressHeaderAlterationDsaEntryGet(inArgs, inFields, numFields, outArgs);
}


/**
* @internal wrCpssPxEgressHeaderAlterationDebugEntrySet
*           function
* @endinternal
*
* @brief   Set per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlterationDebugEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_PACKET_TYPE                                 packetType;
    GT_U32                                              entryIndex;
    GT_U32                                              firmwareInstructionPointer;
    GT_U32                                              haEntryWords[5];/* needed 144 bits , supporting 32*5=160 bits */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    packetType                          = (GT_U32)inArgs[2];
    haEntryWords[0]                     = (GT_U32)inArgs[3];
    haEntryWords[1]                     = (GT_U32)inArgs[4];
    haEntryWords[2]                     = (GT_U32)inArgs[5];
    haEntryWords[3]                     = (GT_U32)inArgs[6];
    firmwareInstructionPointer          = (GT_U32)inArgs[7];;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    entryIndex = portNum << 5 | packetType;
    haEntryWords[4] = firmwareInstructionPointer;
    result = prvCpssPxWriteTableEntry(devNum,CPSS_PX_TABLE_PHA_HA_TABLE_E,
        entryIndex,&haEntryWords[0]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxEgressHeaderAlterationDebugEntryGet
*           function
* @endinternal
*
* @brief   Get per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressHeaderAlterationDebugEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PX_PACKET_TYPE             packetType;
    GT_U32                          entryIndex;
    GT_U32                          haEntryWords[5];/* needed 144 bits , supporting 32*5=160 bits */


    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    portNum                             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    packetType                          = (GT_U32)inArgs[2];
    entryIndex = portNum << 5 | packetType;

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;


    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api get function */
    result = prvCpssPxReadTableEntry(devNum,CPSS_PX_TABLE_PHA_HA_TABLE_E,
        entryIndex,&haEntryWords[0]);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");

    }

    else
    {
        /* pack output arguments to galtis string */
        cpssOsPrintf("haEntryWords[0] = %x\n",  haEntryWords[0]);
        cpssOsPrintf("haEntryWords[1] = %x\n",  haEntryWords[1]);
        cpssOsPrintf("haEntryWords[2] = %x\n",  haEntryWords[2]);
        cpssOsPrintf("haEntryWords[3] = %x\n",  haEntryWords[3]);
        cpssOsPrintf("fwInstructionPointer = %x\n",  haEntryWords[4]);
    }
    return CMD_OK;
}




static GT_U32 gPortNum;
/**
* @internal wrCpssPxEgressTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressTargetPortEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT       infoType;
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT            portInfo;
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC               *commonInfoPtr;

    GT_U32                          actualNumFields;
    GT_U32                          i;
    GT_U32                          tabIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_SW_DEV_NUM)inArgs[0];

    if (gPortNum >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, gPortNum);

    /* call cpss api get function */
    result = cpssPxEgressTargetPortEntryGet(devNum, gPortNum, &infoType, &portInfo);
    if (result != GT_OK && result != GT_NOT_INITIALIZED)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    commonInfoPtr = &portInfo.info_common;

    i = 0;
    inFields[i++] = gPortNum++;

    switch (infoType)
    {
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E:
            inFields[i++]   = commonInfoPtr->tpid;
            inFields[i++]   = commonInfoPtr->pcid;
            inFields[i++]   = commonInfoPtr->egressDelay;
            tabIndex = 0;
            break;
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E:
            tabIndex = 1;
            break;
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E:
            tabIndex = 2;
            break;
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E:
            inFields[i++]   = commonInfoPtr->tpid;
            tabIndex = 3;
            break;
        default:
            tabIndex = 4;
            break;
    }

    actualNumFields = i;
    /* pack and output table fields */
    for (i = 0; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%d%f", tabIndex);

    return CMD_OK;
}

/* wrapper for cpssPxEgressTargetPortEntryGet */
CMD_STATUS wrCpssPxEgressTargetPortEntryGetFirst
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

    gPortNum = 0;

    return wrCpssPxEgressTargetPortEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxEgressTargetPortEntryGet */
CMD_STATUS wrCpssPxEgressTargetPortEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxEgressTargetPortEntryGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssPxEgressTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressTargetPortEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT           infoType;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT                portInfo;
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC               *commonInfoPtr;
    GT_U32                                              i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&portInfo, 0, sizeof(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT));

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    infoType                            = (CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT)inArgs[1] + CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E;


    commonInfoPtr = &portInfo.info_common;

    i = 0;
    portNum                             = (GT_PHYSICAL_PORT_NUM)inFields[i++];
    switch (infoType)
    {
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E:
            commonInfoPtr->tpid            = (GT_U16)inFields[i++];
            commonInfoPtr->pcid            = (GT_U32)inFields[i++];
            commonInfoPtr->egressDelay     = (GT_U32)inFields[i++];
            break;
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E:
            commonInfoPtr->tpid            = (GT_U16)inFields[i++];
            break;
        default:
            break;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxEgressTargetPortEntrySet(devNum, portNum, infoType, &portInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxEgressSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressSourcePortEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT           infoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT                portInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC              *info_802_1brPtr;
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC                  *dsaInfoPtr;
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC                  *info_evbPtr;
    CPSS_PX_VLAN_TAG_STC                                *vlanTagPtr;


    GT_U32                                              i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdOsMemSet(&portInfo, 0, sizeof(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT));

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];
    infoType                            = (CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT)inArgs[1] + CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E;

    i = 0;
    portNum                             = (GT_PHYSICAL_PORT_NUM)inFields[i++];
    switch (infoType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
            info_802_1brPtr = &portInfo.info_802_1br;
            info_802_1brPtr->pcid = (GT_U32)inFields[i++];
            info_802_1brPtr->srcPortInfo.srcFilteringVector = (GT_U32)inFields[i++];
            info_802_1brPtr->upstreamPort = (GT_BOOL)inFields[i++];
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            dsaInfoPtr = &portInfo.info_dsa;
            dsaInfoPtr->srcPortNum = (GT_U32)inFields[i++];
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E:
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E:
            info_802_1brPtr = &portInfo.info_802_1br;
            vlanTagPtr = &portInfo.info_802_1br.srcPortInfo.vlanTag;
            info_802_1brPtr->pcid = (GT_U32)inFields[i++];
            info_802_1brPtr->upstreamPort = (GT_BOOL)inFields[i++];
            vlanTagPtr->vid = (GT_U32)inFields[i++];
            vlanTagPtr->dei = (GT_U32)inFields[i++];
            vlanTagPtr->pcp = (GT_U32)inFields[i++];
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            info_evbPtr = &portInfo.info_evb;
            info_evbPtr->vid = (GT_U32)inFields[i++];
            break;
        default:
            break;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxEgressSourcePortEntrySet(devNum, portNum, infoType, &portInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxEgressSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressSourcePortEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                           result;
    GT_SW_DEV_NUM                                       devNum;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT           infoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT                portInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC              *info_802_1brPtr;
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC                  *dsaInfoPtr;
    GT_U32                                              actualNumFields;
    GT_U32                                              i;
    GT_U32                                              tabIndex;
    CPSS_PX_VLAN_TAG_STC                                *vlanTagPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (gPortNum >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    cmdOsMemSet(&portInfo, 0, sizeof(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT));

    /* map input arguments to locals */
    devNum                              = (GT_SW_DEV_NUM)inArgs[0];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, gPortNum);

    /* call cpss api function */
    result = cpssPxEgressSourcePortEntryGet(devNum, gPortNum, &infoType, &portInfo);
    if (result != GT_OK && result != GT_NOT_INITIALIZED)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    i = 0;
    inFields[i++] = gPortNum++;

    switch (infoType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
            info_802_1brPtr = &portInfo.info_802_1br;
            inFields[i++] = info_802_1brPtr->pcid;
            inFields[i++] = info_802_1brPtr->srcPortInfo.srcFilteringVector;
            inFields[i++] = info_802_1brPtr->upstreamPort;
            tabIndex = 0;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E:
            info_802_1brPtr = &portInfo.info_802_1br;
            vlanTagPtr = &portInfo.info_802_1br.srcPortInfo.vlanTag;
            inFields[i++] = info_802_1brPtr->pcid;
            inFields[i++] = info_802_1brPtr->upstreamPort;
            inFields[i++] = vlanTagPtr->vid;
            inFields[i++] = vlanTagPtr->dei;
            inFields[i++] = vlanTagPtr->pcp;
            tabIndex = 1;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            dsaInfoPtr = &portInfo.info_dsa;
            inFields[i++] = dsaInfoPtr->srcPortNum;
            tabIndex = 2;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E:
            tabIndex = 3;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            tabIndex = 4;
            break;
        default:
            tabIndex = 5;
            break;
    }

    actualNumFields = i;
    /* pack and output table fields */
    for (i = 0; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%d%f", tabIndex);

    return CMD_OK;
}

/* wrapper for cpssPxEgressSourcePortEntryGet */
CMD_STATUS wrCpssPxEgressSourcePortEntryGetFirst
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

    gPortNum = 0;

    return wrCpssPxEgressSourcePortEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxEgressSourcePortEntryGet */
CMD_STATUS wrCpssPxEgressSourcePortEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxEgressSourcePortEntryGet(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssPxEgressVlanTagStateEntrySet function
* @endinternal
*
* @brief   Sets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressVlanTagStateEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          vlanId;
    CPSS_PX_PORTS_BMP               portsTagging;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    vlanId                                  = (GT_U32)inArgs[1];
    portsTagging                            = (CPSS_PX_PORTS_BMP)inArgs[2];

    /* call cpss api function */
    result = cpssPxEgressVlanTagStateEntrySet(devNum, vlanId, portsTagging);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxEgressVlanTagStateEntryGet function
* @endinternal
*
* @brief   Gets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxEgressVlanTagStateEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          vlanId;
    CPSS_PX_PORTS_BMP               portsTagging;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    vlanId                                  = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxEgressVlanTagStateEntryGet(devNum, vlanId, &portsTagging);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portsTagging);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName, funcReference, funcArgs, funcFields */
    { "cpssPxEgressBypassModeSet",
       &wrCpssPxEgressBypassModeSet,
       3,  0 }
   ,{ "cpssPxEgressBypassModeGet",
        &wrCpssPxEgressBypassModeGet,
       1,  0 }
   ,{ "cpssPxEgressTimestampModeSet",
       &wrCpssPxEgressTimestampModeSet,
       1,  2 }
   ,{ "cpssPxEgressTimestampModeGetFirst",
       &wrCpssPxEgressTimestampModeGetFirst,
       1,  0 }
   ,{ "cpssPxEgressTimestampModeGetNext",
       &wrCpssPxEgressTimestampModeGetNext,
       1,  0 }
   ,{ "cpssPxEgressHa802_1BrEntrySet",
       &wrCpssPxEgressHeaderAlteration802_1BrEntrySet,
       3, 21}
   ,{ "cpssPxEgressHa802_1BrEntryGetFirst",
       &wrCpssPxEgressHeaderAlteration802_1BrEntryGetFirst,
       3,  0 }
   ,{ "cpssPxEgressHa802_1BrEntryGetNext",
       &wrCpssPxEgressHeaderAlteration802_1BrEntryGetNext,
       3,  0 }
   ,{ "cpssPxEgressHa802_1BrExtEntrySet",
       &wrCpssPxEgressHeaderAlteration802_1BrExtEntrySet,
       3,  9}
   ,{ "cpssPxEgressHa802_1BrExtEntryGetFirst",
       &wrCpssPxEgressHeaderAlteration802_1BrExtEntryGetFirst,
       3,  0 }
   ,{ "cpssPxEgressHa802_1BrExtEntryGetNext",
       &wrCpssPxEgressHeaderAlteration802_1BrExtEntryGetNext,
       3,  0 }
   ,{ "cpssPxEgressHaDsaEntrySet",
       &wrCpssPxEgressHeaderAlterationDsaEntrySet,
       3, 20}
   ,{ "cpssPxEgressHaDsaEntryGetFirst",
       &wrCpssPxEgressHeaderAlterationDsaEntryGetFirst,
       3,  0 }
   ,{ "cpssPxEgressHaDsaEntryGetNext",
       &wrCpssPxEgressHeaderAlterationDsaEntryGetNext,
       3,  0 }
   ,{ "cpssPxEgressHeaderAlterationDebugEntrySet",
       &wrCpssPxEgressHeaderAlterationDebugEntrySet,
      8,  0 }
   ,{ "cpssPxEgressHeaderAlterationDebugEntryGet",
       &wrCpssPxEgressHeaderAlterationDebugEntryGet,
      3,  0 }
   ,{ "cpssPxEgressTargetPortEntrySet",
       &wrCpssPxEgressTargetPortEntrySet,
       2,  4 }
   ,{ "cpssPxEgressTargetPortEntryGetFirst",
       &wrCpssPxEgressTargetPortEntryGetFirst,
       1,  0 }
   ,{ "cpssPxEgressTargetPortEntryGetNext",
       &wrCpssPxEgressTargetPortEntryGetNext,
       1,  0 }
   ,{ "cpssPxEgressSourcePortEntrySet",
       &wrCpssPxEgressSourcePortEntrySet,
       2,  6 }
   ,{ "cpssPxEgressSourcePortEntryGetFirst",
       &wrCpssPxEgressSourcePortEntryGetFirst,
       1,  0 }
   ,{ "cpssPxEgressSourcePortEntryGetNext",
       &wrCpssPxEgressSourcePortEntryGetNext,
       1,  0 }
   ,{  "cpssPxEgressVlanTagStateEntrySet",
       &wrCpssPxEgressVlanTagStateEntrySet,
       3,  0 }
   ,{ "cpssPxEgressVlanTagStateEntryGet",
       &wrCpssPxEgressVlanTagStateEntryGet,
       2,  0 }

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxEgress function
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
GT_STATUS cmdLibInitCpssPxEgress
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



