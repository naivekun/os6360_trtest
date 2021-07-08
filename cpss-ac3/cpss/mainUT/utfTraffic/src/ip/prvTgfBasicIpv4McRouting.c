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
* @file prvTgfBasicIpv4McRouting.c
*
* @brief Basic IPV4 MC Routing
*
* @version   39
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfBasicIpv4McRouting.h>
#include <gtOs/gtOsMem.h>

extern GT_STATUS appDemoHwAccessFailerBind(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
);

extern GT_STATUS appDemoHwAccessCounterGet(
    OUT GT_U32   *hwAccessCounterPtr
);


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* send VLAN */
#define PRV_TGF_SEND_VLANID_CNS           80

/* multicast subscriber 1 VLAN */
#define PRV_TGF_MC_1_VLANID_CNS           55

/* multicast subscriber 2 VLAN */
#define PRV_TGF_MC_2_VLANID_CNS           17

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         2

/* port index for subscriber 1 */
#define PRV_TGF_MC_1_PORT_IDX_CNS         1

/* port index for subscriber 2 */
#define PRV_TGF_MC_2_PORT_IDX_CNS         3

/* port index for subscriber 3 */
#define PRV_TGF_MC_3_PORT_IDX_CNS         0

/* default vidx number */
#define PRV_TGF_DEF_VIDX_CNS              0

#define SECOND_HEMISPHERE_CNS   64

/* is this a call from Dual HW Device test */
static GT_BOOL       isDualHwDevTest = GT_FALSE;

/* ports numbers for multi hemisphere devices */
static GT_U32    testPortsForHwDevArr[PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS] =
{
    0, 34, 10+SECOND_HEMISPHERE_CNS, 58+SECOND_HEMISPHERE_CNS

};

/* config restore ports array */
static GT_U32   prvTgfDefPortsArr[PRV_TGF_MAX_PORTS_NUM_CNS] = {0};

/* config restore number of ports array */
static GT_U8   prvTgfDefPortsNum = 0;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* MLL pair pointer indexes */
static GT_U32        prvTgfMllPointerIndex     = 5;
static GT_U32        prvTgfNextMllPointerIndex = 6;

/* the Route entry index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 10;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* expected number of sent packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountRxMC1 = 0;
static GT_U8 prvTgfPacketsCountRxMC2 = 1;
static GT_U8 prvTgfPacketsCountRxMC3 = 0;

/* expected number of sent packets on MC subscriber's ports */
    static GT_U8 prvTgfPacketsCountTxMC1 = 1;
static GT_U8 prvTgfPacketsCountTxMC2 = 1;
static GT_U8 prvTgfPacketsCountTxMC3 = 1;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

static PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr = ipSrc */
    { 224,1,  1,  1}                   /* dstAddr = ipGrp */
};
/* DATA of packet len = 22 (0x16) */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS           rc = GT_OK;
    GT_U8               tagArray[]  = {1, 1, 1, 1};
    CPSS_PORTS_BMP_STC  portMembers = {{0, 0}};

    /* AUTODOC: SETUP CONFIGURATION: */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portMembers);

    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
       (isDualHwDevTest == GT_TRUE))
    {
        /* AUTODOC: create VLAN 80 with tagged port 1,2 */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                prvTgfPortsArray + 1, NULL, tagArray, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: create VLAN 80 with untagged port 2 */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                prvTgfPortsArray + 2, NULL, NULL, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    }
    /* AUTODOC: create VLAN 55 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_MC_1_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 17 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_MC_2_VLANID_CNS,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfBasicIpv4McRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfBasicIpv4McRoutingLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_IPADDR                       ipGrp, ipSrc;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC             *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: enable MC IPv4 Routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 MC Routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* create first and second MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_MC_1_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_FALSE;

    mllNodePtr = &mllPairEntry.secondMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_MC_2_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_FALSE;

    mllPairEntry.nextPointer                     = (GT_U16)prvTgfNextMllPointerIndex;

    /* AUTODOC: create 1 and 2 MC Link List 5: */
    /* AUTODOC:   1. port 1, VLAN 55 */
    /* AUTODOC:   2. port 3, VLAN 17 */
    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex,
                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");
    /* create third MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_MC_1_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;

    mllPairEntry.nextPointer                     = 0;

    /* AUTODOC: create 3 MC Link List 6: */
    /* AUTODOC:   3. port 0, VLAN 55 */
    rc = prvTgfIpMllPairWrite(prvTgfNextMllPointerIndex,
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
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex,
                                     1,
                                     protocol,
                                     mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the LPM */
    cpssOsMemCpy(ipGrp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipGrp.arIP));
    cpssOsMemCpy(ipSrc.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(ipSrc.arIP));

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.1.1.3/32, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 32, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv4McRoutingRouteConfigurationMllSilentDropSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] overwriteTtl             - GT_TRUE: Just overwrite the TTL value
*                                      GT_FALSE: do full configuration
* @param[in] ttlHopLimitThresholdValue - ttl value to set to L2 mll elements
*                                       None
*/
GT_VOID prvTgfBasicIpv4McRoutingRouteConfigurationMllSilentDropSet
(
    GT_BOOL     overwriteTtl,
    GT_U32      ttlHopLimitThresholdValue
)
{
    GT_STATUS                       rc = GT_OK;
    GT_IPADDR                       ipGrp, ipSrc;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC             *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;
    PRV_TGF_L2_MLL_PAIR_STC         l2MllPairEntry;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC    l2LttEntry;
    GT_U32                          l2LttIndex;

    if(overwriteTtl==GT_FALSE)
    {
        /* AUTODOC: enable MC IPv4 Routing on port 2 */
        rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                       CPSS_IP_MULTICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: enable IPv4 MC Routing on Vlan 80 */
        rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
                                       CPSS_IP_MULTICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        l2LttEntry.mllPointer = 1;
        l2LttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
        l2LttEntry.mllMaskProfile=0;
        l2LttEntry.mllMaskProfileEnable=GT_FALSE;
        l2LttIndex = 1;/* 4097(=IP MLL eVidx) - (4095(=maxVidxIndex used as the base for L2 MLL) + 1)*/

        /* AUTODOC: set the MLL LTT entry that point to this MLL */
        rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, l2LttIndex, &l2LttEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet");


         /* create first and second MC Link List */
        cpssOsMemSet(&l2MllPairEntry, 0, sizeof(l2MllPairEntry));

        l2MllPairEntry.firstMllNode.egressInterface.type            = CPSS_INTERFACE_PORT_E;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];
        l2MllPairEntry.firstMllNode.ttlThreshold                     = ttlHopLimitThresholdValue;
        l2MllPairEntry.firstMllNode.last                              = GT_FALSE;

        l2MllPairEntry.secondMllNode.egressInterface.type            = CPSS_INTERFACE_PORT_E;
        l2MllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
        l2MllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS];
        l2MllPairEntry.secondMllNode.ttlThreshold                     = ttlHopLimitThresholdValue;
        l2MllPairEntry.secondMllNode.last                             = GT_FALSE;

        l2MllPairEntry.nextPointer=2;

        /* AUTODOC: create 1 and 2 MC Link List 1: */
        /* AUTODOC:   1. port 1, VLAN 55 */
        /* AUTODOC:   2. port 3, VLAN 17 */
        rc = prvTgfL2MllPairWrite(prvTgfDevNum,1,
                                  PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                                  &l2MllPairEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

       /* create first and second MC Link List */
        cpssOsMemSet(&l2MllPairEntry, 0, sizeof(l2MllPairEntry));

        l2MllPairEntry.firstMllNode.egressInterface.type            = CPSS_INTERFACE_PORT_E;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS];
        l2MllPairEntry.firstMllNode.ttlThreshold                     = ttlHopLimitThresholdValue;
        l2MllPairEntry.firstMllNode.last                             = GT_TRUE;

        /* AUTODOC: create 1 and 2 MC Link List 2: */
        /* AUTODOC:   1. port 1, VLAN 55 */
        /* AUTODOC:   2. port 3, VLAN 17 */
        rc = prvTgfL2MllPairWrite(prvTgfDevNum,2,
                                  PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                                  &l2MllPairEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

        /* create first and second MC Link List */
        cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

        mllNodePtr = &mllPairEntry.firstMllNode;
        mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_VIDX_E;
        mllNodePtr->nextHopInterface.vidx            = 4097;
        mllNodePtr->nextHopVlanId                    = PRV_TGF_MC_1_VLANID_CNS;
        mllNodePtr->ttlHopLimitThreshold             = 4;
        mllNodePtr->last                             = GT_FALSE;

        mllNodePtr = &mllPairEntry.secondMllNode;
        mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
        mllNodePtr->nextHopInterface.devPort.portNum =
                                      prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];
        mllNodePtr->nextHopVlanId                    = PRV_TGF_MC_1_VLANID_CNS;
        mllNodePtr->ttlHopLimitThreshold             = 4;
        mllNodePtr->last                             = GT_TRUE;

        mllPairEntry.nextPointer                     = 0;

        /* AUTODOC: create 1 and 2 MC Link List 5: */
        /* AUTODOC:   1. port 1, VLAN 55 */
        /* AUTODOC:   2. port 3, VLAN 17 */
        rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex,
                                  PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                                  &mllPairEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

        /* create a MC Route entry */
        cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
        mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
        mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
        mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;

        /* AUTODOC: add MC route entry with index 10, MLL Ptr 5 */
        rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex,
                                         1,
                                         protocol,
                                         mcRouteEntryArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

        /* fill a nexthope info for the LPM */
        cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
        mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

        /* fill a destination IP address for the LPM */
        cpssOsMemCpy(ipGrp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipGrp.arIP));
        cpssOsMemCpy(ipSrc.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(ipSrc.arIP));

        /* AUTODOC: add IPv4 MC prefix: */
        /* AUTODOC:   ipGrp=224.1.1.3/32, ipSrc=1.1.1.3/32 */
        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 32, ipSrc, 32,
                                       &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
               "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);
    }
    else
    {
        /* create first and second MC Link List */
        cpssOsMemSet(&l2MllPairEntry, 0, sizeof(l2MllPairEntry));

        l2MllPairEntry.firstMllNode.egressInterface.type            = CPSS_INTERFACE_PORT_E;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];
        l2MllPairEntry.firstMllNode.ttlThreshold                     = ttlHopLimitThresholdValue;
        l2MllPairEntry.firstMllNode.last                              = GT_FALSE;

        l2MllPairEntry.secondMllNode.egressInterface.type            = CPSS_INTERFACE_PORT_E;
        l2MllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
        l2MllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS];
        l2MllPairEntry.secondMllNode.ttlThreshold                     = ttlHopLimitThresholdValue;
        l2MllPairEntry.secondMllNode.last                             = GT_FALSE;

        l2MllPairEntry.nextPointer=2;

        /* AUTODOC: create 1 and 2 MC Link List 1: */
        /* AUTODOC:   1. port 1, VLAN 55 */
        /* AUTODOC:   2. port 3, VLAN 17 */
        rc = prvTgfL2MllPairWrite(prvTgfDevNum,1,
                                  PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                                  &l2MllPairEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

       /* create first and second MC Link List */
        cpssOsMemSet(&l2MllPairEntry, 0, sizeof(l2MllPairEntry));

        l2MllPairEntry.firstMllNode.egressInterface.type            = CPSS_INTERFACE_PORT_E;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
        l2MllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS];
        l2MllPairEntry.firstMllNode.ttlThreshold                     = ttlHopLimitThresholdValue;
        l2MllPairEntry.firstMllNode.last                             = GT_TRUE;

        /* AUTODOC: create 1 and 2 MC Link List 2: */
        /* AUTODOC:   1. port 1, VLAN 55 */
        /* AUTODOC:   2. port 3, VLAN 17 */
        rc = prvTgfL2MllPairWrite(prvTgfDevNum,2,
                                  PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                                  &l2MllPairEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");
    }
}

