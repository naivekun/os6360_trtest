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
* @file prvTgfPclBothUserDefinedBytes.c
*
* @brief Second Lookup match
*
* @version   10
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
#include <pcl/prvTgfPclBothUserDefinedBytes.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS              1

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                5

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            PRV_TGF_PORTS_NUM_CNS

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

#define PCL_RULE_INDEX  0
#define LOOKUP_2_PCL_RULE_INDEX  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(2,PCL_RULE_INDEX) , usedPclLookup = 2
#define LOOKUP_0_PCL_RULE_INDEX  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0,PCL_RULE_INDEX) , usedPclLookup = 0
#define LOOKUP_x_PCL_RULE_INDEX  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(usedPclLookup,PCL_RULE_INDEX)
static GT_U32   usedPclLookup = 0;


/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/* define User Define Byte as end of MAC SA */
static GT_U8         prvTgfUdbArray[] = {0x00, 0x11};

/* UDB to check IPV4 packet: OFFSET_L2 0-8 bytes – Lion-B0 simulation */
static GT_U8         prvTgfUdb_L2_0_8_Array[] = {
    0x00, 0x00, 0x00, 0x00, 0x34, 0x02,
    0x00, 0x00, 0x00
};

/* UDB to check IPV4 packet: OFFSET_L4 0-15 bytes – Lion-B0 simulation */
static GT_U8         prvTgfUdb_L4_0_15_Array[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

/* UDB to check ARP packet: OFFSET_L2 0-11 bytes – Lion-B0 simulation */
static GT_U8         prvTgfUdb_L2_0_11_Array[] = {
    0x00, 0x00, 0x00, 0x00, 0x34, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x11
};

/* UDB to check LLC Non SNAP packet: OFFSET_L2 - 2 0-3 bytes */
static GT_U8         prvTgfUdb_L2_0_3_Array[] = {
    0x02, 0x01, 0x15, 0x16
};

/******************************* IPv4 packet 1 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C9D,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};
/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* LENGTH of packet 1 */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacket1PayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    PRV_TGF_PACKET_LEN_CNS,                                       /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/****************************** ARP packet ************************************/

/* ethertype part of ARP packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketArpEtherTypePart = {
    TGF_ETHERTYPE_0806_ARP_TAG_CNS
};
/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacketArpPart = {
    0x01,                                   /* HW Type */
    0x0800,                                 /* Protocol (IPv4= 0x0800) */
    0x06,                                   /* HW Len = 6 */
    0x04,                                   /* Proto Len = 4 */
    0x01,                                   /* Opcode */
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},   /* nbytes: HW Address of Sender (MAC SA) */
    {0x11, 0x12, 0x13, 0x14},               /* mbytes: Protocol Address of Sender (SIP) */
    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25},   /* nbytes: HW Address of Target (MAC DA) */
    {0x30, 0x31, 0x32, 0x33}                /* mbytes: Protocol Address of Target (DIP) */
};
/* PARTS of packet ARP */
static TGF_PACKET_PART_STC prvTgfPacketArpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketArpEtherTypePart},
    {TGF_PACKET_PART_ARP_E,       &prvTgfPacketArpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* LENGTH of packet ARP */
#define PRV_TGF_PACKET_ARP_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_ARP_HEADER_SIZE_CNS + sizeof(prvTgfPacket1PayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketArpInfo = {
    PRV_TGF_PACKET_ARP_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketArpPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketArpPartArray                                        /* partsArray */
};

/******************************* LLC Non SNAP packet 2 **********************************/
/* DATA of packet 2 */
static GT_U8 prvTgfPacket2PayloadDataArr[] = {
    0x01, 0x00, 0x89, 0x02, 0x01, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};
/* PAYLOAD part of packet 2 */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPacket2PayloadDataArr),                 /* dataLength */
    prvTgfPacket2PayloadDataArr                          /* dataPtr */
};

/* LENGTH of packet 2 */
#define PRV_TGF_PACKET_LLC_NON_SNAP_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPacket2PayloadDataArr)

