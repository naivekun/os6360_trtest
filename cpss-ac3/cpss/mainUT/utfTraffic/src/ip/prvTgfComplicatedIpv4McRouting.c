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
* @file prvTgfComplicatedIpv4McRouting.c
*
* @brief Complicated IPV4 MC Routing
*
* @version   14
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfComplicatedIpv4McRouting.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* send VLAN */
#define PRV_TGF_SEND_VLANID_CNS           80

/* multicast subscriber VLAN */
#define PRV_TGF_NEXTHOP_VLANID_CNS        55

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         2

/* port index to receive traffic from */
#define PRV_TGF_RCV_PORT_IDX_CNS          1

/* port index to receive traffic from */
#define PRV_TGF_RCV1_PORT_IDX_CNS         0

/* default port index to receive traffic from */
#define PRV_TGF_DEF_RCV_PORT_IDX_CNS      3

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS      2

/* number of send stages */
#define PRV_TGF_STAGES_NUM_CNS            4

/* default vidx number */
#define PRV_TGF_DEF_VIDX_CNS              0

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* MLL pair pointer index */
static GT_U32        prvTgfMllPointerIndex     = 5;

/* Default Route entry index for MC Route entry Table */
static GT_U32        prvTgfDefRouteEntryIndex  = 1;

/* the Route entry index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 10;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the Virtual Router id for LPM Table */
static GT_U32        prvTgfVrId                = 0;

/* expected number of packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountRxTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS * PRV_TGF_STAGES_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 1, 1, 1},
    {1, 0, 1, 1},
    {0, 1, 1, 1},
    {0, 0, 1, 2},
    {0, 1, 1, 1},
    {1, 0, 1, 1},
    {0, 0, 1, 2},
    {0, 0, 1, 2},
};

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* Source IP addresses for packets */
static TGF_IPV4_ADDR  prvTgfSrcAddrArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    {149, 218, 213, 102},
    { 10,  99,   0, 102}
};

/* port indexes for MC subscribers */
static GT_U8 prvTgfMcPortsIdxArr[PRV_TGF_STAGES_NUM_CNS][PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    {PRV_TGF_RCV_PORT_IDX_CNS,     PRV_TGF_RCV1_PORT_IDX_CNS},
    {PRV_TGF_RCV_PORT_IDX_CNS,     PRV_TGF_DEF_RCV_PORT_IDX_CNS},
    {PRV_TGF_RCV_PORT_IDX_CNS,     PRV_TGF_RCV1_PORT_IDX_CNS},
    {PRV_TGF_DEF_RCV_PORT_IDX_CNS, PRV_TGF_DEF_RCV_PORT_IDX_CNS}
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x11},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                /* version */
    5,                                /* headerLen */
    0,                                /* typeOfService */
    0x2A,                             /* totalLen = 42 */
    0,                                /* id */
    0,                                /* flags */
    0,                                /* offset */
    0x40,                             /* timeToLive = 64 */
    0x04,                             /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    {149, 218, 213, 102},             /* srcAddr = ipSrc */
    {225,   1,   1,  17}              /* dstAddr = ipGrp */
};

/* DATA of packet len = 22 (0x16) */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U16              vid;
    CPSS_PORTS_BMP_STC  defPortMembers;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfComplicatedIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfComplicatedIpv4McRoutingBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS           rc          = GT_OK;
    CPSS_PORTS_BMP_STC  portMembers = {{0, 0}};

    /* AUTODOC: SETUP CONFIGURATION: */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portMembers);

    /* AUTODOC: create VLAN 80 with untagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray + 2, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 55 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
                                           prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID 80 for port 2 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* save default port members for MC group */
    rc = prvTgfBrgMcEntryRead(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                              &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryRead: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* create bmp with ports for MC group */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_RCV1_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS]);

    /* AUTODOC: create VIDX 0 with ports 0,1,3 */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS, &portMembers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* AUTODOC: add FDB entry with MAC 01:00:5e:01:01:11, VLAN 80, VIDX 0 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_SEND_VLANID_CNS,
                                          PRV_TGF_DEF_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

