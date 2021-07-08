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
* @file prvTgfPclBc2.c
*
* @brief SIP5 specific PCL features testing
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfOamGen.h>
#include <pcl/prvTgfPclBc2.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS    2

/* lookup0 rule index */
#define PRV_TGF_LOOKUP0_RULE_IDX_CNS    3

#define PRV_TGF_EGRESS_LOOKUP0_RULE_IDX_CNS    prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(3)

/* lookup1 rule index */
#define PRV_TGF_LOOKUP1_RULE_IDX_CNS    prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(1/*lookupId*/,5)

/* lookup0 PLL ID */
#define PRV_TGF_LOOKUP0_PCL_ID_CNS      1

/* lookup1 PLL ID */
#define PRV_TGF_LOOKUP1_PCL_ID_CNS      2

/* original VLAN Id */
#define PRV_TGF_ORIGINAL_VLAN_ID_CNS    3

/* lookup1 VLAN Id */
#define PRV_TGF_LOOKUP1_VLAN_ID_CNS     7

/* ARP enry index */
#define PRV_TGF_ARP_ENTRY_IDX_CNS       9

/* MPLS Channel Type0  */
#define PRV_TGF_MPLS_CHANNEL_TYPE0_CNS  0x1234

/* MPLS Channel Type0 Profile */
#define PRV_TGF_MPLS_CHANNEL_TYPE0_PROFILE_CNS  4

/* MPLS Channel Type0  */
#define PRV_TGF_MPLS_CHANNEL_VERSION0_CNS  3

/* OAM opcode0  */
#define PRV_TGF_OAM_OPCODE_CNS  5

/* OAM opcode0  */
#define PRV_TGF_OAM_CMD_PROFILE  6

/* Action Flow Id  */
#define PRV_TGF_ACTION_FLOW_ID_CNS  2

/********************************************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketEthOth1L2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthOth1VlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_ORIGINAL_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOth1EthertypePart =
{
    0x3333
};

/* DATA of packet */
static GT_U8 prvTgfPacketEthOth1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketEthOth1PayloadPart = {
    sizeof(prvTgfPacketEthOth1PayloadDataArr),            /* dataLength */
    prvTgfPacketEthOth1PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketEthOth1PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketEthOth1L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketEthOth1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketEthOth1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketEthOth1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketEthOth1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketEthOth1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthOth1PartArray                                        /* partsArray */
};

/* L2 part of packet2 */
static TGF_PACKET_L2_STC prvTgfPacketEthOth2L2Part =
{
    {0x00, 0xA9, 0x87, 0x65, 0x43, 0x21},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

/* PARTS of packet2 */
static TGF_PACKET_PART_STC prvTgfPacketEthOth2PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketEthOth2L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketEthOth1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketEthOth1PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacketEthOth2Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketEthOth1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketEthOth2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthOth2PartArray                                        /* partsArray */
};