/* PARTS of packet LLC Non SNAP */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket2PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacket2Info = {
    PRV_TGF_PACKET_LLC_NON_SNAP_LEN_CNS,                           /* totalLen */
    sizeof(prvTgfPacket2PartArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfPacket2PartArray                                         /* partsArray */
};



/******************************************************************************/

/* Current packet to send */
static TGF_PACKET_STC *prvTgfPacketSendPtr = &prvTgfPacket1Info;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/**
* @internal prvTgfPclBothUserDefinedBytesConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclBothUserDefinedBytesConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfPclBothUserDefinedBytesPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclBothUserDefinedBytesPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           udbIndex;
    GT_U8                            udbOffset;

    PRV_UTF_LOG0_MAC("==== TEST of TWO UDBs ====\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, second lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* configure the User Defined Byte (UDB) 0 */
    udbOffset = 10;
    rc = prvTgfPclUdbIndexConvert(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E, 0, &udbIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbIndexConvert");

    /* AUTODOC: configure User Defined Byte (UDB) 0: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_OTHER */
    /* AUTODOC:   offsetType PCL_OFFSET_L2, offset 10 */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                                     PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                     CPSS_PCL_DIRECTION_INGRESS_E,
                                     udbIndex,
                                     PRV_TGF_PCL_OFFSET_L2_E,
                                     udbOffset);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    /* configure the User Defined Byte (UDB) 1 */
    udbOffset = 11;
    rc = prvTgfPclUdbIndexConvert(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E, 1, &udbIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbIndexConvert");

    /* AUTODOC: configure User Defined Byte (UDB) 1: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_OTHER */
    /* AUTODOC:   offsetType PCL_OFFSET_L2, offset 11 */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             udbIndex,
                             PRV_TGF_PCL_OFFSET_L2_E,
                             udbOffset);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    /* set udbErrorCmd */
    rc = prvTgfPclInvalidUdbCmdSet(PRV_TGF_UDB_ERROR_CMD_LOOKUP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclInvalidUdbCmdSet: %d", prvTgfDevNum);

    /* set PCL rule 0 - DROP packet where MAC SA ends on 00 11 */
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));
    cpssOsMemSet(&mask.ruleStdIpL2Qos.udb, 0xFF, sizeof(prvTgfUdbArray));
    cpssOsMemCpy(&pattern.ruleStdIpL2Qos.udb, prvTgfUdbArray, sizeof(prvTgfUdbArray));

    ruleIndex     = LOOKUP_0_PCL_RULE_INDEX;
    ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;

    /* AUTODOC: set PCL rule 0 - DROP_HARD Standard L2+IPv4/v6 QoS with SA=XX:XX:XX:XX:00:11 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclBothUserDefinedBytesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPclBothUserDefinedBytesTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketSendPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send IPv4 packet on port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: verify traffic is dropped */
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], 0, 0,
            prvTgfPacketSendPtr->totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
}