/**
* @internal prvTgfBasicIpv4McRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingRouteConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet");

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            PRV_UTF_LOG0_MAC("==== MC IS NOT SUPPORTED IN PBR MODE ====\n");
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            prvTgfBasicIpv4McRoutingLttRouteConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}

/**
* @internal prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource function
* @endinternal
*
* @brief   Set source and group for the packet and for the mc entry
*/
GT_VOID prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource
(
    IN GT_IPADDR                    ipGrp,
    IN GT_U32                       ipGrpPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN GT_BOOL                      flushMcTable
)
{
    GT_U32 rc;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (flushMcTable==GT_TRUE)
    {
        /* AUTODOC: flush all MC Ipv4 prefix */
        rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush");
    }
    /* set packet source and destination IPs */
    cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, ipGrp.arIP, sizeof(ipGrp.arIP));
    cpssOsMemCpy(prvTgfPacketIpv4Part.srcAddr, ipSrc.arIP, sizeof(ipSrc.arIP));



    /* copy bits 0..24  of dst Ip to mac DA - prevent ipv4 mac DA mismatch exception */
    prvTgfPacketL2Part.daMac[5] = prvTgfPacketIpv4Part.dstAddr[3];
    prvTgfPacketL2Part.daMac[4] = prvTgfPacketIpv4Part.dstAddr[2];
    prvTgfPacketL2Part.daMac[3] = prvTgfPacketIpv4Part.dstAddr[1];

    prvTgfPacketL2Part.daMac[3]&=0x7F;/* MAC_DA[23]=0 */

    /* AUTODOC: add IPv4 MC prefix: */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0,
                         ipGrp, ipGrpPrefixLen, ipSrc, ipSrcPrefixLen,
                                   &mcRouteLttEntry, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);
}



