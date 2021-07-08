/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapCpssPxIngressHash.c
*
* DESCRIPTION:
*       Wrapper functions for
*           cpss/Px/ingress/cpssPxIngressHash.h API's
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

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


/*******************************************************************************
* cpssPxIngressHashUdeEthertypeSet
*
* DESCRIPTION:
*       Set user-defined ethertypes for hash packet classificator.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       hashPacketType      - hash packet type. (APPLICABLE VALUES:
*                              CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                              CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
*       etherType           - ethertype.
*       valid               - validity of the record.
*                               GT_TRUE  - ethertype is valid.
*                               GT_FALSE - ethertype is invalid.
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
CMD_STATUS wrCpssPxIngressHashUdeEthertypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    GT_U16                               etherType;
    GT_BOOL                              valid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    hashPacketType  = (CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)inArgs[1];
    etherType       = (GT_U16)inArgs[2];
    valid           = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssPxIngressHashUdeEthertypeSet(
        devNum, hashPacketType, etherType, valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashUdeEthertypeGet
*
* DESCRIPTION:
*       Get user-defined ethertypes (in addition to predefined types) for hash packet classificator.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       hashPacketType      - hash packet type. (APPLICABLE VALUES:
*                              CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                              CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
*
* OUTPUTS:
*       etherTypePtr        - (pointer to) ethertype.
*       validPtr            - (pointer to) validity of the ethertype.
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
CMD_STATUS wrCpssPxIngressHashUdeEthertypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    GT_U16                               etherType;
    GT_BOOL                              valid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    hashPacketType  = (CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)inArgs[1];
    etherType       = 0x0000;
    valid           = GT_FALSE;

    /* call cpss api function */
    result = cpssPxIngressHashUdeEthertypeGet(
        devNum, hashPacketType, &etherType, &valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", etherType, valid);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashSeedSet
*
* DESCRIPTION:
*       Sets up CRC32 seed for hash calculation
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       crc32Seed           - seed for CRC32 packet hash calculation.
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
CMD_STATUS wrCpssPxIngressHashSeedSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    GT_U32                               crc32Seed;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    crc32Seed       = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressHashSeedSet(
        devNum, crc32Seed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashSeedGet
*
* DESCRIPTION:
*       Get CRC32 seed for hash calculation
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*
* OUTPUTS:
*       crc32SeedPtr        - (pointer to) seed for CRC32 packet hash
*                             calculation.
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
CMD_STATUS wrCpssPxIngressHashSeedGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    GT_U32                               crc32Seed;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    crc32Seed       = 0;

    /* call cpss api function */
    result = cpssPxIngressHashSeedGet(
        devNum, &crc32Seed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", crc32Seed);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashPortIndexModeSet
*
* DESCRIPTION:
*       Sets up LAG designated port table indexing mode for selected source
*       port.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       portNum             - port number.
*       indexMode           - designated port table indexing mode.
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
CMD_STATUS wrCpssPxIngressHashPortIndexModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_SW_DEV_NUM                             devNum;
    GT_PHYSICAL_PORT_NUM                      portNum;
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT  indexMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    indexMode = (CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxIngressHashPortIndexModeSet(
        devNum, portNum, indexMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashPortIndexModeGet
*
* DESCRIPTION:
*       Gets LAG designated port table indexing mode for selected source
*       port.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       portNum             - port number.
*
* OUTPUTS:
*       indexModePtr        - designated port table indexing mode.
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
CMD_STATUS wrCpssPxIngressHashPortIndexModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_SW_DEV_NUM                             devNum;
    GT_PHYSICAL_PORT_NUM                      portNum;
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT  indexMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    indexMode = CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxIngressHashPortIndexModeGet(
        devNum, portNum, &indexMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", indexMode);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashDesignatedPortsEntrySet
*
* DESCRIPTION:
*       Set entry in LAG designated ports table.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       entryIndex          - entry index (APPLICABLE RANGES: 0..127).
*       portsBmp            - member port bitmap.
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
CMD_STATUS wrCpssPxIngressHashDesignatedPortsEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_SW_DEV_NUM                 devNum;
    GT_U32                        entryIndex;
    CPSS_PX_PORTS_BMP             portsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    entryIndex = (GT_U32)inFields[0];
    portsBmp   = (CPSS_PX_PORTS_BMP)inFields[1];

    /* call cpss api function */
    result = cpssPxIngressHashDesignatedPortsEntrySet(
        devNum, entryIndex, portsBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static GT_U32 gCurrentIndex;
/*******************************************************************************
* cpssPxIngressHashDesignatedPortsEntryGet
*
* DESCRIPTION:
*       Get entry in LAG designated ports table.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       entryIndex          - entry index (APPLICABLE RANGES: 0..127).
*
* OUTPUTS:
*       portsBmpPtr         - (pointer to) member port bitmap.
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
CMD_STATUS wrCpssPxIngressHashDesignatedPortsEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    CPSS_PX_PORTS_BMP               portsBmp;
    GT_U32                          startIndex;
    GT_U32                          endIndex;
    GT_U32                          totalEntries;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_SW_DEV_NUM)inArgs[0];
    startIndex          = (GT_U32)inArgs[1];
    totalEntries        = (GT_U32)inArgs[2];

    endIndex = startIndex + totalEntries;

    if (gCurrentIndex >= endIndex)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api set function */
    result = cpssPxIngressHashDesignatedPortsEntryGet(devNum, gCurrentIndex, &portsBmp);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput("%d%d", gCurrentIndex, portsBmp);
    fieldOutputSetAppendMode();

    /* Increnment current index before next read */
    gCurrentIndex++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressHashDesignatedPortsEntryGet */
CMD_STATUS wrCpssPxIngressHashDesignatedPortsEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32 maxEntries;
    GT_U32 totalEntries;
    GT_SW_DEV_NUM  devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum        = (GT_SW_DEV_NUM)inArgs[0];
    if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        maxEntries = 256;
    }
    else
    {
        maxEntries = 128;
    }
    gCurrentIndex = (GT_U32)inArgs[1];
    totalEntries  = (GT_U32)inArgs[2];

    if ((gCurrentIndex + totalEntries) > maxEntries)
    {
        /* Index is out of range */
        galtisOutput(outArgs, GT_OUT_OF_RANGE, "%d", -1);
        return CMD_AGENT_ERROR;
    }

    return wrCpssPxIngressHashDesignatedPortsEntryGet(inArgs, inFields, numFields, outArgs); 
}

/* wrapper for cpssPxIngressHashDesignatedPortsEntryGet */
CMD_STATUS wrCpssPxIngressHashDesignatedPortsEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressHashDesignatedPortsEntryGet(inArgs, inFields, numFields, outArgs);
}

/* table cpssPxIngressHashPacketTypeEntry */

static GT_U32                               ut_cpssPxIngressHashPacketTypeEntry_bytePairIndex = 0;
static CPSS_PX_INGRESS_HASH_UDBP_STC        ut_cpssPxIngressHashPacketTypeEntry_udbpArr[
  CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];

CMD_STATUS wrCpssPxIngressHashPacketTypeEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    GT_BOOL                              sourcePortEnable;
    GT_BOOL                              sourcePortEnableGet;
    GT_U32                               bytePairIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           =  (GT_SW_DEV_NUM)inArgs[0];
    hashPacketType   =  (CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)inArgs[1];
    sourcePortEnable =  (GT_BOOL)inArgs[2];

    /* get and check bytePairIndex from fields */
    bytePairIndex = (GT_U32)inFields[0];
    if (bytePairIndex >= CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS)
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "bytePairIndex out of range");
        return CMD_OK;
    }
    /* read before update */
    result = cpssPxIngressHashPacketTypeEntryGet(
        devNum, hashPacketType,
        ut_cpssPxIngressHashPacketTypeEntry_udbpArr,
        &sourcePortEnableGet);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    ut_cpssPxIngressHashPacketTypeEntry_udbpArr[bytePairIndex].anchor =
        (CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT)inFields[1];
    ut_cpssPxIngressHashPacketTypeEntry_udbpArr[bytePairIndex].offset =
        (GT_U32)inFields[2];
    ut_cpssPxIngressHashPacketTypeEntry_udbpArr[bytePairIndex].nibbleMaskArr[0] =
        (GT_BOOL)inFields[3];
    ut_cpssPxIngressHashPacketTypeEntry_udbpArr[bytePairIndex].nibbleMaskArr[1] =
        (GT_BOOL)inFields[4];
    ut_cpssPxIngressHashPacketTypeEntry_udbpArr[bytePairIndex].nibbleMaskArr[2] =
        (GT_BOOL)inFields[5];
    ut_cpssPxIngressHashPacketTypeEntry_udbpArr[bytePairIndex].nibbleMaskArr[3] =
        (GT_BOOL)inFields[6];
    /* update */
    result = cpssPxIngressHashPacketTypeEntrySet(
        devNum, hashPacketType,
        ut_cpssPxIngressHashPacketTypeEntry_udbpArr,
        sourcePortEnable);
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

CMD_STATUS wrCpssPxIngressHashPacketTypeEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CPSS_PX_INGRESS_HASH_UDBP_STC *entryPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (ut_cpssPxIngressHashPacketTypeEntry_bytePairIndex >= CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    entryPtr = &(ut_cpssPxIngressHashPacketTypeEntry_udbpArr[
                  ut_cpssPxIngressHashPacketTypeEntry_bytePairIndex]);

    fieldOutput(
        "%d%d%d%d%d%d%d",
        ut_cpssPxIngressHashPacketTypeEntry_bytePairIndex,
        entryPtr->anchor, entryPtr->offset,
        entryPtr->nibbleMaskArr[0], entryPtr->nibbleMaskArr[1],
        entryPtr->nibbleMaskArr[2], entryPtr->nibbleMaskArr[3]);

    galtisOutput(outArgs, GT_OK, "%f");

    ut_cpssPxIngressHashPacketTypeEntry_bytePairIndex++;
    return CMD_OK;
}

CMD_STATUS wrCpssPxIngressHashPacketTypeEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    GT_BOOL                              sourcePortEnableGet;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           =  (GT_SW_DEV_NUM)inArgs[0];
    hashPacketType   =  (CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)inArgs[1];

    /* read all table */
    result = cpssPxIngressHashPacketTypeEntryGet(
        devNum, hashPacketType,
        ut_cpssPxIngressHashPacketTypeEntry_udbpArr,
        &sourcePortEnableGet);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    ut_cpssPxIngressHashPacketTypeEntry_bytePairIndex = 0;

    return wrCpssPxIngressHashPacketTypeEntryGetNext(
        inArgs, inFields, numFields, outArgs);
}

CMD_STATUS wrCpssPxIngressHashPacketTypeEntrySrcPortEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    GT_BOOL                              sourcePortEnableGet;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              =  (GT_SW_DEV_NUM)inArgs[0];
    hashPacketType      =  (CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)inArgs[1];
    sourcePortEnableGet =  GT_FALSE;

    /* read all table */
    result = cpssPxIngressHashPacketTypeEntryGet(
        devNum, hashPacketType,
        ut_cpssPxIngressHashPacketTypeEntry_udbpArr,
        &sourcePortEnableGet);

    galtisOutput(outArgs, result, "%d", sourcePortEnableGet);
    return CMD_OK;
}

