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
* @file prvTgfPclUdb10KeyCustomer.c
*
* @brief PCL UDB10 Only key customer testing
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfOamGen.h>
#include <pcl/prvTgfPclUdb10KeyCustomer.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS         2

/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             8

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0x55

/* original VLAN Id */
#define PRV_TGF_TAG0_VLAN_ID_CNS             0x10

/* original VLAN Id */
#define PRV_TGF_TAG1_VLAN_ID_CNS             0x11

/* original UP0 */
#define PRV_TGF_TAG0_UP_ID_CNS               0x5

/* original UP1 */
#define PRV_TGF_TAG1_UP_ID_CNS               0x3

/* original DSCP */
#define PRV_TGF_DSCP_ID_CNS                  0x12

/* lookup1 VLAN Id */
#define PRV_TGF_PCL_NEW_VLAN_ID_CNS          0x31

/* Action Flow Id  */
#define PRV_TGF_PCL_ACTION_FLOW_ID_CNS       0x26

/* eVid */
#define PRV_TGF_EVID_CNS                    ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(0x1234)

/* Source ePort */
#define PRV_TGF_SRC_EPORT_CNS               ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1432)

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* PCL id for restore procedure */
static GT_U32 savePclId;
/********************************************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                       /* etherType */
    PRV_TGF_TAG0_UP_ID_CNS, 0, PRV_TGF_TAG0_VLAN_ID_CNS     /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                        /* etherType */
    PRV_TGF_TAG1_UP_ID_CNS, 0, PRV_TGF_TAG1_VLAN_ID_CNS     /* pri, cfi, VlanId */
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OtherHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    (PRV_TGF_DSCP_ID_CNS << 2),                                      /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)), /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    4,                                                               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    {22, 22, 22, 22},                                                /* srcAddr */
    { 1,  1,  1,  3}                                                 /* dstAddr */
};

/* Double Tagged IPV4 Other Packet Parts Array */
static TGF_PACKET_PART_STC prvTgfPacket2tagsIpv4OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV4 Other Packet Info */
static TGF_PACKET_STC prvTgfPacket2tagsIpv4OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                 /* totalLen */
    sizeof(prvTgfPacket2tagsIpv4OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket2tagsIpv4OthPartsArray                                        /* partsArray */
};

/**
* @internal prvTgfPclUdb10KeyCustomerPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerPortVlanFdbSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_TAG0_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as PCL assigned VID with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_NEW_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as TAG in with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_EVID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_TAG0_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_EVID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdb10KeyCustomerPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerPortVlanFdbReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TAG0_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PCL_NEW_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EVID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
}

/**
* @internal prvTgfPclUdb10KeyCustomerVidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
static GT_VOID prvTgfPclUdb10KeyCustomerVidTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal prvTgfPclUdb10KeyCustomerTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
* @param[in] checkMsb                 - to check High bits of VID and UP
*                                       None
*/
static GT_VOID prvTgfPclUdb10KeyCustomerTrafficEgressVidCheck
(
    IN GT_U16  egressVid,
    IN GT_BOOL checkMsb
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    if (checkMsb == GT_FALSE)
    {
        vfdArray[0].offset = 15; /* 6 + 6 + 3 */
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = (GT_U8)(egressVid & 0xFF);
    }
    else
    {
        vfdArray[0].offset = 14; /* 6 + 6 + 2 */
        vfdArray[0].cycleCount = 2;
        vfdArray[0].patternPtr[0] = (GT_U8)((egressVid >> 8) & 0xFF);
        vfdArray[0].patternPtr[1] = (GT_U8)(egressVid & 0xFF);
    }

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfPclUdb10KeyCustomerPclLegacyCfgSet function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*                                       None
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerPclLegacyCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormatArr[3];
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           portIndex;

    portIndex =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_RECEIVE_PORT_IDX_CNS
            : PRV_TGF_SEND_PORT_IDX_CNS;

    ruleFormatArr[0] =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    ruleFormatArr[1] =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    ruleFormatArr[2] = ruleFormatArr[1];

    ruleFormatArr[pktType] = ruleFormat;


    /* AUTODOC: Init and configure all needed PCL configuration per port Lookup0 */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[portIndex],
        direction, lookupNum,
        PRV_TGF_PCL_ID_CNS,
        ruleFormatArr[0] /*nonIpKey*/,
        ruleFormatArr[1] /*ipv4Key*/,
        ruleFormatArr[2] /*ipv6Key*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExt1");

    /* AUTODOC: action - TAG0 vlan modification */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.egressPolicy = GT_TRUE;
        action.vlan.egressVlanId0Cmd =
            PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }
    else
    {
        action.egressPolicy = GT_FALSE;
        action.vlan.vlanCmd =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }

    /* AUTODOC: PCL rule: Write Rule */
    rc = prvTgfPclRuleSet(
        ruleFormat,
        PRV_TGF_PCL_RULE_IDX_CNS,
        maskPtr, patternPtr, &action);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfPclUdb10KeyCustomerPclLegacyCfgReset function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
