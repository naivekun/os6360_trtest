/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapCpssPxIngress.c
*
* DESCRIPTION:
*       Wrapper functions for
*           cpss/Px/ingress/cpssPxIngress.h API's
*
*       DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


/*******************************************************************************
* cpssPxIngressTpidEntrySet
*
* DESCRIPTION:
*       Set global TPID table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       entryIndex      - entry index for TPID table (APPLICABLE RANGES: 0..3).
*       tpidEntryPtr    - (pointer to) global TPID entry
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressTpidEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          entryIndex;
    CPSS_PX_INGRESS_TPID_ENTRY_STC  tpidEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex                              = (GT_U32)inArgs[1];
    tpidEntry.val                           = (GT_U16)inArgs[2];
    tpidEntry.size                          = (GT_U32)inArgs[3];
    tpidEntry.valid                         = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssPxIngressTpidEntrySet(devNum, entryIndex, &tpidEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressTpidEntryGet
*
* DESCRIPTION:
*       Get global TPID table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       entryIndex      - entry index for TPID table (APPLICABLE RANGES: 0..3).
*
* OUTPUTS:
*       tpidEntryPtr    - (pointer to) global TPID entry
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressTpidEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          entryIndex;
    CPSS_PX_INGRESS_TPID_ENTRY_STC  tpidEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex                              = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressTpidEntryGet(devNum, entryIndex, &tpidEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", tpidEntry.val, tpidEntry.size, tpidEntry.valid);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressEtherTypeSet
*
* DESCRIPTION:
*       Set global ethertype configuration.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       configType          - type of ethertype to be configured
*       etherType           - EtherType for protocol recognition.
*       valid               - EtherType entry status
*                               GT_TRUE     - EtherType is valid,
*                               GT_FALSE    - EtherType is not valid.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressEtherTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_ETHERTYPE_ENT   configType;
    GT_U16                          etherType;
    GT_BOOL                         valid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    configType                              = (CPSS_PX_INGRESS_ETHERTYPE_ENT)inArgs[1];
    etherType                               = (GT_U16)inArgs[2];
    valid                                   = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssPxIngressEtherTypeSet(devNum, configType, etherType, valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressEtherTypeGet
*
* DESCRIPTION:
*       Get global ethertype configuration.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       configType      - type of ethertype
*
* OUTPUTS:
*       etherTypePtr    - (pointer to) EtherType for protocol recognition.
*       validPtr        - (pointer to) EtherType entry status
*                           GT_TRUE     - EtherType is valid,
*                           GT_FALSE    - EtherType is not valid.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressEtherTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_ETHERTYPE_ENT   configType;
    GT_U16                          etherType;
    GT_BOOL                         valid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    configType                              = (CPSS_PX_INGRESS_ETHERTYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressEtherTypeGet(devNum, configType, &etherType, &valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", etherType, valid);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPortMacDaByteOffsetSet
*
* DESCRIPTION:
*       Set byte offset of MAC DA on specified port.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number.
*       offset      - byte offset of MAC DA.
*                     Only even values are applicable.
*                     (APPLICABLE RANGES: 0..30)
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortMacDaByteOffsetSet
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
    GT_U32                          offset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    offset                                  = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxIngressPortMacDaByteOffsetSet(devNum, portNum, offset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPortMacDaByteOffsetGet
*
* DESCRIPTION:
*       Get byte offset of MAC DA for specified port.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number.
*
*
* OUTPUTS:
*       offsetPtr   - (pointer to) byte offset of MAC DA.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortMacDaByteOffsetGet
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
    GT_U32                          offset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxIngressPortMacDaByteOffsetGet(devNum, portNum, &offset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", offset);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPortPacketTypeKeySet
*
* DESCRIPTION:
*       Set port packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum                  - device number.
*       portNum                 - physical port number.
*       portKeyPtr              - (pointer to) port packet type key.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortPacketTypeKeySet
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
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    GT_U32                          i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inFields[0];
    portKey.srcPortProfile                  = (GT_U32)inFields[1];

    i = 2;
    portKey.portUdbPairArr[0].udbAnchorType = (CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT)inFields[i++];
    portKey.portUdbPairArr[0].udbByteOffset = (GT_U32)inFields[i++];
    portKey.portUdbPairArr[1].udbAnchorType = (CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT)inFields[i++];
    portKey.portUdbPairArr[1].udbByteOffset = (GT_U32)inFields[i++];
    portKey.portUdbPairArr[2].udbAnchorType = (CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT)inFields[i++];
    portKey.portUdbPairArr[2].udbByteOffset = (GT_U32)inFields[i++];
    portKey.portUdbPairArr[3].udbAnchorType = (CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT)inFields[i++];
    portKey.portUdbPairArr[3].udbByteOffset = (GT_U32)inFields[i++];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api set function */
    result = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static GT_U32 gPortNum;