/**
* @internal prvTgfBasicIpv4McRoutingDifferentMcEntries function
* @endinternal
*
* @brief   Main test function of tgfBasicIpv4McRoutingDifferentMcEntries
*/
GT_VOID prvTgfBasicIpv4McRoutingDifferentMcEntries
(
    GT_VOID
)
{
    GT_IPADDR   ipGrp;
    GT_IPADDR   ipSrc;

    prvTgfBasicIpv4McRoutingBaseConfigurationSet();
    prvTgfBasicIpv4McRoutingRouteConfigurationSet();

    /* AUTODOC: set legal IPV4 Address: group 224.1.1.1/32, source 1.1.1.1/32, expect traffic */
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: set IPv4 MC Illegal DIP Address - Multicast class D */
    /* AUTODOC: group 224.0.0.0/4, source 1.0.0.0/0, expect traffic */

    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 0;
    ipGrp.arIP[2] = 0;
    ipGrp.arIP[3] = 0;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 0;
    ipSrc.arIP[2] = 0;
    ipSrc.arIP[3] = 0;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 4, ipSrc, 0, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: set IPv4 MC Illegal SIP Address - 0/0 - illegal SIP */
    /* AUTODOC: group 224.2.3.4/32, source 0.0.0.0/0, expect no traffic (dropped) */

    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 2;
    ipGrp.arIP[2] = 3;
    ipGrp.arIP[3] = 4;

    ipSrc.arIP[0] = 0;
    ipSrc.arIP[1] = 0;
    ipSrc.arIP[2] = 0;
    ipSrc.arIP[3] = 0;

    /* there is no option to forward such exceptions (see exceptions db in file cpssDxChIpCtrl.c) */
    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 32, ipSrc, 0, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_FALSE); /* IPv4 MC Illegal SIP Address, no option to forward */

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: set IPv4 MC entry: group 224.2.3.4/32, source 1.2.3.4/32, expect traffic */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 2;
    ipGrp.arIP[2] = 3;
    ipGrp.arIP[3] = 4;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 2;
    ipSrc.arIP[2] = 3;
    ipSrc.arIP[3] = 4;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 32, ipSrc, 32, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: set legal IPV4 Address: group 224.1.1.1/32, source 1.2.3.4/8, expect traffic */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 1;
    ipGrp.arIP[2] = 1;
    ipGrp.arIP[3] = 1;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 2;
    ipSrc.arIP[2] = 3;
    ipSrc.arIP[3] = 4;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 32, ipSrc, 8, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: set legal IPV4 Address: group 224.1.1.1/8, source 1.2.3.4/8, expect traffic */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 1;
    ipGrp.arIP[2] = 1;
    ipGrp.arIP[3] = 1;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 2;
    ipSrc.arIP[2] = 3;
    ipSrc.arIP[3] = 4;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 8, ipSrc, 8, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: set legal IPV4 Address: group 224.0.0.0/4, source 0.0.0.1/8, expect traffic */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 0;
    ipGrp.arIP[2] = 0;
    ipGrp.arIP[3] = 0;

    ipSrc.arIP[0] = 0;
    ipSrc.arIP[1] = 0;
    ipSrc.arIP[2] = 0;
    ipSrc.arIP[3] = 1;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 4, ipSrc, 8, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: set legal IPV4 Address: group 224.0.0.0/4, source 1.2.3.4/32, expect traffic */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 0;
    ipGrp.arIP[2] = 0;
    ipGrp.arIP[3] = 0;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 2;
    ipSrc.arIP[2] = 3;
    ipSrc.arIP[3] = 4;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 4, ipSrc, 32, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* AUTODOC: set legal IPV4 Address: group 224.0.0.0/8, source 1.2.3.4/32, expect traffic */
    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 8, ipSrc, 32, GT_TRUE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------------------- */
    /* AUTODOC: restore the packet: group 224.1.1.1/32, source 1.1.1.1/32 */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 1;
    ipGrp.arIP[2] = 1;
    ipGrp.arIP[3] = 1;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 1;
    ipSrc.arIP[2] = 1;
    ipSrc.arIP[3] = 1;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 32, ipSrc, 32, GT_TRUE);
    prvTgfBasicIpv4McRoutingConfigurationRestore();
}