/* MPLS Packet */

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketMpls1L2Part =
{
    {0x00, 0x21, 0x43, 0x65, 0x87, 0xA9},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketMpls1VlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_ORIGINAL_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMpls1EthertypePart =
{
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/* MPLS LSP addressing */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls1Lsp =
{
    1111 /* Addressing Label */,
    0/*exp*/,
    0/*stack - not last*/,
    100/*timeToLive*/
};

/* MPLS General Associated Label */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls1Gal =
{
    13 /* GAL Generic Associated Label */,
    0/*exp*/,
    1/*stack - last*/,
    101/*timeToLive*/
};

/* MPLS General Associated Channel Header */
static GT_U8 prvTgfPacketMpls1GAchArr[] =
{
    ((1 << 4) | (PRV_TGF_MPLS_CHANNEL_VERSION0_CNS & 0xF)),
    0, /*reserved*/
    ((PRV_TGF_MPLS_CHANNEL_TYPE0_CNS >> 8) & 0xFF),
    (PRV_TGF_MPLS_CHANNEL_TYPE0_CNS & 0xFF)
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketMpls1GAchPart = {
    sizeof(prvTgfPacketMpls1GAchArr),            /* dataLength */
    prvTgfPacketMpls1GAchArr                     /* dataPtr */
};

/* DATA of packet */
static GT_U8 prvTgfPacketMpls1PayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketMpls1PayloadPart = {
    sizeof(prvTgfPacketMpls1PayloadDataArr),            /* dataLength */
    prvTgfPacketMpls1PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketMpls1PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketMpls1L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketMpls1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMpls1EthertypePart},
    {TGF_PACKET_PART_MPLS_E, &prvTgfPacketMpls1Lsp},
    {TGF_PACKET_PART_MPLS_E, &prvTgfPacketMpls1Gal},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketMpls1GAchPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketMpls1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketMpls1Info = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                    /* totalLen   */
    (sizeof(prvTgfPacketMpls1PartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    prvTgfPacketMpls1PartArray                                          /* partsArray */
};


/**
* @internal prvTgfPclBc2VidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
GT_VOID prvTgfPclBc2VidTrafficGenerate
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
* @internal prvTgfPclBc2TrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] portIndex                - port index
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
* @param[in] checkMsb                 - to check High bits of VID and UP
*                                       None
*/
GT_VOID prvTgfPclBc2TrafficEgressVidCheck
(
    IN GT_U32  portIndex,
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
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[portIndex];
    portInterface.devPort.portNum = prvTgfPortsArray[portIndex];

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
* @internal prvTgfPclBc2FdbEntryCheck function
* @endinternal
*
* @brief   Checks fdb entry
*
* @param[in] macAddrPtr               - MAC address to search
* @param[in] vlanId                   - VID for search
*                                       None
*/
static GT_VOID prvTgfPclBc2FdbEntryCheck
(
    IN GT_ETHERADDR     *macAddrPtr,
    IN GT_U16           vlanId
)
{
    GT_STATUS rc;
    PRV_TGF_MAC_ENTRY_KEY_STC    searchKey;
    GT_U32                       index;
    GT_BOOL                      valid;
    GT_BOOL                      skip;
    GT_BOOL                      age;
    GT_HW_DEV_NUM                hwDevNum;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;

    /* AUTODOC: Check Presence of FDB entry with given MAC and VID */

    cpssOsMemSet(&searchKey, 0, sizeof(searchKey));
    searchKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    cpssOsMemCpy(
        &(searchKey.key.macVlan.macAddr), macAddrPtr, sizeof(GT_ETHERADDR));
    searchKey.key.macVlan.vlanId = vlanId;

    /* AUTODOC: Search index of FDB entry by MAC and VID key */
    rc = prvTgfBrgFdbMacEntryIndexFind(
        &searchKey, &index);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind");

    /* AUTODOC: read found MAC entry */
    rc = prvTgfBrgFdbMacEntryRead(
        index, &valid, &skip, &age, &hwDevNum, &macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntryRead");

    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_TRUE, valid, "FDB record invalid");

    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_FALSE, skip, "FDB record skipped");
}


/**
* @internal prvTgfPclBc2Mac2meSetBaseConfiguration function
* @endinternal
*
* @brief   Setup base configuration for Mac2me Test
*/
GT_VOID prvTgfPclBc2Mac2meSetBaseConfiguration
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN 3 with all TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_ORIGINAL_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN 7 with all TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_LOOKUP1_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");
}

/**
* @internal prvTgfPclBc2Mac2meSetMac2MeWithPclRule function
* @endinternal
*
* @brief   Setup PCL rule to set mac2me for given mac address
*/
GT_VOID prvTgfPclBc2Mac2meSetMac2MeWithPclRule
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    static GT_U8                     macMask[6] =
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting mac2me with Pcl Configuration =======\n");

    /* AUTODOC: Init and configure all needed PCL configuration per port Lookup0 */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_LOOKUP0_PCL_ID_CNS,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit lookup0: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Lookup0 action SET MAC2ME */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd     = CPSS_PACKET_CMD_FORWARD_E;
    action.setMacToMe = GT_TRUE;

    /* AUTODOC: Lookup0 rule: PCL_ID and destination MAC */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_LOOKUP0_PCL_ID_CNS;
    cpssOsMemCpy(mask.ruleStdNotIp.macDa.arEther, macMask, 6);
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketEthOth1L2Part.daMac, 6);

    /* AUTODOC: Lookup0 rule: Write Rule */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_LOOKUP0_RULE_IDX_CNS,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet Lookup0");
}

/**
* @internal prvTgfPclBc2Mac2meCheckMac2MeWithPclRule function
* @endinternal
*
* @brief   Setup PCL rule to modify packet VID if mac2me is set
*/
GT_VOID prvTgfPclBc2Mac2meCheckMac2MeWithPclRule
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Set Pcl rule to check mac2me value =======\n");

    /* AUTODOC: Init and configure all needed PCL configuration per port Lookup1 */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_1_E,
        PRV_TGF_LOOKUP1_PCL_ID_CNS,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit lookup1: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Lookup1 action modify VID */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd          = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_LOOKUP1_VLAN_ID_CNS;
    action.vlan.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* AUTODOC: Lookup1 rule: PCL_ID and MAC2ME */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_LOOKUP1_PCL_ID_CNS;
    mask.ruleStdNotIp.common.macToMe    = 1;
    pattern.ruleStdNotIp.common.macToMe = 1;

    /* AUTODOC: Lookup0 rule: Write Rule */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_LOOKUP1_RULE_IDX_CNS,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet Lookup1");
}