/*******************************************************************************
* cpssPxIngressPortPacketTypeKeyGet
*
* DESCRIPTION:
*       Get packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       portNum             - physical port number.
*
* OUTPUTS:
*       portKeyPtr          - (pointer to) packet type key.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortPacketTypeKeyGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    GT_U32                          i;
    GT_U32                          actualNumFields;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gPortNum >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, gPortNum);

    /* call cpss api get function */
    result = cpssPxIngressPortPacketTypeKeyGet(devNum, gPortNum, &portKey);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, ""); 
        return CMD_OK;
    }

    i = 0;
    inFields[i++] = gPortNum++;
    inFields[i++] = portKey.srcPortProfile;
    inFields[i++] = portKey.portUdbPairArr[0].udbAnchorType;
    inFields[i++] = portKey.portUdbPairArr[0].udbByteOffset;
    inFields[i++] = portKey.portUdbPairArr[1].udbAnchorType;
    inFields[i++] = portKey.portUdbPairArr[1].udbByteOffset;
    inFields[i++] = portKey.portUdbPairArr[2].udbAnchorType;
    inFields[i++] = portKey.portUdbPairArr[2].udbByteOffset;
    inFields[i++] = portKey.portUdbPairArr[3].udbAnchorType;
    inFields[i++] = portKey.portUdbPairArr[3].udbByteOffset;

    actualNumFields = i;
    /* pack and output table fields */
    for (i = 0; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressPortPacketTypeKeyGet */
CMD_STATUS wrCpssPxIngressPortPacketTypeKeyGetFirst
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

    /* map input fields to global */
    gPortNum = 0;

    return wrCpssPxIngressPortPacketTypeKeyGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxIngressPortPacketTypeKeyGet */
CMD_STATUS wrCpssPxIngressPortPacketTypeKeyGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressPortPacketTypeKeyGet(inArgs, inFields, numFields, outArgs);
}