/**
* @internal prvTgfComplicatedIpv4McRoutingDefaultLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfComplicatedIpv4McRoutingDefaultLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC             *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* AUTODOC: enable IPv4 Multicast Routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Multicast Routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
                                   CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* create MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_SEND_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;
    mllPairEntry.nextPointer                     = 0;

    /* AUTODOC: create 1 MC Link Lists 0: */
    /* AUTODOC:   1. port 3, VLAN 80 */
    rc = prvTgfIpMllPairWrite(0,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr = 0;

    /* AUTODOC: add MC route entry with index 1, MLL Ptr 0 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfDefRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");
}

/**
* @internal prvTgfComplicatedIpv4McRoutingDefaultTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic to defaults
*/
GT_VOID prvTgfComplicatedIpv4McRoutingDefaultTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc        = GT_OK;
    GT_U32                      portIter  = 0;
    GT_U32                      packetLen = 0;
    PRV_TGF_IP_COUNTER_SET_STC  ipCounters;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset IP couters and set ROUTE_ENTRY mode */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv4 packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:11, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=80, srcIP=149.218.213.102, dstIP=225.1.1.17 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    if (GT_TRUE == prvUtfIsGmCompilation())
                cpssOsTimerWkAfter(1000);

    /* check ETH counter of ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* calculate packet length */
        packetLen = prvTgfPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS);

        /* AUTODOC: verify routed packet on default port 3 */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    (GT_U8) (portIter == PRV_TGF_SEND_PORT_IDX_CNS),
                                    prvTgfPacketsCountRxTxArr[3][portIter],
                                    packetLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfComplicatedIpv4McRoutingAdditionalLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfComplicatedIpv4McRoutingAdditionalLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_IPADDR                       ipGrp, ipSrc;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC             *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* create MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_NEXTHOP_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;
    mllPairEntry.nextPointer                     = 0;

    /* AUTODOC: create 1 MC Link List 5: */
    /* AUTODOC:   1. port 1, VLAN 55 */
    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;

    /* AUTODOC: add MC route entry with index 10, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the LPM */
    cpssOsMemCpy(ipGrp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipGrp.arIP));
    cpssOsMemCpy(ipSrc.arIP, prvTgfSrcAddrArr[0], sizeof(ipSrc.arIP));

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=225.1.1.17/32, ipSrc=149.218.213.102/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, prvTgfVrId,
                                   ipGrp, 32, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

    /* create another MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RCV1_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_NEXTHOP_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;
    mllPairEntry.nextPointer                     = 0;

    /* AUTODOC: create 1 MC Link List 6: */
    /* AUTODOC:   1. port 0, VLAN 55 */
    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex + 1,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex + 1;

    /* AUTODOC: add MC route entry with index 11, MLL Ptr 6 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex + 1, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex + 1;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the LPM */
    cpssOsMemCpy(ipSrc.arIP, prvTgfSrcAddrArr[1], sizeof(ipSrc.arIP));

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=225.1.1.17/32, ipSrc=10.99.0.102/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, prvTgfVrId,
                                   ipGrp, 32, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfComplicatedIpv4McRoutingAdditionalRouteConfigurationSet function
* @endinternal
*
* @brief   Set additional Route Configuration
*/
GT_VOID prvTgfComplicatedIpv4McRoutingAdditionalRouteConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet");

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            PRV_UTF_LOG0_MAC("==== MC IS NOT SUPPORTED IN PBR MODE ====\n");
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            prvTgfComplicatedIpv4McRoutingAdditionalLttRouteConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:
        default:
            PRV_UTF_LOG0_MAC("==== ROUTING MODE IS NOT DEFINED ====\n");
    }
}