/**
* @internal prvTgfPclBc2Mac2meTest function
* @endinternal
*
* @brief   Main test function
*/
GT_VOID prvTgfPclBc2Mac2meTest
(
    GT_VOID
)
{
    /* AUTODOC: set base configuration */
    prvTgfPclBc2Mac2meSetBaseConfiguration();

    /* AUTODOC: set mac2me using pcl rule */
    prvTgfPclBc2Mac2meSetMac2MeWithPclRule();

    /* AUTODOC: check mac2me using pcl rule */
    prvTgfPclBc2Mac2meCheckMac2MeWithPclRule();

    /* AUTODOC: send matched packet */
    prvTgfPclBc2VidTrafficGenerate(&prvTgfPacketEthOth1Info);

    /* AUTODOC: check matched packet */
    prvTgfPclBc2TrafficEgressVidCheck(
        PRV_TGF_RECEIVE_PORT_IDX_CNS,
        PRV_TGF_LOOKUP1_VLAN_ID_CNS,
        GT_FALSE /*checkMsb*/);

    /* AUTODOC: send unmatched packet */
    prvTgfPclBc2VidTrafficGenerate(&prvTgfPacketEthOth2Info);

    /* AUTODOC: check unmatched packet */
    prvTgfPclBc2TrafficEgressVidCheck(
        PRV_TGF_RECEIVE_PORT_IDX_CNS,
        PRV_TGF_ORIGINAL_VLAN_ID_CNS,
        GT_FALSE /*checkMsb*/);
}