/*******************************************************************************
* cpssPxIngressPacketTypeKeyEntrySet
*
* DESCRIPTION:
*       Set packet type key table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       packetType          - index of the packet type key in the table.
*                             (APPLICABLE RANGES: 0..31)
*       keyDataPtr          - (pointer to) 17 bytes packet type key data.
*       keyMaskPtr          - (pointer to) 17 bytes packet type key mask.
*                             The packet type key mask is AND styled one.
*                             Mask bit's 0 means don't care bit (corresponding bit in
*                             the pattern is not using in the TCAM like lookup).
*                             Mask bit's 1 means that corresponding bit in the
*                             pattern is using in the TCAM like lookup.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PACKET_TYPE                         packetType;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyFormayPtr;
    GT_ETHERADDR                                mac;    /* MAC address */
    GT_BYTE_ARRY                                bArr;   /* Binary data */
    GT_U32                          i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    packetType                              = (CPSS_PX_PACKET_TYPE)inFields[0];

    result = cpssPxIngressPacketTypeKeyEntryGet(devNum, packetType, &keyData, &keyMask);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, ""); 
        return CMD_OK;
    }

    if(inFields[1] == 0) /* data */
    {
        keyFormayPtr = &keyData;
    }
    else                 /* mask */
    {
        keyFormayPtr = &keyMask;
    }

    i = 2;
    galtisMacAddr(&mac,  (GT_U8*)inFields[i++]);
    cmdOsMemCpy(keyFormayPtr->macDa.arEther, mac.arEther, 6);

    keyFormayPtr->etherType     =    (GT_U16)inFields[i++];
    keyFormayPtr->isLLCNonSnap  =    (GT_BOOL)inFields[i++];
    keyFormayPtr->profileIndex  =    (GT_U32)inFields[i++];

    galtisBArray(&bArr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(&keyFormayPtr->udbPairsArr[0].udb, bArr.data, bArr.length);

    galtisBArray(&bArr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(&keyFormayPtr->udbPairsArr[1].udb, bArr.data, bArr.length);

    galtisBArray(&bArr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(&keyFormayPtr->udbPairsArr[2].udb, bArr.data, bArr.length);

    galtisBArray(&bArr, (GT_U8*)inFields[i++]);
    cmdOsMemCpy(&keyFormayPtr->udbPairsArr[3].udb, bArr.data, bArr.length);

    galtisBArrayFree(&bArr);

    keyFormayPtr->ip2meIndex = (GT_U32)inFields[i++];

    /* call cpss api set function */
    result = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static CPSS_PX_PACKET_TYPE                          gPacketType;
static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC   gKeyData;
static CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC   gKeyMask;
static GT_BOOL                                      bReadEntry = GT_TRUE;
/*******************************************************************************
* cpssPxIngressPacketTypeKeyEntryGet
*
* DESCRIPTION:
*       Get packet type key table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       entryIndex          - index of the packet type key in the table.
*                             (APPLICABLE RANGES: 0..31)
*
* OUTPUTS:
*       keyDataPtr          - (pointer to) 17 bytes packet type key pattern.
*       keyMaskPtr          - (pointer to) 17 bytes packet type key mask.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  * keyPtr;
    GT_U32                          dataType;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];

    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    if (bReadEntry == GT_TRUE)
    {
        /* call cpss api get function */
        result = cpssPxIngressPacketTypeKeyEntryGet(devNum, gPacketType, &gKeyData, &gKeyMask);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }
        /* Parse key data */
        keyPtr = &gKeyData;
        dataType = 0;
    }
    else
    {
        /* Parse key mask */
        keyPtr = &gKeyMask;
        dataType = 1;
    }

    /* Tab-number, packet type and dataType(mask/pattern) before fields */
    fieldOutput(
        "%d%d", gPacketType, dataType);
    fieldOutputSetAppendMode();

    fieldOutput(
        "%s%d%d%d%s%s%s%s%d",
        galtisByteArrayToString(keyPtr->macDa.arEther, 6),
        keyPtr->etherType, 
        keyPtr->isLLCNonSnap, 
        keyPtr->profileIndex, 
        galtisByteArrayToString(keyPtr->udbPairsArr[0].udb, 2),
        galtisByteArrayToString(keyPtr->udbPairsArr[1].udb, 2),
        galtisByteArrayToString(keyPtr->udbPairsArr[2].udb, 2),
        galtisByteArrayToString(keyPtr->udbPairsArr[3].udb, 2),
        keyPtr->ip2meIndex);
    fieldOutputSetAppendMode();

    /* Inverse read action */
    bReadEntry = !bReadEntry;
    if (bReadEntry == GT_TRUE)
    {
        /* Increnment packet type before next read */
        gPacketType++;
    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressPacketTypeKeyEntryGet */
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntryGetFirst
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

    return wrCpssPxIngressPacketTypeKeyEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxIngressPortPacketTypeKeyGet */
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressPacketTypeKeyEntryGet(inArgs, inFields, numFields, outArgs);
}

/*******************************************************************************
* cpssPxIngressPortMapPacketTypeFormatEntrySet
*
* DESCRIPTION:
*       Set Source/Destination information for packet type in the format table,
*       used to extract the information from the packet forwarding tag (DSA or E-Tag).
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       tableType           - Source/Destination port map table.
*       packetType          - index of the packet type format.
*                             in the Source/Destination format table.
*                             (APPLICABLE RANGES: 0..31)
*       packetTypeFormatPtr - (pointer to) Source/Destination packet type format
*                             in the Source/Destination format table.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortMapPacketTypeFormatEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType;
    CPSS_PX_PACKET_TYPE                                 packetType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *frmPtr = &packetTypeFormat;
    GT_U32                                              i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    tableType                               = (CPSS_PX_PACKET_TYPE)inArgs[1];
    /* map input fields to locals */
    packetType                              = (CPSS_PX_PACKET_TYPE)inFields[0];

    i = 1;
    frmPtr->bitFieldArr[0].byteOffset       = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[0].startBit         = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[0].numBits          = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[1].byteOffset       = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[1].startBit         = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[1].numBits          = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[2].byteOffset       = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[2].startBit         = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[2].numBits          = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[3].byteOffset       = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[3].startBit         = (GT_U32)inFields[i++];
    frmPtr->bitFieldArr[3].numBits          = (GT_U32)inFields[i++];
    frmPtr->indexConst                      = (GT_32)inFields[i++];
    frmPtr->indexMax                        = (GT_U32)inFields[i++];

    /* call cpss api set function */
    result = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType, packetType, &packetTypeFormat);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPortMapPacketTypeFormatEntryGet
*
* DESCRIPTION:
*       Get Source/Destination information for packet type in the format table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       tableType           - Source/Destination port map table.
*       packetType          - index of the packet type format
*                             in the Source/Destination format table.
*                             (APPLICABLE RANGES: 0..31)
*
* OUTPUTS:
*       packetTypeFormatPtr - (pointer to) Source/Destination packet type format
*                             in the Source/Destination format table.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortMapPacketTypeFormatEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *frmPtr;
    GT_U32 actualNumFields, i;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_SW_DEV_NUM)inArgs[0];
    tableType           = (CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT)inArgs[1];

    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api set function */
    result = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum, tableType, gPacketType, &packetTypeFormat);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    frmPtr = &packetTypeFormat;

    i = 0;
    inFields[i++] = gPacketType++;
    inFields[i++] = frmPtr->bitFieldArr[0].byteOffset;
    inFields[i++] = frmPtr->bitFieldArr[0].startBit;
    inFields[i++] = frmPtr->bitFieldArr[0].numBits;
    inFields[i++] = frmPtr->bitFieldArr[1].byteOffset;
    inFields[i++] = frmPtr->bitFieldArr[1].startBit;
    inFields[i++] = frmPtr->bitFieldArr[1].numBits;
    inFields[i++] = frmPtr->bitFieldArr[2].byteOffset;
    inFields[i++] = frmPtr->bitFieldArr[2].startBit;
    inFields[i++] = frmPtr->bitFieldArr[2].numBits;
    inFields[i++] = frmPtr->bitFieldArr[3].byteOffset;
    inFields[i++] = frmPtr->bitFieldArr[3].startBit;
    inFields[i++] = frmPtr->bitFieldArr[3].numBits;
    inFields[i++] = frmPtr->indexConst;
    inFields[i++] = frmPtr->indexMax;

    actualNumFields = i;
    /* pack and output table fields */
    for (i = 0; i < actualNumFields; i++)
    {
        fieldOutput("%d", inFields[i]);
        fieldOutputSetAppendMode();
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressPortMapPacketTypeFormatEntryGet */
CMD_STATUS wrCpssPxIngressPortMapPacketTypeFormatEntryGetFirst
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

    return wrCpssPxIngressPortMapPacketTypeFormatEntryGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxIngressPortMapPacketTypeFormatEntryGet */
CMD_STATUS wrCpssPxIngressPortMapPacketTypeFormatEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressPortMapPacketTypeFormatEntryGet(inArgs, inFields, numFields, outArgs);
}