/**
* @internal prvTgfPclBothUserDefinedBytesConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore PCL Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfPclBothUserDefinedBytesConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  ruleIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore current packet by default */
    prvTgfPacketSendPtr = &prvTgfPacket1Info;

    /* -------------------------------------------------------------------------
     * 2. Restore PCL Configuration
     */

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    ruleIndex = LOOKUP_x_PCL_RULE_INDEX;
    /* AUTODOC: invalidate PCL Rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfPclUserDefinedBytes_L2_0_8_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L2_0_8_ConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_PACKET_TYPE_ENT      packetType;
    PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType;
    GT_U32                           udbIndex;
    GT_U8                            udbOffset;
    GT_U32                           udbOffsetMax = sizeof(prvTgfUdb_L2_0_8_Array);

    PRV_UTF_LOG0_MAC("==== TEST of IPV4 packet OFFSET_L2 0-8 bytes ====\n");

    /* set default values */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
    packetType = PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E;
    offsetType = PRV_TGF_PCL_OFFSET_L2_E;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_UDB */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             ruleFormat,
                             ruleFormat,
                             ruleFormat);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* configure the User Defined Bytes */
    for (udbOffset = 0; udbOffset < udbOffsetMax; udbOffset++) {
        rc = prvTgfPclUdbIndexConvert(ruleFormat, udbOffset, &udbIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbIndexConvert");

        /* AUTODOC: configure 9 User Defined Bytes (UDBs): */
        /* AUTODOC:   format INGRESS_STD_UDB */
        /* AUTODOC:   packet type IPV4_OTHER */
        /* AUTODOC:   offsetType PCL_OFFSET_L2, offsets [0..8] */
        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType,
            CPSS_PCL_DIRECTION_INGRESS_E,
            udbIndex, offsetType, udbOffset);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: set udbErrorCmd to UDB_ERROR_CMD_LOOKUP */
    rc = prvTgfPclInvalidUdbCmdSet(PRV_TGF_UDB_ERROR_CMD_LOOKUP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclInvalidUdbCmdSet: %d", prvTgfDevNum);

    /* set PCL rule 0 - DROP packet */
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));

    cpssOsMemSet(&mask.ruleStdUdb.udb, 0xFF, sizeof(prvTgfUdb_L2_0_8_Array));
    cpssOsMemCpy(&pattern.ruleStdUdb.udb, prvTgfUdb_L2_0_8_Array, sizeof(prvTgfUdb_L2_0_8_Array));
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    ruleIndex     = LOOKUP_0_PCL_RULE_INDEX;

    /* AUTODOC: set PCL rule 0 - DROP_HARD packet with UDB=00:00:00:00:34:02:00:00:00 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclUserDefinedBytes_L4_0_15_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L4_0_15_ConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_PACKET_TYPE_ENT      packetType;
    PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType;
    GT_U32                           udbIndex;
    GT_U8                            udbOffset;
    GT_U32                           udbOffsetMax = sizeof(prvTgfUdb_L4_0_15_Array);
    PRV_TGF_PCL_OVERRIDE_UDB_STC     udbOverride;


    PRV_UTF_LOG0_MAC("==== TEST of IPV4 packet OFFSET_L4 0-15 bytes ====\n");

    /* set default values */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
    packetType = PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E;
    offsetType = PRV_TGF_PCL_OFFSET_L4_E;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_UDB */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             ruleFormat,
                             ruleFormat,
                             ruleFormat);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* FIX for lion_B0: smem 0b800014 MUST be 00000000 */
    cpssOsMemSet(&udbOverride,  0, sizeof(udbOverride));
    prvTgfPclOverrideUserDefinedBytesSet(&udbOverride);

    /* configure the User Defined Bytes */
    for (udbOffset = 0; udbOffset < udbOffsetMax; udbOffset++) {
        rc = prvTgfPclUdbIndexConvert(ruleFormat, udbOffset, &udbIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbIndexConvert");

        /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
        /* AUTODOC:   format INGRESS_STD_UDB */
        /* AUTODOC:   packet type IPV4_OTHER */
        /* AUTODOC:   offsetType PCL_OFFSET_L4, offsets [0..15] */
        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType,
            CPSS_PCL_DIRECTION_INGRESS_E,
            udbIndex, offsetType, udbOffset);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: set udbErrorCmd to UDB_ERROR_CMD_LOOKUP */
    rc = prvTgfPclInvalidUdbCmdSet(PRV_TGF_UDB_ERROR_CMD_LOOKUP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclInvalidUdbCmdSet: %d", prvTgfDevNum);

    /* set PCL rule 0 - DROP packet */
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));

    cpssOsMemSet(&mask.ruleStdUdb.udb, 0xFF, sizeof(prvTgfUdb_L4_0_15_Array));
    cpssOsMemCpy(&pattern.ruleStdUdb.udb, prvTgfUdb_L4_0_15_Array, sizeof(prvTgfUdb_L4_0_15_Array));
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    ruleIndex     = LOOKUP_0_PCL_RULE_INDEX;

    /* AUTODOC: set PCL rule 0 - DROP_HARD packet with: */
    /* AUTODOC:   UDB=FA:B5:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclUserDefinedBytes_L2_0_11_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L2_0_11_ConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_PACKET_TYPE_ENT      packetType;
    PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType;
    GT_U32                           udbIndex;
    GT_U8                            udbOffset;
    GT_U32                           udbOffsetMax = sizeof(prvTgfUdb_L2_0_11_Array);

    PRV_UTF_LOG0_MAC("==== TEST of ARP packet OFFSET_L2 0-11 bytes ====\n");

    /* set ARP packet */
    prvTgfPacketSendPtr = &prvTgfPacketArpInfo;

    /* set default values */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
    packetType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    offsetType = PRV_TGF_PCL_OFFSET_L2_E;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_UDB */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             ruleFormat,
                             ruleFormat,
                             ruleFormat);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* configure the User Defined Bytes */
    for (udbOffset = 0; udbOffset < udbOffsetMax; udbOffset++) {
        rc = prvTgfPclUdbIndexConvert(ruleFormat, udbOffset, &udbIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbIndexConvert");

        /* AUTODOC: configure 12 User Defined Bytes (UDBs): */
        /* AUTODOC:   format INGRESS_STD_UDB */
        /* AUTODOC:   packet type ETHERNET_OTHER */
        /* AUTODOC:   offsetType PCL_OFFSET_L2, offsets [0..11] */
        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType,
            CPSS_PCL_DIRECTION_INGRESS_E,
            udbIndex, offsetType, udbOffset);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: set udbErrorCmd to UDB_ERROR_CMD_LOOKUP */
    rc = prvTgfPclInvalidUdbCmdSet(PRV_TGF_UDB_ERROR_CMD_LOOKUP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclInvalidUdbCmdSet: %d", prvTgfDevNum);

    /* set PCL rule 0 - DROP packet */
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));

    cpssOsMemSet(&mask.ruleStdUdb.udb, 0xFF, sizeof(prvTgfUdb_L2_0_11_Array));
    cpssOsMemCpy(&pattern.ruleStdUdb.udb, prvTgfUdb_L2_0_11_Array, sizeof(prvTgfUdb_L2_0_11_Array));
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    ruleIndex     = LOOKUP_0_PCL_RULE_INDEX;

    /* AUTODOC: set PCL rule 0 - DROP_HARD packet with: */
    /* AUTODOC:   UDB=00:00:00:00:34:02:00:00:00:00:00:11 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclUdbL3Minus2IngressLlcNonSnapTest function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for LLC Non SNAP packets
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressLlcNonSnapTest
(
    GT_VOID
)
{
    /* Set bridge configuration */
    prvTgfPclBothUserDefinedBytesConfigurationSet();

    /* 1. Set PCL configuration to check LLC Non SNAP packet:
     * OFFSET_L2 0-3 bytes
     * Generate traffic
     */
    prvTgfPclUserDefinedBytes_L2_0_3_ConfigurationSet();
    prvTgfPclBothUserDefinedBytesTrafficGenerate();

    /* Restore configuration */
    prvTgfPclBothUserDefinedBytesConfigurationRestore();

    prvTgfPclRestore();
}