/**
* @internal prvTgfBasicIpv4McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv4McRoutingTrafficGenerate
(
    GT_BOOL expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInt;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          mllOutMCPkts;
    GT_U32                          oldMllOutMCPktsSet0;
    GT_U32                          oldMllOutMCPktsSet1;
    GT_U32                          mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable capture on a MC subscriber port */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
           TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
           "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
           prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS]);

    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    /* set MLL counters mode for MC subscriber 1 */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
    cntIntCfg.devNum           = prvTgfDevNum;
    cntIntCfg.portTrunkCntMode = PRV_TGF_IP_PORT_CNT_MODE_E;
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* set MLL counters mode for MC subscriber 2 */
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv4 packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=224.1.1.1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of StartTransmitting: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(100);

    /* disable capture on a MC subscriber port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    if(expectTraffic==GT_TRUE)
    {
        /* AUTODOC: verify routed packet on ports 0,1,3 */
        if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
           (isDualHwDevTest == GT_TRUE))
        {
            /* AUTODOC: verify 1 flood in ingress VLAN 80 to port 1 */
            prvTgfPacketsCountTxMC1 = 2;
        }

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS],
                prvTgfPacketsCountRxMC3, prvTgfPacketsCountTxMC3,
                prvTgfPacketInfo.totalLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS],
                prvTgfPacketsCountRxMC1, prvTgfPacketsCountTxMC1,
                prvTgfPacketInfo.totalLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS],
                prvTgfPacketsCountRxMC2, prvTgfPacketsCountTxMC2,
                prvTgfPacketInfo.totalLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInt.devPort.portNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr,
                     prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInt, 1, vfdArray,
                                                            &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
            portInt.devPort.hwDevNum, portInt.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

        /* get and print ip counters values */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
            PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n",
                             prvTgfPortsArray[portIter]);
            prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
        }
        PRV_UTF_LOG0_MAC("\n");
    }
    else
    {
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS],
                0, 0,
                prvTgfPacketInfo.totalLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS],
                0, 0,
                prvTgfPacketInfo.totalLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS],
                0, 0,
                prvTgfPacketInfo.totalLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    }
    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 1: mllOutMCPkts = %d\n", mllOutMCPkts - oldMllOutMCPktsSet0);

    /* get MLL counters for MC subscriber 2 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_1, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 2: mllOutMCPkts = %d\n", mllOutMCPkts - oldMllOutMCPktsSet1);

}
/**
* @internal prvTgfBasicIpv4McRoutingMllSilentDropTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectSilentDrop         - GT_TRUE: expect silent drop packets - no traffic
*                                      GT_FALSE: expect NO silent drop packets - traffic exists
*                                       None
*/
GT_VOID prvTgfBasicIpv4McRoutingMllSilentDropTrafficGenerate
(
    GT_BOOL     expectSilentDrop
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          silentDropPkts;
    GT_U32                          expectedPackets;
    GT_U32                          expectedPackets1;
    GT_U32                          expectedSilentDropPkts;
    CPSS_INTERFACE_INFO_STC         portInt;

    GT_U32                          oldMllOutMCPktsSet0;
    GT_U32                          oldMllOutMCPktsSet1;
    GT_U32                          mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* enable capture on a MC subscriber port */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
           TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
           "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
           prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS]);

    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    /* set MLL counters mode for MC subscriber 1 */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
    cntIntCfg.devNum           = prvTgfDevNum;
    cntIntCfg.portTrunkCntMode = PRV_TGF_IP_PORT_CNT_MODE_E;
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* set MLL counters mode for MC subscriber 2 */
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");


    /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv4 packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=224.1.1.1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of StartTransmitting: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(100);


       /* disable capture on a MC subscriber port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);


    if(expectSilentDrop==GT_FALSE)
    {
        expectedPackets = 2;
        expectedPackets1= 2;
        expectedSilentDropPkts=0;
    }
    else
    {
        expectedPackets= 0;
        expectedPackets1= 1;
        expectedSilentDropPkts=1;
    }

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS],
            0, expectedPackets,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS],
            0, expectedPackets1,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS],
            0, 0,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    /* get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n",
                         prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

     /* Check L2 MLL silent drop counter is 1 pachet was filtered */
    rc = prvTgfL2MllSilentDropCntGet(prvTgfDevNum, &silentDropPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllSilentDropCntGet");
    PRV_UTF_LOG1_MAC("silentDropPkts = %d\n", silentDropPkts);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedSilentDropPkts, silentDropPkts,
                                 "silentDropPkts[%d] different then expected[%d]",
                                  silentDropPkts, expectedSilentDropPkts);
}