/**
* @internal prvTgfComplicatedIpv4McRoutingAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfComplicatedIpv4McRoutingAdditionalTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    GT_U32                          sendIter    = 0;
    GT_U32                          stageIter   = 0;
    GT_U32                          numTriggers = 0;
    GT_U32                          expTriggers = 0;
    GT_U8                           expCountRx  = 0;
    GT_U8                           expCountTx  = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          packetLen   = 0;
    GT_IPADDR                       ipGrp, ipSrc       = {0};
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry    = {0};
    PRV_TGF_IP_LTT_ENTRY_STC        rcvMcRouteLttEntry = {0};
    GT_U32                          rcvTcamGroupRowIndex    = 0;
    GT_U32                          rcvTcamGroupColumnIndex = 0;
    GT_U32                          rcvTcamSrcRowIndex      = 0;
    GT_U32                          rcvTcamSrcColumnIndex   = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* AUTODOC: Iterate thru 4 stages: */
    for (stageIter = 0; stageIter < PRV_TGF_STAGES_NUM_CNS; stageIter++)
    {
        for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
        {
            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture on a MC subscriber port */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[prvTgfMcPortsIdxArr[stageIter][sendIter]];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[prvTgfMcPortsIdxArr[stageIter][sendIter]]);

            /* reset IP couters and set ROUTE_ENTRY mode */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                             prvTgfDevNum, prvTgfPortsArray[portIter]);
            }

            /* reset ETH counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

            /* fill source IP address for packet */
            cpssOsMemCpy(prvTgfPacketIpv4Part.srcAddr, prvTgfSrcAddrArr[sendIter],
                         sizeof(prvTgfPacketIpv4Part.srcAddr));

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("========== Sending %d packet (stage %d) ==========\n",
                             sendIter + 1, stageIter + 1);

            /* AUTODOC: for each stage send 2 IPv4 packets from port 2 with: */
            /* AUTODOC:   1. DA=01:00:5e:01:01:11, SA=00:00:00:00:00:33 */
            /* AUTODOC:   VID=80, srcIP=149.218.213.102, dstIP=225.1.1.17 */
            /* AUTODOC:   2. DA=01:00:5e:01:01:11, SA=00:00:00:00:00:33 */
            /* AUTODOC:   VID=80, srcIP=10.99.0.102, dstIP=225.1.1.17 */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            if (GT_TRUE == prvUtfIsGmCompilation())
                cpssOsTimerWkAfter(1000);

            /* disable capture on a MC subscriber port */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* check ETH counter of ports for MC subscriber */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                /* calculate packet length */
                packetLen = prvTgfPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS);

                /* calculate counters */
                expCountRx = (GT_U8)((portIter == PRV_TGF_SEND_PORT_IDX_CNS) ? 1 :
                                     (portIter == prvTgfMcPortsIdxArr[stageIter][sendIter]) ?
                                     prvTgfPacketsCountRxTxArr[sendIter + stageIter * PRV_TGF_SEND_PACKETS_NUM_CNS][portIter] : 0);
                expCountTx = prvTgfPacketsCountRxTxArr[sendIter + stageIter * PRV_TGF_SEND_PACKETS_NUM_CNS][portIter];

                /* AUTODOC: verify traffic: */
                /* AUTODOC:   stage#1: 1 packet received on ports 1,3 */
                /* AUTODOC:   stage#1: 2 packet received on ports 0,3 */
                /* AUTODOC:   stage#2: 1 packet received on ports 1,3 */
                /* AUTODOC:   stage#2: 2 packets received on port 3 */
                /* AUTODOC:   stage#3: 1 packet received on ports 1,3 */
                /* AUTODOC:   stage#3: 2 packet received on ports 0,3 */
                /* AUTODOC:   stage#4: 1 packets received on port 3 */
                /* AUTODOC:   stage#4: 2 packets received on port 3 */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            expCountRx, expCountTx,
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }

            /* get Trigger Counters */
            PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

            /* get trigger counters where packet has MAC DA as prvTgfArpMac */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 0;
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            /* check TriggerCounters */
            rc = rc == GT_NO_MORE ? GT_OK : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

            /* check if captured packet has the same MAC DA as prvTgfArpMac */
            expTriggers = (1 << prvTgfPacketsCountRxTxArr[sendIter + stageIter * PRV_TGF_SEND_PACKETS_NUM_CNS][prvTgfMcPortsIdxArr[stageIter][sendIter]]) - 1;

            UTF_VERIFY_EQUAL6_STRING_MAC(expTriggers, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

            /* get and print ip counters values */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
                prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
            }
        }

        /* fill a destination IP address for the LPM */
        cpssOsMemCpy(ipGrp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipGrp.arIP));

        for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
        {
            /* fill a source IP address for the LPM */
            cpssOsMemCpy(ipSrc.arIP, prvTgfSrcAddrArr[sendIter], sizeof(ipSrc.arIP));

            /* find specific multicast adrress entry */
            rc = prvTgfIpLpmIpv4McEntrySearch(prvTgfLpmDBId, prvTgfVrId,
                                              ipGrp, 32, ipSrc, 32, &rcvMcRouteLttEntry,
                                              &rcvTcamGroupRowIndex, &rcvTcamGroupColumnIndex,
                                              &rcvTcamSrcRowIndex, &rcvTcamSrcColumnIndex);
            rc = (((stageIter == 1) && (sendIter == 1)) || (stageIter == 3)) ? ((GT_NOT_FOUND == rc) ? GT_OK : rc) : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntrySearch: %d, %d",
                                         prvTgfLpmDBId, prvTgfVrId);
        }

        /* modify configuration for each stage */
        switch (stageIter)
        {
            case 0:
                /* AUTODOC: after Stage#1 delete IPv4 MC prefix: */
                /* AUTODOC:   ipGrp=225.1.1.17/32, ipSrc=10.99.0.102/32 */
                rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, prvTgfVrId,
                                               ipGrp, 32, ipSrc, 32);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryDel: %d, %d",
                                             prvTgfDevNum, prvTgfVrId);

                break;

            case 1:
                /* fill a nexthope info for the LPM */
                cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
                mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex + 1;
                mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

                /* AUTODOC: after Stage#2 restore deleted MC Ipv4 prefix */
                rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, prvTgfVrId,
                                               ipGrp, 32, ipSrc, 32,
                                               &mcRouteLttEntry, GT_FALSE, GT_FALSE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d, %d",
                                             prvTgfDevNum, prvTgfVrId);

                break;

            case 2:
                /* AUTODOC: after Stage#3 flush all MC Ipv4 prefix */
                rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, prvTgfVrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush: %d, %d",
                                             prvTgfDevNum, prvTgfVrId);

                break;

            default:
                break;
        }
    }
}

/**
* @internal prvTgfComplicatedIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfComplicatedIpv4McRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* 2. Restore Route Configuration */

    /* restore MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET0_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr = 0;

    /* AUTODOC: restore default values for MC route entry with index 1, MLL Ptr 0 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfDefRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

    /* AUTODOC: flush all MC Ipv4 prefix */
    rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, prvTgfVrId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush");

    /* AUTODOC: disable multicast IPv4 routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable multicast IPv4 routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* 1. Restore Base Configuration */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* AUTODOC: restore default MC Group entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                               &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

    /* invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_SEND_VLANID_CNS);

    /* invalidate nexthop VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_NEXTHOP_VLANID_CNS);
}