/**
* @internal prvTgfPclUserDefinedBytes_L2_0_3_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L2_0_3_ConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_PACKET_TYPE_ENT      packetType;
    PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType;
    GT_U32                           udbIndex;
    GT_U8                            udbOffset;
    GT_U32                           udbOffsetMax = sizeof(prvTgfUdb_L2_0_3_Array);

    PRV_UTF_LOG0_MAC("==== TEST of LLC Non SNAP packet OFFSET_L2 0-3 bytes ====\n");

    /* set LLC Non SNAP packet */
    prvTgfPacketSendPtr = &prvTgfPacket2Info;

    /* set default values */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
    packetType = PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
    offsetType = PRV_TGF_PCL_OFFSET_L3_MINUS_2_E;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_UDB */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             ruleFormat,
                             ruleFormat,
                             ruleFormat);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* configure the User Defined Bytes */
    for (udbOffset = 0; udbOffset < udbOffsetMax; udbOffset++) {
        rc = prvTgfPclUdbIndexConvert(ruleFormat, udbOffset, &udbIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbIndexConvert");

        /* AUTODOC: configure 4 User Defined Bytes (UDBs): */
        /* AUTODOC:   format INGRESS_STD_UDB */
        /* AUTODOC:   packet type ETHERNET_OTHER */
        /* AUTODOC:   offsetType PCL_OFFSET_L3_MINUS_2, offsets [0..3] */
        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType,
            CPSS_PCL_DIRECTION_INGRESS_E,
            udbIndex, offsetType, udbOffset);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: set udbErrorCmd to UDB_ERROR_CMD_LOOKUP */
    rc = prvTgfPclInvalidUdbCmdSet(PRV_TGF_UDB_ERROR_CMD_LOOKUP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclInvalidUdbCmdSet: %d", prvTgfDevNum);

    /* set PCL rule 0 - DROP packet */
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));
    cpssOsMemSet(&mask.ruleStdUdb.udb, 0xFF, sizeof(prvTgfUdb_L2_0_3_Array));
    cpssOsMemCpy(&pattern.ruleStdUdb.udb, prvTgfUdb_L2_0_3_Array, sizeof(prvTgfUdb_L2_0_3_Array));
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    ruleIndex     = LOOKUP_0_PCL_RULE_INDEX;

    /* AUTODOC: set PCL rule 0 - DROP_HARD packet with UDB = 04:00:89:02 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);
}