/**
* @internal prvTgfBasicIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* AUTODOC: flush all MC Ipv4 prefix */
    rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush");

    /* AUTODOC: disable MC IPv4 routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable MC IPv4 routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");


    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* invalidate VLAN entry for MC subscriver 1 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_MC_1_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 1");

    /* invalidate VLAN entry for MC subscriver 2 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_MC_2_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 2");

    /* set default MLL counters mode */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));

    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");
}

/**
* @internal prvTgfBasicIpv4McRoutingForDualHwDevConfigurationInit function
* @endinternal
*
* @brief   Set configuration for multi hemisphere devices:
*         - set specific ports numbers
*/
GT_VOID prvTgfBasicIpv4McRoutingForDualHwDevConfigurationInit
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;


    /* save ports array */
    cpssOsMemCpy(prvTgfDefPortsArr, prvTgfPortsArray,
                 sizeof(prvTgfPortsArray[0]) * PRV_TGF_MAX_PORTS_NUM_CNS);

    /* save number of ports array */
    prvTgfDefPortsNum = prvTgfPortsNum;

    /* AUTODOC: config ports - tx port in 1 hemisphere and rest in other */
    rc = prvTgfDefPortsArraySet(testPortsForHwDevArr, PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArraySet: %d", prvTgfDevNum);

    isDualHwDevTest = GT_TRUE;
}
/**
* @internal prvTgfBasicIpv4McRoutingForDualHwDevConfigurationRestore function
* @endinternal
*
* @brief   Restore config for multi hemisphere devices
*/
GT_VOID prvTgfBasicIpv4McRoutingForDualHwDevConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: restore default ports configuration */
    rc = prvTgfDefPortsArraySet(prvTgfDefPortsArr, prvTgfDefPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArraySet: %d", prvTgfDevNum);

    isDualHwDevTest = GT_FALSE;
    prvTgfPacketsCountTxMC1 = 1;
}

/**
* @internal prvTgfBasicIpv4McRoutingRouteConfigurationDifferentPrefixLengthSet function
* @endinternal
*
* @brief   Set Prefix with different length configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingRouteConfigurationDifferentPrefixLengthSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_IPADDR                       ipGrp, ipSrc;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;

    /* AUTODOC: add MC route entry with index 11, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite((prvTgfRouteEntryBaseIndex+1),
                                     1,
                                     protocol,
                                     mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = (prvTgfRouteEntryBaseIndex+1);
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the LPM */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 170;
    ipGrp.arIP[2] = 0;
    ipGrp.arIP[3] = 0;

    ipSrc.arIP[0]=prvTgfPacketIpv4Part.srcAddr[0];
    ipSrc.arIP[1]=prvTgfPacketIpv4Part.srcAddr[1];
    ipSrc.arIP[2]=prvTgfPacketIpv4Part.srcAddr[2];
    ipSrc.arIP[3]=prvTgfPacketIpv4Part.srcAddr[3];

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.170.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);


    /* fill a destination IP address for the LPM */
    ipGrp.arIP[1] = 172;

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.172.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

    /* fill a destination IP address for the LPM */
    ipGrp.arIP[1] = 174;

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.174.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

    /* fill a destination IP address for the LPM */
    ipGrp.arIP[1] = 176;

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.176.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

    /* fill a destination IP address for the LPM */
    ipGrp.arIP[1] = 178;

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.178.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

     /* fill a destination IP address for the LPM */
    ipGrp.arIP[1] = 180;

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.180.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);


     /* fill a destination IP address for the LPM */
    ipGrp.arIP[1] = 182;

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.182.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

    /* fill a destination IP address for the LPM */
    ipGrp.arIP[1] = 184;

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.184.x.x/15, ipSrc=1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 15, ipSrc, 32,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

}
/**
* @internal prvTgfBasicIpv4McRoutingTrafficDifferentPrefixLengthGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv4McRoutingTrafficDifferentPrefixLengthGenerate
(
    GT_VOID
)
{
    TGF_IPV4_ADDR                   dstAddr;
    TGF_MAC_ADDR                    daMac;


    /* update ip prefix in the packet info */

    dstAddr[0]=224;
    dstAddr[1]=184;
    dstAddr[2]=61;
    dstAddr[3]=61;

    daMac[0]=0x01;
    daMac[1]=0x00;
    daMac[2]=0x5E;
    daMac[3]=0x38;/* MAC_DA[23]=0 */
    daMac[4]=0x3D;
    daMac[5]=0x3D;

    cpssOsMemCpy(prvTgfPacketL2Part.daMac, &daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, &dstAddr, sizeof(TGF_IPV4_ADDR));

    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);

    daMac[0]=0x01;
    daMac[1]=0x00;
    daMac[2]=0x5E;
    daMac[3]=0xB8;/* MAC_DA[23]=1 */
    daMac[4]=0x3D;
    daMac[5]=0x3D;

    cpssOsMemCpy(prvTgfPacketL2Part.daMac, &daMac, sizeof(TGF_MAC_ADDR));

    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_FALSE);


}