/**/
CMD_STATUS wrCpssPxIngressHashPacketTypeHashModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode;
    GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           =  (GT_SW_DEV_NUM)inArgs[0];
    hashPacketType   =  (CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)inArgs[1];

    /* get values from fields */
    hashMode         = (CPSS_PX_INGRESS_HASH_MODE_ENT)inFields[0];
    bitOffsetsArr[0] = (GT_U32)inFields[1];
    bitOffsetsArr[1] = (GT_U32)inFields[2];
    bitOffsetsArr[2] = (GT_U32)inFields[3];
    bitOffsetsArr[3] = (GT_U32)inFields[4];
    bitOffsetsArr[4] = (GT_U32)inFields[5];
    bitOffsetsArr[5] = (GT_U32)inFields[6];
    bitOffsetsArr[6] = (GT_U32)inFields[7];
    if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        bitOffsetsArr[7] = (GT_U32)inFields[8];
    }
    /* call API */
    result = cpssPxIngressHashPacketTypeHashModeSet(
        devNum, hashPacketType, hashMode, bitOffsetsArr);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

CMD_STATUS wrCpssPxIngressHashPacketTypeHashModeGetNext
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

CMD_STATUS wrCpssPxIngressHashPacketTypeHashModeGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode = 0;
    GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum           =  (GT_SW_DEV_NUM)inArgs[0];
    hashPacketType   =  (CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT)inArgs[1];

    /* call API */
    result = cpssPxIngressHashPacketTypeHashModeGet(
        devNum, hashPacketType, &hashMode, bitOffsetsArr);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    fieldOutput(
        "%d%d%d%d%d%d%d%d%d", hashMode, bitOffsetsArr[0], bitOffsetsArr[1], bitOffsetsArr[2],
        bitOffsetsArr[3], bitOffsetsArr[4], bitOffsetsArr[5], bitOffsetsArr[6], bitOffsetsArr[7]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashLagTableModeSet
*
* DESCRIPTION:
*       Sets global LAG Designated Port Table mode
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       lagTableMode        - LAG table mode.
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
*       Applicable starting from revision A1
* 
*******************************************************************************/
CMD_STATUS wrCpssPxIngressHashLagTableModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT     lagTableMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    lagTableMode    = (CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxIngressHashLagTableModeSet(devNum, lagTableMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashLagTableModeGet
*
* DESCRIPTION:
*       Gets global LAG Designated Port Table mode
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*
* OUTPUTS:
*       lagTableModePtr     - (pointer to) LAG table mode.
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
*       Applicable starting from revision A1
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressHashLagTableModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_SW_DEV_NUM                           devNum;
    CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT lagTableMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    lagTableMode    = 0;

    /* call cpss api function */
    result = cpssPxIngressHashLagTableModeGet(devNum, &lagTableMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", lagTableMode);

    return CMD_OK;
}

/*******************************************************************************
* cpssPxIngressHashPacketTypeLagTableNumberSet
*
* DESCRIPTION:
*       Sets LAG table number for specific packet type
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       packetType          - index of the packet type key in the table.
*                             (APPLICABLE RANGES: 0..31)
*       lagTableNumber      - the LAG table number.
*                             (APPLICABLE RANGES: 0..1)
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
*       Applicable starting from revision A1
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressHashPacketTypeLagTableNumberSet
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
    GT_U32                          lagTableNumber;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                                  = (GT_SW_DEV_NUM)inArgs[0];
    /* map input fields to locals */
    packetType                              = (CPSS_PX_PACKET_TYPE)inFields[0];
    lagTableNumber                          = (GT_U32)inFields[1];

    /* call cpss api function */
    result = cpssPxIngressHashPacketTypeLagTableNumberSet(devNum, packetType, lagTableNumber);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static CPSS_PX_PACKET_TYPE     gPacketType;
/*******************************************************************************
* cpssPxIngressHashPacketTypeLagTableNumberGet
*
* DESCRIPTION:
*       Gets LAG table number for specific packet type
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       devNum              - device number.
*       packetType          - index of the packet type key in the table.
*                             (APPLICABLE RANGES: 0..31)
*
* OUTPUTS:
*       lagTableNumberPtr   - (pointer to) the LAG table number.
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
*       Applicable starting from revision A1
*
*******************************************************************************/
CMD_STATUS wrCpssPxIngressHashPacketTypeLagTableNumberGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_SW_DEV_NUM                   devNum;
    GT_U32                          lagTableNumber;

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

    /* call cpss api get function */
    result = cpssPxIngressHashPacketTypeLagTableNumberGet(devNum, gPacketType, &lagTableNumber);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* Tab-number, packet type before fields */
    fieldOutput(
        "%d%d", gPacketType, lagTableNumber);

    /* Increment packet type before next read */
    gPacketType++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* wrapper for cpssPxIngressHashPacketTypeLagTableNumberGet */
CMD_STATUS wrCpssPxIngressHashPacketTypeLagTableNumberGetFirst
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
    return wrCpssPxIngressHashPacketTypeLagTableNumberGet(inArgs, inFields, numFields, outArgs);
}

/* wrapper for cpssPxIngressHashPacketTypeLagTableNumberGet */
CMD_STATUS wrCpssPxIngressHashPacketTypeLagTableNumberGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxIngressHashPacketTypeLagTableNumberGet(inArgs, inFields, numFields, outArgs);
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName,                               funcReference                                   funcArgs funcFields */
    { "cpssPxIngressHashUdeEthertypeSet",         &wrCpssPxIngressHashUdeEthertypeSet,            4, 0},
    { "cpssPxIngressHashUdeEthertypeGet",         &wrCpssPxIngressHashUdeEthertypeGet,            2, 0},
    { "cpssPxIngressHashSeedSet",                 &wrCpssPxIngressHashSeedSet,                    2, 0},
    { "cpssPxIngressHashSeedGet",                 &wrCpssPxIngressHashSeedGet,                    1, 0},
    { "cpssPxIngressHashPortIndexModeSet",        &wrCpssPxIngressHashPortIndexModeSet,           3, 0},
    { "cpssPxIngressHashPortIndexModeGet",        &wrCpssPxIngressHashPortIndexModeGet,           2, 0},
    { "cpssPxIngressHashDesignatedPortEntrySet", 
        &wrCpssPxIngressHashDesignatedPortsEntrySet,        1, 2},
    { "cpssPxIngressHashDesignatedPortEntryGetFirst", 
        &wrCpssPxIngressHashDesignatedPortsEntryGetFirst,   3, 0},
    { "cpssPxIngressHashDesignatedPortEntryGetNext", 
        &wrCpssPxIngressHashDesignatedPortsEntryGetNext,    3, 0},
    { "cpssPxIngressHashPacketTypeEntrySet",      &wrCpssPxIngressHashPacketTypeEntrySet,         3, 7},
    { "cpssPxIngressHashPacketTypeEntryGetNext",  &wrCpssPxIngressHashPacketTypeEntryGetNext,     3, 0},
    { "cpssPxIngressHashPacketTypeEntryGetFirst", &wrCpssPxIngressHashPacketTypeEntryGetFirst,    3, 0},
    { "cpssPxIngressHashPacketTypeEntrySrcPortEnableGet",
        &wrCpssPxIngressHashPacketTypeEntrySrcPortEnableGet,    2, 0},
    { "cpssPxIngressHashPacketTypeHashModeSet",      &wrCpssPxIngressHashPacketTypeHashModeSet,      2, 8},
    { "cpssPxIngressHashPacketTypeHashModeGetNext",  &wrCpssPxIngressHashPacketTypeHashModeGetNext,  2, 0},
    { "cpssPxIngressHashPacketTypeHashModeGetFirst", &wrCpssPxIngressHashPacketTypeHashModeGetFirst, 2, 0},
    { "cpssPxIngressHashLagTableModeSet",            &wrCpssPxIngressHashLagTableModeSet,            2, 0},
    { "cpssPxIngressHashLagTableModeGet",            &wrCpssPxIngressHashLagTableModeGet,            1, 0},
    { "cpssPxIngressHashPktTypeLagTableNumberSet",       &wrCpssPxIngressHashPacketTypeLagTableNumberSet,        1, 2},
    { "cpssPxIngressHashPktTypeLagTableNumberGetFirst",  &wrCpssPxIngressHashPacketTypeLagTableNumberGetFirst,   1, 0},
    { "cpssPxIngressHashPktTypeLagTableNumberGetNext",   &wrCpssPxIngressHashPacketTypeLagTableNumberGetNext,    1, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/*******************************************************************************
* cmdLibInitCpssPxIngressHash
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
GT_STATUS cmdLibInitCpssPxIngressHash
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