/**
* @internal prvTgfPclBc2Mac2meTestRestore function
* @endinternal
*
* @brief   Restore after test configuration
*/
GT_VOID prvTgfPclBc2Mac2meTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disables configuration table and port configurations */
    prvTgfPclPortsRestore(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: disables configuration table and port configurations */
    prvTgfPclPortsRestore(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E);

    /* AUTODOC: invalidate PCL rule for Lookup0 */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_LOOKUP0_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: invalidate PCL rule for Lookup1 */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_LOOKUP1_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: invalidate VLAN Entry VID == 7 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_LOOKUP1_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Entry VID == 3 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ORIGINAL_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

/**
* @internal prvTgfPclBc2SetMacSaTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclBc2SetMacSaTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          portInterface;
    TGF_VFD_INFO_STC                 vfdArray[1];
    GT_U32                           numTriggersBmp;
    static GT_U8                     macMask[6] =
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    static GT_ETHERADDR              newFdbMacSa =
        {{0/*0x7C*/, 0 /*0x23*/, 0x45, 0x67, 0x89, 0x6A}};
    static GT_ETHERADDR              newEgressMacSa =
        {{0x66, 0x77, 0x88, 0x99, 0xAA, 0x44}};

    /* AUTODOC: Set MAC To Me Test */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Write VLAN Table Entry VID == 3, all ports TAG0 tagged */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_ORIGINAL_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Init and configure all needed PCL configuration per port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_LOOKUP0_PCL_ID_CNS,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit lookup0: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: PCL Action SET MAC_SA for learning in FDB */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd =
        PRV_TGF_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E;
    action.redirect.data.modifyMacSa.arpPtr =
        PRV_TGF_ARP_ENTRY_IDX_CNS;
    cpssOsMemCpy(
        action.redirect.data.modifyMacSa.macSa.arEther,
        newFdbMacSa.arEther, 6);

    /* AUTODOC: PCL rule: PCL_ID and destination MAC */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_LOOKUP0_PCL_ID_CNS;
    cpssOsMemCpy(mask.ruleStdNotIp.macDa.arEther, macMask, 6);
    cpssOsMemCpy(
        pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketEthOth1L2Part.daMac, 6);

    /* AUTODOC: Write PCL rule */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_LOOKUP0_RULE_IDX_CNS,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet Lookup0");

    /* AUTODOC: Write MAC for egress packet to ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(
        PRV_TGF_ARP_ENTRY_IDX_CNS, &newEgressMacSa);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfIpRouterArpAddrWrite");

    /* AUTODOC: send matched packet with capture */
    prvTgfPclBc2VidTrafficGenerate(&prvTgfPacketEthOth1Info);

    /* AUTODOC: check matched packet */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 6;
    vfdArray[0].cycleCount = 6;
    cpssOsMemCpy(vfdArray[0].patternPtr, newEgressMacSa.arEther, 6);

    /* AUTODOC: check captured packet */
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

    prvTgfPclBc2FdbEntryCheck(
        &newFdbMacSa, PRV_TGF_ORIGINAL_VLAN_ID_CNS);
}

/**
* @internal prvTgfPclBc2SetMacSaTestRestore function
* @endinternal
*
* @brief   Restore after test configuration
*/
GT_VOID prvTgfPclBc2SetMacSaTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: Restore Configuration  */

    /* AUTODOC: disables configuration table and port configurations */
    prvTgfPclPortsRestore(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: disable ingress policy on ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_LOOKUP0_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: invalidate VLAN Table Entry, VID == 3 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ORIGINAL_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfPclBc2MplsCh2OamOpcodeGenerateAncCheck function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
* @param[in] checkForwarded           - GT_TRUE - check forwarded, GT_FALSE - check dropped
*                                       None
*/
static GT_VOID prvTgfPclBc2MplsCh2OamOpcodeGenerateAncCheck
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr,
    IN GT_BOOL        checkForwarded
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          expectedTrigBmp;
    GT_U32                          numTriggersBmp;

    expectedTrigBmp = (checkForwarded == GT_FALSE) ? 0 : 1;

    /* AUTODOC: Transmit Packets With Capture*/

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

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

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount =0;
    vfdArray[0].patternPtr[0] = 0;

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
    UTF_VERIFY_EQUAL0_STRING_MAC(
        expectedTrigBmp, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfPclBc2MapMplsChannelToOamOpcodeTest function
* @endinternal
*
* @brief   Set test configuration and traffic checking
*/
GT_VOID prvTgfPclBc2MapMplsChannelToOamOpcodeTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_OAM_ENTRY_STC            oamEntry;

    /* AUTODOC: Map Mpls Channel To Oam Opcode Test */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: Write VLAN Table Entry VID == 3, all ports TAG0 tagged */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_ORIGINAL_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketMpls1L2Part.daMac,
        PRV_TGF_ORIGINAL_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Init and configure all needed PCL configuration per port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_LOOKUP0_PCL_ID_CNS,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit lookup0: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: PCL Action OAM enable, profile=0, channel Type To Opcode Map Enable */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable = GT_TRUE;
    action.oam.oamProfile = 0;
    action.channelTypeToOpcodeMapEnable = GT_TRUE;
    action.flowId = PRV_TGF_ACTION_FLOW_ID_CNS;

    /* AUTODOC: PCL rule: PCL_ID */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleEgrStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleEgrStdNotIp.common.pclId = PRV_TGF_LOOKUP0_PCL_ID_CNS;

    /* AUTODOC: Write PCL rule */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_EGRESS_LOOKUP0_RULE_IDX_CNS,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet Lookup0");

    /* AUTODOC: enable OAM processing */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableSet");

    /* AUTODOC: Reset OAM flow Id base */
    rc = prvTgfOamTableBaseFlowIdSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");

    cpssOsMemSet(&oamEntry, 0, sizeof(oamEntry));
    oamEntry.opcodeParsingEnable  = GT_TRUE;
    oamEntry.packetCommandProfile = PRV_TGF_OAM_CMD_PROFILE;

    /* AUTODOC: set OAM entry */
    rc = prvTgfOamEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_ACTION_FLOW_ID_CNS, &oamEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEntrySet");

    /* AUTODOC: channel type0 map to channel type profile */
    rc = prvTgfOamMplsCwChannelTypeProfileSet(
        PRV_TGF_MPLS_CHANNEL_TYPE0_PROFILE_CNS,
        PRV_TGF_MPLS_CHANNEL_TYPE0_CNS,
        GT_TRUE /*profileEnable*/);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamMplsCwChannelTypeProfileSet");

    /* AUTODOC: channel type0 profile map to opcode */
    rc = prvTgfPclOamChannelTypeProfileToOpcodeMappingSet(
        PRV_TGF_MPLS_CHANNEL_TYPE0_PROFILE_CNS,
        PRV_TGF_OAM_OPCODE_CNS,
        GT_FALSE /*RBitAssignmentEnable*/);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfPclOamChannelTypeProfileToOpcodeMappingSet");

    /* AUTODOC: set Opcode Command to Drop */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_OAM_OPCODE_CNS,
        PRV_TGF_OAM_CMD_PROFILE,
        CPSS_PACKET_CMD_DROP_HARD_E,
        0 /*cpuCodeOffset*/);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamOpcodeProfilePacketCommandEntrySet");


    /* AUTODOC: send packet counter checking */
    prvTgfPclBc2MplsCh2OamOpcodeGenerateAncCheck(&prvTgfPacketMpls1Info, GT_FALSE);

    /* AUTODOC: set Opcode Command to Drop */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_OAM_OPCODE_CNS,
        PRV_TGF_OAM_CMD_PROFILE,
        CPSS_PACKET_CMD_FORWARD_E,
        0 /*cpuCodeOffset*/);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamOpcodeProfilePacketCommandEntrySet");

    /* AUTODOC: send packet counter checking */
    prvTgfPclBc2MplsCh2OamOpcodeGenerateAncCheck(&prvTgfPacketMpls1Info, GT_TRUE);
}