/*******************************************************************************
* cpssPxIngressPortMapEntrySet
*
* DESCRIPTION:
*       Set Source/Destination port map table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       tableType           - Source/Destination port map table.
*       entryIndex          - port map entry index.
*                             Index calculated by packet type source/destination format.
*                             Destination table -
*                               (APPLICABLE RANGES: 0..8191)
*                             Source table -
*                               (APPLICABLE RANGES: 0..4095)
*       portsBmp            - the bitmap of ports.
* 
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortMapEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT  tableType;
    GT_U32                          entryIndex;
    CPSS_PX_PORTS_BMP               portsBmp;
    GT_BOOL                         bypassLagDesignatedBitmap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    tableType                               = (CPSS_PX_PACKET_TYPE)inArgs[1];
    /* map input fields to locals */
    entryIndex                              = (GT_U32)inFields[0];
    portsBmp                                = (CPSS_PX_PORTS_BMP)inFields[1];
    bypassLagDesignatedBitmap               = (GT_BOOL)inFields[2];

    /* call cpss api set function */
    result = cpssPxIngressPortMapEntrySet(devNum, tableType, entryIndex, portsBmp, bypassLagDesignatedBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static GT_U32 gCurrentIndex;
/*******************************************************************************
* cpssPxIngressPortMapEntryGet
*
* DESCRIPTION:
*       Get Source/Destination port map table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       tableType           - Source/Destination port map table.
*       entryIndex          - port map entry index.
*                             Index calculated by packet type source/destination format.
*                             Destination table -
*                               (APPLICABLE RANGES: 0..8191)
*                             Source table -
*                               (APPLICABLE RANGES: 0..4095)
*
* OUTPUTS:
*       portsBmpPtr         - (pointer to) the bitmap of ports.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortMapEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_PORTS_BMP               portsBmp;
    GT_BOOL                         bypassLagDesignatedBitmap;
    GT_U32                          startIndex;
    GT_U32                          endIndex;
    GT_U32                          totalEntries;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_SW_DEV_NUM)inArgs[0];
    tableType           = (CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT)inArgs[1];
    startIndex          = (GT_U32)inArgs[2];
    totalEntries        = (GT_U32)inArgs[3];

    endIndex = startIndex + totalEntries;

    if (gCurrentIndex >= endIndex)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api set function */
    result = cpssPxIngressPortMapEntryGet(devNum, tableType, gCurrentIndex, &portsBmp, &bypassLagDesignatedBitmap);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput("%d%d%d", gCurrentIndex, portsBmp, bypassLagDesignatedBitmap);
    fieldOutputSetAppendMode();

    /* Increnment current index before next read */
    gCurrentIndex++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressPortMapEntryGet */
CMD_STATUS wrCpssPxIngressPortMapEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32 maxEntries;
    GT_U32 totalEntries;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to global */
    tableType           = (CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT)inArgs[1];
    maxEntries = 0x2000;
    if (tableType == CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E)
    {
        maxEntries = 0x1000;
    }
    gCurrentIndex = (GT_U32)inArgs[2];
    totalEntries  = (GT_U32)inArgs[3];

    if ((gCurrentIndex + totalEntries) > maxEntries)
    {
        /* Index is out of range */
        galtisOutput(outArgs, GT_OUT_OF_RANGE, "%d", -1);
        return CMD_AGENT_ERROR;
    }

    return wrCpssPxIngressPortMapEntryGet(inArgs, inFields, numFields, outArgs); 
}

/* wrapper for cpssPxIngressPortMapEntryGet */
CMD_STATUS wrCpssPxIngressPortMapEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressPortMapEntryGet(inArgs, inFields, numFields, outArgs);
}