/**
* @internal prvTgfBasicIpv4McRoutingConfigurationDifferentPrefixLengthRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingConfigurationDifferentPrefixLengthRestore
(
    GT_VOID
)
{
    TGF_IPV4_ADDR   dstAddr;
    TGF_MAC_ADDR    daMac;


    /* AUTODOC: RESTORE CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 1. Restore packet Configuration
     */
    /* update ip prefix in the packet info */

    dstAddr[0]=224;
    dstAddr[1]=1;
    dstAddr[2]=1;
    dstAddr[3]=1;

    daMac[0]=0x01;
    daMac[1]=0x00;
    daMac[2]=0x5E;
    daMac[3]=0x01;
    daMac[4]=0x01;
    daMac[5]=0x01;

    cpssOsMemCpy(prvTgfPacketL2Part.daMac, &daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, &dstAddr, sizeof(TGF_IPV4_ADDR));
}



/**
* @internal prvTgfIpv4McHwAccessAndMallocNumberCalculation function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*
* @param[out] hwWriteNumberPtr         - number of hw write accesses for given ipv4 mc prefix.
* @param[out] mallocNumberPtr          - number of malloc allocations for given ipv4 mc prefix.
*                                       None
*/
static GT_VOID  prvTgfIpv4McHwAccessAndMallocNumberCalculation
(
    IN GT_IPADDR                    ipGrp,
    IN GT_U32                       ipGrpPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC     *mcRouteLttEntryPtr,
    OUT GT_U32                      *hwWriteNumberPtr,
    OUT GT_U32                      *mallocNumberPtr
)
{
    GT_STATUS rc = GT_OK;
    /* enable calculation of hw writes number */
    rc = appDemoHwAccessFailerBind(GT_TRUE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    /* enable calculation of malloc allocations */
    rc = osDbgMallocFailSet(GT_TRUE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);

    /* don't make validation function after prefix insertion */
     prvTgfIpValidityCheckEnable(GT_FALSE);

    /* AUTODOC: add IPv4 MC prefix: */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, ipGrpPrefixLen, ipSrc, ipSrcPrefixLen,
                                   mcRouteLttEntryPtr, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

    prvTgfIpValidityCheckEnable(GT_TRUE);

    /* get hw writes number during add prefix operation */
    rc = appDemoHwAccessCounterGet(hwWriteNumberPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessCounterGet: %d", prvTgfDevNum);
    /* get number of malloc allocations during add prefix operation*/
    rc = osMemGetMallocAllocationCounter(mallocNumberPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osMemGetMallocAllocationCounter: %d", prvTgfDevNum);

    /* prevent counters to work under delete */
    rc = appDemoHwAccessFailerBind(GT_FALSE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    rc = osDbgMallocFailSet(GT_FALSE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);

    /* delete prefix */
    rc =  prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId,0,ipGrp,ipGrpPrefixLen,ipSrc,ipSrcPrefixLen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryDel: %d", prvTgfDevNum);
}


/**
* @internal prvTgfIpv4McAddPrefixAndValidateRollback function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
static GT_VOID prvTgfIpv4McAddPrefixAndValidateRollback
(
    IN GT_IPADDR                    ipGrp,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       srcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC     *mcRouteLttEntryPtr,
    IN GT_U32                       failedSeqNumber,
    IN GT_BOOL                      hwFailerAction
)
{
    GT_STATUS rc = GT_OK;
    if (hwFailerAction == GT_TRUE)
    {
        rc = appDemoHwAccessFailerBind(GT_TRUE,failedSeqNumber);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    }
    else
    {
        rc = osDbgMallocFailSet(GT_TRUE,failedSeqNumber);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);
    }
    /* This operation is going to fail */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 32, ipSrc, srcPrefixLen,
                                   mcRouteLttEntryPtr, GT_TRUE, GT_FALSE);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

    /* deactivate  fail mechanism */
    if (hwFailerAction == GT_TRUE)
    {
        rc = appDemoHwAccessFailerBind(GT_FALSE,0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    }
    else
    {
        rc = osDbgMallocFailSet(GT_FALSE,0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);

    }

    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    /* generate traffic for previous prefix:*/
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);
}


/**
* @internal prvTgfIpv4McRollBackCheck function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
GT_VOID  prvTgfIpv4McRollBackCheck(GT_VOID)
{
    GT_IPADDR ipGrp, ipSrc;
    GT_U32    srcPrefixLength = 0;
    GT_U32    seed = 0;
    GT_U32    maxHwWriteNumber = 0;
    GT_U32    maxMallocNumber = 0;
    GT_U32    failedSeqNumber = 0;
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;

    /* get random seed */
    seed = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(seed);
    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = 19;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the LPM */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 1;
    ipGrp.arIP[2] = cpssOsRand()% 255;
    ipGrp.arIP[3] = cpssOsRand()% 255;

    /* fill a source IP address for the LPM */
    ipSrc.arIP[0] = cpssOsRand()% 255;;
    ipSrc.arIP[1] = cpssOsRand()% 255;
    ipSrc.arIP[2] = cpssOsRand()% 255;
    ipSrc.arIP[3] = cpssOsRand()% 255;

    while(0 == srcPrefixLength) /* the prefix len should not be zero */
    {
        srcPrefixLength = cpssOsRand()% 32;
    }

    /* calculate number of hw write and malloc operations for given ipv4 mc prefix */
     prvTgfIpv4McHwAccessAndMallocNumberCalculation(ipGrp,32,ipSrc,srcPrefixLength,
                                                    &mcRouteLttEntry,
                                                    &maxHwWriteNumber,&maxMallocNumber);

    /* calculate which hw write would fail */
    if( 0 == maxHwWriteNumber)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "zero maxHwWriteNumber, should never happen");
    }
    else
    {
        failedSeqNumber = cpssOsRand()% maxHwWriteNumber;
    }

    if (failedSeqNumber == 0)
    {
        failedSeqNumber++;
    }
    /* add prefix and validate that roolback is OK */
    prvTgfIpv4McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
                                             failedSeqNumber,GT_TRUE);
    /* calculate which malloc would fail */
    if( 0 == maxMallocNumber)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "zero maxMallocNumber, should never happen");
    }
    else
    {
        failedSeqNumber = cpssOsRand()% maxMallocNumber;
    }
    if (failedSeqNumber == 0)
    {
        failedSeqNumber++;
    }
    /* add prefix and validate that roolback is OK */
    prvTgfIpv4McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
                                             failedSeqNumber,GT_FALSE);
    /* fill a destination IP address for the LPM */
    ipGrp.arIP[0] = 227;
    ipGrp.arIP[1] = cpssOsRand()% 255;;
    ipGrp.arIP[2] = cpssOsRand()% 255;
    ipGrp.arIP[3] = cpssOsRand()% 255;

    /* calculate number of hw write and malloc operations for given ipv4 mc prefix */
    prvTgfIpv4McHwAccessAndMallocNumberCalculation(ipGrp,32,ipSrc,srcPrefixLength,
                                                   &mcRouteLttEntry,
                                                   &maxHwWriteNumber,&maxMallocNumber);
    /* calculate which hw write would fail */
    if( 0 == maxHwWriteNumber)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "zero maxHwWriteNumber, should never happen");
    }
    else
    {
        failedSeqNumber = cpssOsRand()% maxHwWriteNumber;
    }
    if (failedSeqNumber == 0)
    {
        failedSeqNumber++;
    }

    /* add prefix and validate that roolback is OK */
    prvTgfIpv4McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
                                             failedSeqNumber,GT_TRUE);

    /* do test with CPU memory allocation problem */
    /* calculate which malloc allocation will fail */
    if( 0 == maxMallocNumber)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "zero maxMallocNumber, should never happen");
    }
    else
    {
        failedSeqNumber = cpssOsRand()% maxMallocNumber;
    }
    if (failedSeqNumber == 0)
    {
        failedSeqNumber++;
    }

    /* add prefix and validate that roolback is OK */
    prvTgfIpv4McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
                                             failedSeqNumber,GT_FALSE);
}