/**
* @internal prvTgfPclBc2MapMplsChannelToOamOpcodeReset function
* @endinternal
*
* @brief   Set reset test configuration
*/
GT_VOID prvTgfPclBc2MapMplsChannelToOamOpcodeReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    PRV_TGF_OAM_ENTRY_STC            oamEntry;

    /* AUTODOC: Map Mpls Channel To Oam Opcode Restore configuration */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: invalidate VLAN Table Entry, VID == 3 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ORIGINAL_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_EGRESS_LOOKUP0_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: disables configuration table and port configurations */
    prvTgfPclPortsRestore(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: disables egress policy per devices */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: enable OAM processing */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfOamEnableSet");

    /* AUTODOC: Reset OAM flow Id base */
    rc = prvTgfOamTableBaseFlowIdSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, 0);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfOamTableBaseFlowIdSet");

    cpssOsMemSet(&oamEntry, 0, sizeof(oamEntry));

    /* AUTODOC: reset OAM entry */
    rc = prvTgfOamEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_ACTION_FLOW_ID_CNS, &oamEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfOamEntrySet");

    /* AUTODOC: set Opcode Command to Forward */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_OAM_OPCODE_CNS,
        PRV_TGF_OAM_CMD_PROFILE,
        CPSS_PACKET_CMD_FORWARD_E,
        0 /*cpuCodeOffset*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfOamOpcodeProfilePacketCommandEntrySet");

    /* AUTODOC: channel type0 map to channel type profile */
    rc = prvTgfOamMplsCwChannelTypeProfileSet(
        PRV_TGF_MPLS_CHANNEL_TYPE0_PROFILE_CNS,
        0 /*channelTypeId*/,
        GT_FALSE /*profileEnable*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfOamMplsCwChannelTypeProfileSet");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}