/*******************************************************************************
* cpssPxIngressPortTargetEnableSet
*
* DESCRIPTION:
*       Enable/disable forwarding to the target port.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       portNum             - physical port number.
*       enable              - enable or disable forwarding to the target port.
*                               GT_TRUE - Enables packet forwarding to the target port,
*                               GT_FALSE - Filters all traffic to the target port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortTargetEnableSet
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
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inFields[0];
    enable                                  = (GT_BOOL)inFields[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxIngressPortTargetEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPortTargetEnableGet
*
* DESCRIPTION:
*       Get state of the forwarding target port.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       portNum             - physical port number.
*
* OUTPUTS:
*       enablePtr           - (pointer to) state of the target port.
*                               GT_TRUE - Enables packet forwarding to the target port,
*                               GT_FALSE - Filters all traffic to the target port.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortTargetEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gPortNum >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, gPortNum);

    /* call cpss api function */
    result = cpssPxIngressPortTargetEnableGet(devNum, gPortNum, &enable);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput("%d%d", gPortNum, enable);
    fieldOutputSetAppendMode();

    /* Increnment port before next read */
    gPortNum++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressPortTargetEnableGet */
CMD_STATUS wrCpssPxIngressPortTargetEnableGetFirst
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
    return wrCpssPxIngressPortTargetEnableGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxIngressPortTargetEnableGet */
CMD_STATUS wrCpssPxIngressPortTargetEnableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressPortTargetEnableGet(inArgs, inFields, numFields, outArgs);
}

/*******************************************************************************
* cpssPxIngressPacketTypeErrorGet
*
* DESCRIPTION:
*       Get unmatched packet type error counter and key.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*
* OUTPUTS:
*       errorCounterPtr     - (pointer to) unmatched packet type counter.
*       errorKeyPtr         - (pointer to) unmatched packet type key.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPacketTypeErrorGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          errorCounter;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  errorKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyPtr = &errorKey;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Clear structure before read */
    cpssOsMemSet(keyPtr, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxIngressPacketTypeErrorGet(devNum, &errorCounter, keyPtr);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, ""); 
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, 
        "%d%s%d%d%d%s%s%s%s",
        errorCounter,
        galtisByteArrayToString(keyPtr->macDa.arEther, 6),
        keyPtr->etherType, 
        keyPtr->isLLCNonSnap, 
        keyPtr->profileIndex, 
        galtisByteArrayToString(keyPtr->udbPairsArr[0].udb, 2),
        galtisByteArrayToString(keyPtr->udbPairsArr[1].udb, 2),
        galtisByteArrayToString(keyPtr->udbPairsArr[2].udb, 2),
        galtisByteArrayToString(keyPtr->udbPairsArr[3].udb, 2));

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressForwardingPortMapExceptionSet
*
* DESCRIPTION:
*       Set exception Source/Destination forwarding port map.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       tableType           - Source/Destination port map table.
*       portsBmp            - the bitmap of ports.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressForwardingPortMapExceptionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_PORTS_BMP               portsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    tableType                               = (CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT)inArgs[1];
    portsBmp                                = (CPSS_PX_PORTS_BMP)inArgs[2];

    /* call cpss api function */
    result = cpssPxIngressForwardingPortMapExceptionSet(devNum, tableType, portsBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressForwardingPortMapExceptionGet
*
* DESCRIPTION:
*       Get exception Source/Destination port map.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       tableType           - Source/Destination port map table.
*
* OUTPUTS:
*       portsBmpPtr         - (pointer to) the bitmap of ports.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressForwardingPortMapExceptionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_PORTS_BMP               portsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    tableType                               = (CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressForwardingPortMapExceptionGet(devNum, tableType, &portsBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portsBmp);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPortMapExceptionCounterGet
*
* DESCRIPTION:
*       Get Source/Destination index exception counter.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       tableType           - Source/Destination port map table.
*
* OUTPUTS:
*       errorCounterPtr     - (pointer to) Source/Destination index exception counter.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortMapExceptionCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    GT_U32                          errorCounter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    tableType                               = (CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressPortMapExceptionCounterGet(devNum, tableType, &errorCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", errorCounter);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPacketTypeKeyEntryEnableSet
*
* DESCRIPTION:
*       Enable/disable packet type key table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       packetType          - index of the packet type key in the table.
*                               (APPLICABLE RANGES: 0..31)
*       enable              - Enable or disable packet type key table entry
*                               GT_TRUE - enable packet type key table entry,
*                               GT_FALSE - disable packet type key table entry.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntryEnableSet
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
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    packetType                              = (CPSS_PX_PACKET_TYPE)inFields[0];
    enable                                  = (GT_BOOL)inFields[1];

    /* call cpss api function */
    result = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPacketTypeKeyEntryEnableGet
*
* DESCRIPTION:
*       Get status of the packet type key table entry.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum              - device number.
*       packetType          - index of the packet type key in the table.
*                               (APPLICABLE RANGES: 0..31)
*
* OUTPUTS:
*       enablePtr           - (pointer to) state of the packet type key table entry
*                               GT_TRUE - enable packet type key table entry,
*                               GT_FALSE - disable packet type key table entry.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntryEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    OUT GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gPacketType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api set function */
    result = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum, gPacketType, &enable);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* Tab-number, packet type before fields */
    fieldOutput(
        "%d%d", gPacketType, enable);

    /* Increnment packet type before next read */
    gPacketType++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressPacketTypeKeyEntryEnableGet */
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntryEnableGetFirst
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
    return wrCpssPxIngressPacketTypeKeyEntryEnableGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxIngressPacketTypeKeyEntryEnableGet */
CMD_STATUS wrCpssPxIngressPacketTypeKeyEntryEnableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressPacketTypeKeyEntryEnableGet(inArgs, inFields, numFields, outArgs);
}

/*******************************************************************************
* cpssPxIngressPortDsaTagEnableSet
*
* DESCRIPTION:
*       Enable/disable the source port for DSA-tagged packets.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number.
*       enable      - enable or disable the source port for DSA-tagged packets
*                       GT_TRUE   - enable the port for DSA-tagged packets,
*                       GT_FALSE  - disable the port for DSA-tagged packets
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortDsaTagEnableSet
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
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inFields[0];
    enable                                  = (GT_BOOL)inFields[1];

    /* call cpss api function */
    result = cpssPxIngressPortDsaTagEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressPortDsaTagEnableGet
*
* DESCRIPTION:
*       Get DSA-tagged packets receiving status on the source port.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - device number.
*       portNum     - physical port number.
*
*
* OUTPUTS:
*       enablePtr   - (pointer to) DSA-tagged packets receiving status on the source port
*                       GT_TRUE   - enable the port for DSA-tagged packets,
*                       GT_FALSE  - disable the port for DSA-tagged packets
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortDsaTagEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    OUT GT_BOOL                     enable;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];

    if (gPortNum >= PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, gPortNum);

    /* call cpss api get function */
    result = cpssPxIngressPortDsaTagEnableGet(devNum, gPortNum, &enable);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, ""); 
        return CMD_OK;
    }

    /* Tab-number, port number before fields */
    fieldOutput(
        "%d%d", gPortNum, enable);

    /* Increnment port before next read */
    gPortNum++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressPortDsaTagEnableGet */
CMD_STATUS wrCpssPxIngressPortDsaTagEnableGetFirst
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
    return wrCpssPxIngressPortDsaTagEnableGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxIngressPortDsaTagEnableGet */
CMD_STATUS wrCpssPxIngressPortDsaTagEnableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressPortDsaTagEnableGet(inArgs, inFields, numFields, outArgs);
}

/*******************************************************************************
* cpssPxIngressPortRedirectSet
*
* DESCRIPTION:
*       Set redirecting of the all packets from ingress port to list of
*       egress ports.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       portNum         - ingress physical port number.
*       targetPortsBmp  - the bitmap of egress ports.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressPortRedirectSet
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
    CPSS_PX_PORTS_BMP               targetPortsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    targetPortsBmp                          = (CPSS_PX_PORTS_BMP)inArgs[2];

    /* call cpss api function */
    result = cpssPxIngressPortRedirectSet(devNum, portNum, targetPortsBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressFilteringEnableSet
*
* DESCRIPTION:
*       Set port filtering enabled state.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - device number.
*       direction   - direction.
*       packetType  - packet type.
*       enable      - enable or disable port filtering
*                       GT_TRUE   - port filtering is enabled
*                       GT_FALSE  - port filtering is disabled
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*   Applicable starting from revision A1.
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressFilteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_DIRECTION_ENT              direction;
    CPSS_PX_PACKET_TYPE             packetType;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    direction  = (CPSS_DIRECTION_ENT)inArgs[1];
    packetType = (CPSS_PX_PACKET_TYPE)inArgs[2];
    enable     = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssPxIngressFilteringEnableSet(devNum, direction, packetType,
        enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressFilteringEnableGet
*
* DESCRIPTION:
*       Get port filtering enabled state.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - device number.
*       direction   - direction: ingress or egress.
*       packetType  - packet type.
*
*
* OUTPUTS:
*       enablePtr   - (pointer to) enabled state
*                       GT_TRUE   - port filtering is enabled
*                       GT_FALSE  - port filtering is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*   Applicable starting from revision A1.
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_DIRECTION_ENT           direction;
    CPSS_PX_PACKET_TYPE             packetType;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    direction  = (CPSS_DIRECTION_ENT)inArgs[1];
    packetType = (CPSS_PX_PACKET_TYPE)inArgs[2];

    /* call cpss api function */
    result = cpssPxIngressFilteringEnableGet(devNum, direction, packetType,
        &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressIp2MeEntrySet
*
* DESCRIPTION:
*       Set IP2ME table entry for IPv4 packets.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       entryIndex      - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
*       ip2meEntryPtr   - (pointer to) IP2ME entry
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*   Applicable starting from revision A1.
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressIpv4ToMeEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntry;
    GT_U32                          entryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];
    ip2meEntry.prefixLength = (GT_U32)inArgs[2];
    ip2meEntry.isIpv6 = GT_FALSE;
    galtisIpAddr(&ip2meEntry.ipAddr.ipv4Addr, (GT_U8*)inArgs[3]);
    ip2meEntry.valid = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssPxIngressIp2MeEntrySet(devNum, entryIndex, &ip2meEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressIp2MeEntryGet
*
* DESCRIPTION:
*       Get IP2ME table entry for IPv4 packets.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       entryIndex      - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
*
* OUTPUTS:
*       ip2meEntryPtr   - (pointer to) IP2ME entry.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*   Applicable starting from revision A1.
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressIpv4ToMeEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntry;
    GT_U32                          entryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressIp2MeEntryGet(devNum, entryIndex, &ip2meEntry);

    if (ip2meEntry.isIpv6 == GT_TRUE)
    {
        /* The entry is occupied by IPV6 address - IPV4 not valid */
        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%4b%d", ip2meEntry.prefixLength, 
                 ip2meEntry.ipAddr.ipv4Addr.arIP, ip2meEntry.valid);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressIpv4ToMeEntrySet
*
* DESCRIPTION:
*       Set IP2ME table entry for IPv6 packets.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       entryIndex      - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
*       ip2meEntryPtr   - (pointer to) IP2ME entry
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*   Applicable starting from revision A1.
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressIpv6ToMeEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntry;
    GT_U32                          entryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];
    ip2meEntry.prefixLength = (GT_U32)inArgs[2];
    ip2meEntry.isIpv6 = GT_TRUE;
    galtisIpv6Addr(&ip2meEntry.ipAddr.ipv6Addr, (GT_U8*)inArgs[3]);
    ip2meEntry.valid = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssPxIngressIp2MeEntrySet(devNum, entryIndex, &ip2meEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressIpv4ToMeEntryGet
*
* DESCRIPTION:
*       Get IP2ME table entry for IPv6 packets.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum          - device number.
*       entryIndex      - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
*
* OUTPUTS:
*       ip2meEntryPtr   - (pointer to) IP2ME entry.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_PTR               - on NULL pointer
*       GT_OUT_OF_RANGE          - one of the parameters is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*   Applicable starting from revision A1.
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressIpv6ToMeEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntry;
    GT_U32                          entryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    entryIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressIp2MeEntryGet(devNum, entryIndex, &ip2meEntry);
    if (ip2meEntry.isIpv6 == GT_FALSE)
    {
        /* The entry is occupied by IPV4 address - IPV6 not valid */
        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC));
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%16b%d", ip2meEntry.prefixLength, 
                 ip2meEntry.ipAddr.ipv6Addr.arIP, ip2meEntry.valid);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName, funcReference, funcArgs, funcFields */
    { "cpssPxIngressTpidEntrySet",                    
       &wrCpssPxIngressTpidEntrySet,               
       5,  0 }
   ,{ "cpssPxIngressTpidEntryGet",                    
        &wrCpssPxIngressTpidEntryGet,               
       2,  0 }
   ,{ "cpssPxIngressEtherTypeSet",                    
       &wrCpssPxIngressEtherTypeSet,                
       4,  0 }
   ,{ "cpssPxIngressEtherTypeGet",                    
       &wrCpssPxIngressEtherTypeGet,                
       2,  0 }
   ,{ "cpssPxIngressPortMacDaByteOffsetSet",                    
       &wrCpssPxIngressPortMacDaByteOffsetSet,      
       3,  0 }
   ,{ "cpssPxIngressPortMacDaByteOffsetGet",                    
       &wrCpssPxIngressPortMacDaByteOffsetGet,      
       2,  0 }
   ,{ "cpssPxIngressPortPacketTypeKeySet",
       &wrCpssPxIngressPortPacketTypeKeySet, 
       1, 10 }
   ,{ "cpssPxIngressPortPacketTypeKeyGetFirst",
       &wrCpssPxIngressPortPacketTypeKeyGetFirst,
       1, 0 }
   ,{ "cpssPxIngressPortPacketTypeKeyGetNext",
       &wrCpssPxIngressPortPacketTypeKeyGetNext,
       1, 0 }
   ,{ "cpssPxIngressPacketTypeKeyEntrySet",
       &wrCpssPxIngressPacketTypeKeyEntrySet, 
       1, 11 }
   ,{ "cpssPxIngressPacketTypeKeyEntryGetFirst",
       &wrCpssPxIngressPacketTypeKeyEntryGetFirst, 
       1, 0 }
   ,{ "cpssPxIngressPacketTypeKeyEntryGetNext",
       &wrCpssPxIngressPacketTypeKeyEntryGetNext, 
       1, 0 }
   ,{ "cpssPxIngressPortMapPktTypeFormatEntrySet",
       &wrCpssPxIngressPortMapPacketTypeFormatEntrySet,
       2, 15 }
   ,{ "cpssPxIngressPortMapPktTypeFormatEntryGetFirst",
       &wrCpssPxIngressPortMapPacketTypeFormatEntryGetFirst,
       2, 0 }
   ,{ "cpssPxIngressPortMapPktTypeFormatEntryGetNext",
       &wrCpssPxIngressPortMapPacketTypeFormatEntryGetNext,
       2, 0 }
   ,{ "cpssPxIngressPortMapEntrySet",
       &wrCpssPxIngressPortMapEntrySet,
       2, 3 }
   ,{ "cpssPxIngressPortMapEntryGetFirst",
       &wrCpssPxIngressPortMapEntryGetFirst,
       4, 0 }
   ,{ "cpssPxIngressPortMapEntryGetNext",
       &wrCpssPxIngressPortMapEntryGetNext,
       4, 0 }
   ,{ "cpssPxIngressPortTargetEnableSet",
       &wrCpssPxIngressPortTargetEnableSet,
       1,  2 }
   ,{ "cpssPxIngressPortTargetEnableGetFirst",
       &wrCpssPxIngressPortTargetEnableGetFirst,
       1,  0 }
   ,{ "cpssPxIngressPortTargetEnableGetNext",
       &wrCpssPxIngressPortTargetEnableGetNext,
       1,  0 }
   ,{ "cpssPxIngressPacketTypeErrorGet",
       &wrCpssPxIngressPacketTypeErrorGet,
       1,  0 }
   ,{ "cpssPxIngressForwardingPortMapExceptionSet",
       &wrCpssPxIngressForwardingPortMapExceptionSet,
       3,  0 }
   ,{ "cpssPxIngressForwardingPortMapExceptionGet",
       &wrCpssPxIngressForwardingPortMapExceptionGet,
       2,  0 }
   ,{ "cpssPxIngressPortMapExceptionCounterGet",
       &wrCpssPxIngressPortMapExceptionCounterGet,
       2,  0 }
   ,{ "cpssPxIngressPacketTypeKeyEntryEnableSet",
       &wrCpssPxIngressPacketTypeKeyEntryEnableSet,
       1,  2 }
   ,{ "cpssPxIngressPacketTypeKeyEntryEnableGetFirst",
       &wrCpssPxIngressPacketTypeKeyEntryEnableGetFirst,
       1,  0 }
   ,{ "cpssPxIngressPacketTypeKeyEntryEnableGetNext",
       &wrCpssPxIngressPacketTypeKeyEntryEnableGetNext,
       1,  0 }
   ,{ "cpssPxIngressPortDsaTagEnableSet",                    
       &wrCpssPxIngressPortDsaTagEnableSet,      
       1,  2 }
   ,{ "cpssPxIngressPortDsaTagEnableGetFirst",                    
       &wrCpssPxIngressPortDsaTagEnableGetFirst,      
       1,  0 }
   ,{ "cpssPxIngressPortDsaTagEnableGetNext",
       &wrCpssPxIngressPortDsaTagEnableGetNext,      
       1,  0 }
   ,{ "cpssPxIngressPortRedirectSet",
       &wrCpssPxIngressPortRedirectSet,
       3,  0 }
   ,{ "cpssPxIngressFilteringEnableSet",
       &wrCpssPxIngressFilteringEnableSet,
       4,  0 }
   ,{ "cpssPxIngressFilteringEnableGet",
       &wrCpssPxIngressFilteringEnableGet,
       3,  0 }
   ,{ "cpssPxIngressIpv4ToMeEntrySet",
       &wrCpssPxIngressIpv4ToMeEntrySet,
       5,  0 }
   ,{ "cpssPxIngressIpv4ToMeEntryGet",
       &wrCpssPxIngressIpv4ToMeEntryGet,
       2,  0 }
   ,{ "cpssPxIngressIpv6ToMeEntrySet",
       &wrCpssPxIngressIpv6ToMeEntrySet,
       5,  0 }
   ,{ "cpssPxIngressIpv6ToMeEntryGet",
       &wrCpssPxIngressIpv6ToMeEntryGet,
       2,  0 }
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/*******************************************************************************
* cmdLibInitCpssPxIngress
*
* DESCRIPTION:
*     Library database initialization function.
*
* INPUTS:
*     none
*
* OUTPUTS:
*     none
*
* RETURNS:
*     GT_OK   - on success.
*     GT_FAIL - on failure.
*
* COMMENTS:
*     none
*
*******************************************************************************/
GT_STATUS cmdLibInitCpssPxIngress
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