*                                       None
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerPclLegacyCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize
)
{
    GT_STATUS rc,rc1 = GT_OK;

    /* AUTODOC: restore PCL configuration table entry */
    prvTgfPclPortsRestore(direction, lookupNum);

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
        ruleSize, PRV_TGF_PCL_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectSet function
* @endinternal
*
* @brief   Set UDB Only PCL Key UDB Range selection
*         Replaced UDB positions ommited.
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
* @param[in] udbReplaceBitmap         - bitmap of replace UDBs (12 bits ingress, 2 bits egress)
* @param[in] udbAmount                - udb Amount
* @param[in] udbIndexBase             - udb Index Base
*                                      udbOffsetIncrement  - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           udbReplaceBitmap,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexBase,
    IN GT_32                            udbIndexIncrement
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              udbInKeyMapped;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    udbInKeyMapped = 0;

    for (i = 0; (i < udbAmount); i++)
    {
        if (udbReplaceBitmap & (1 << i))
        {
            if (i == 0) udbSelect.egrUdb01Replace      = GT_TRUE;
            if (i == 1) udbSelect.egrUdbBit15Replace   = GT_TRUE;
            if (i < 12) udbSelect.ingrUdbReplaceArr[i] = GT_TRUE;
        }
        else
        {
            udbSelect.udbSelectArr[i] =
                udbIndexBase + (udbIndexIncrement * udbInKeyMapped);
            udbInKeyMapped ++;
        }
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedBytesSelectSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectReset function
* @endinternal
*
* @brief   Reset UDB Only PCL Key UDB selection
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
*                                       None
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectReset
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat
)
{
    GT_STATUS                           rc;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedBytesSelectSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdb10KeyCustomerPclUdbRangeCfgSet function
* @endinternal
*
* @brief   Set PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] offsetType               - offset Type
* @param[in] udbIndexBase             - udb Index Base
* @param[in] udbAmount                - udb Amount
* @param[in] udbOffsetBase            - udb Offset Base
* @param[in] udbOffsetIncrement       - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerPclUdbRangeCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbOffsetBase,
    IN GT_32                            udbOffsetIncrement
)
{
    GT_STATUS                            rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U8                                offset;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;
        offset = (GT_U8)(udbOffsetBase + (i * udbOffsetIncrement));

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction,udbIndex, offsetType, offset);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedByteSet");
    }

    return GT_OK;
}


/**
* @internal prvTgfPclUdb10KeyCustomerPclUdbRangeCfgReset function
* @endinternal
*
* @brief   Reset PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
*                                       None
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerPclUdbRangeCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount
)
{
    GT_STATUS                            rc, rc1 = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction, udbIndex,
            PRV_TGF_PCL_OFFSET_INVALID_E, 0/*offset*/);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclUserDefinedByteSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdb10KeyCustomerTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerTtiConfigSet
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


/* eVid PRV_TGF_EVID_CNS*/

    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, assign source ePort */
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                        = PRV_TGF_EVID_CNS;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.sourceEPortAssignmentEnable       = GT_TRUE;
    ttiAction.sourceEPort                       = PRV_TGF_SRC_EPORT_CNS;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    switch (tunnelPacketType)
    {
        case PRV_TGF_TTI_KEY_MPLS_E:
            ttiMask.mpls.common.pclId    = 0x3FF;
            ttiPattern.mpls.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_IPV4_E:
            ttiMask.ipv4.common.pclId    = 0x3FF;
            ttiPattern.ipv4.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_ETH_E:
            ttiMask.eth.common.pclId    = 0x3FF;
            ttiPattern.eth.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_MIM_E:
            ttiMask.mim.common.pclId    = 0x3FF;
            ttiPattern.mim.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        default: break;
    }

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, tunnelPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, tunnelPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS , GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEPortIngressPolicyEnable");

    return GT_OK;
}

/**
* @internal prvTgfPclUdb10KeyCustomerTtiConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerTtiConfigReset
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType
)
{
    GT_STATUS                rc, rc1 = GT_OK;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_FALSE);

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS , GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEPortIngressPolicyEnable");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfPclUdb10KeyCustomerTest function
* @endinternal
*
* @brief   Test on Ingress UDB10 Only Key with customer scenario
*
* @note Fields:
*       pclId - replacedFld.pclId  - muxed with UDB0-1
*       Evlan - replacedFld.vid   - muxed with UDB2-3
*       Eport - replacedFld.srcPort - muxed with UDB4-5
*       VID1  - replacedFld.vid1  - muxed with UDB6-7
*       UP1  - replacedFld.up1   - muxed with UDB7
*       UP0  - UDB8 L2 + 14, mask 0xE0, patten - (up0 << 5)
*       DSCP  - UDB9 L3Minus2 + 3, mask 0xFC, patten - (dscp << 2)
*       UDB mapping 30-1 => 8-9
*
*/
GT_VOID prvTgfPclUdb10KeyCustomerTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdb10KeyCustomerPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: TTI configuration */
    rc = prvTgfPclUdb10KeyCustomerTtiConfigSet(PRV_TGF_TTI_KEY_IPV4_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: PCL configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    mask.ruleIngrUdbOnly.replacedFld.pclId    = 0x3FF;
    pattern.ruleIngrUdbOnly.replacedFld.pclId = PRV_TGF_PCL_ID_CNS;

    /* eVID */
    mask.ruleIngrUdbOnly.replacedFld.vid    = 0x1FFF;
    pattern.ruleIngrUdbOnly.replacedFld.vid = PRV_TGF_EVID_CNS;

    /* Source ePort*/
    mask.ruleIngrUdbOnly.replacedFld.srcPort    = 0x1FFF;
    pattern.ruleIngrUdbOnly.replacedFld.srcPort = PRV_TGF_SRC_EPORT_CNS;

    mask.ruleIngrUdbOnly.replacedFld.vid1    = 0xFFF;
    pattern.ruleIngrUdbOnly.replacedFld.vid1 = PRV_TGF_TAG1_VLAN_ID_CNS;

    mask.ruleIngrUdbOnly.replacedFld.up1    = 7;
    pattern.ruleIngrUdbOnly.replacedFld.up1 = PRV_TGF_TAG1_UP_ID_CNS;

    /* UP0 in UDB8 */
    mask.ruleIngrUdbOnly.udb[8]    = 0xE0;
    pattern.ruleIngrUdbOnly.udb[8] = (PRV_TGF_TAG0_UP_ID_CNS << 5);

    /* DSCP in UDB9 */
    mask.ruleIngrUdbOnly.udb[9]    = 0xFC;
    pattern.ruleIngrUdbOnly.udb[9] = (PRV_TGF_DSCP_ID_CNS << 2);

    rc = prvTgfPclUdb10KeyCustomerPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectSet(
        CPSS_PCL_LOOKUP_0_0_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
        0xFF /*udbReplaceBitmap*/,
        10 /*udbAmount*/,
        30 /*udbIndexBase*/,
        1  /*udbIndexIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: UP0  - UDB30 L2 + 14,  mask 0xE0, patten - (up0 << 5)*/
    rc = prvTgfPclUdb10KeyCustomerPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_L2_E,
        30 /*udbIndexBase*/,
        1  /*udbAmount*/,
        14 /*udbOffsetBase*/,
        1  /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: DSCP - UDB31 L3Minus2 + 3,  mask 0xFC, patten - (dscp << 2)*/
    rc = prvTgfPclUdb10KeyCustomerPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
        31 /*udbIndexBase*/,
        1 /*udbAmount*/,
        3 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Send Double Tagged IPV4 Other Packet */
    prvTgfPclUdb10KeyCustomerVidTrafficGenerate(&prvTgfPacket2tagsIpv4OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdb10KeyCustomerTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_FALSE/*check 8 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdb10KeyCustomerPortVlanFdbReset();

    prvTgfPclUdb10KeyCustomerTtiConfigReset(PRV_TGF_TTI_KEY_IPV4_E);

    prvTgfPclUdb10KeyCustomerPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdb10KeyCustomerPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        30 /*udbIndexBase*/,
        2  /*udbAmount*/);

    prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectReset(
        CPSS_PCL_LOOKUP_0_0_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E);
}



