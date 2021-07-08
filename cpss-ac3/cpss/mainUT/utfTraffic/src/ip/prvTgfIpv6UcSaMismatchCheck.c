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
* @file prvTgfIpv6UcSaMismatchCheck.c
*
* @brief Unicast SIP/SA Check
*
* @version   1
********************************************************************************
*/
#include <ip/prvTgfIpv6UcSaMismatchCheck.h>

#include <utf/utfMain.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           (5)

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLANID_CNS        (6)

/* ARP table Index for Source Mac*/
#define PRV_TGF_ARP_SRC_ENTRY_IDX_CNS     (5)

/* ARP table Index for Dest Mac*/
#define PRV_TGF_ARP_DST_ENTRY_IDX_CNS     (6)

/* Route entry SIP/DIP index */
#define PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS   (4)
#define PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS   (3)

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         (0)

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS      (3)

/* Number of ports */
#define PRV_TGF_PORT_COUNT_CNS            (4)

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS             CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS   (1)

/* the LPM DB id for LPM Table */
#define PRV_TGF_LPM_DB_ID_CNS             (0)


/*************************** Saved data ***************************************/
static GT_ETHERADDR arpEntrySrc;
static GT_ETHERADDR arpEntryDst;
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC sipRouteEntry;
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC dipRouteEntry;
static GT_IPV6ADDR sip;
static GT_IPV6ADDR dip;
static GT_BOOL mismatchServiceEnabled;
static GT_BOOL sndPortRoutingEnabled;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},   /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x01}    /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0x1111, 0x1111}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
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
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

static CPSS_PORT_MAC_COUNTER_SET_STC emptyCounters;
static GT_ETHERADDR wrongMac = {{0x00, 0x00, 0x00, 0x00, 0x13, 0x01}};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal convertMac function
* @endinternal
*
* @brief   Converts TGF_MAC_ADDR to GT_ETHERADDR
*
* @param[in] utfMacAddr               - MAC address in UTF format
*
* @param[out] cpssMacAddrPtr           - ptr to MAC address in CPSS format
*                                       None
*/
static GT_VOID convertMac
(
    IN  TGF_MAC_ADDR utfMacAddr,
    OUT GT_ETHERADDR *cpssMacAddrPtr
)
{
    cpssOsMemCpy(cpssMacAddrPtr->arEther, utfMacAddr,
        sizeof(TGF_MAC_ADDR));
}

/**
* @internal convertIpv6 function
* @endinternal
*
* @brief   Converts TGF_IPV6_ADDR to GT_IPV6ADDR
*
* @param[in] utfIpv6Ptr               - ptr to MAC address in UTF format
*
* @param[out] cpssIpv6Ptr              - ptr to MAC address in CPSS format
*                                       None
*/
static GT_VOID convertIpv6
(
    IN  TGF_IPV6_ADDR *utfIpv6Ptr,
    OUT GT_IPV6ADDR   *cpssIpv6Ptr
)
{
    GT_U32 ii;
    GT_U32 idx = 0;
    for(ii = 0; ii < 8; ii++)
    {
        cpssIpv6Ptr->arIP[idx++] = ((*utfIpv6Ptr)[ii] >> 8) & 0xFF;
        cpssIpv6Ptr->arIP[idx++] = ((*utfIpv6Ptr)[ii] >> 0) & 0xFF;
    }
}

/**
* @internal countersEq function
* @endinternal
*
* @brief   Checks if counter sets are equal
*
* @param[in] counters1                - the first set of counters
* @param[in] counters2                - second set of counters
*                                       Equality of two counter sets. GT_TRUE=equal
*/
static GT_BOOL countersEq
(
    IN CPSS_PORT_MAC_COUNTER_SET_STC counters1,
    IN CPSS_PORT_MAC_COUNTER_SET_STC counters2
)
{
    GT_BOOL retval; 

    retval =
        counters1.goodOctetsSent.l[0] == counters2.goodOctetsSent.l[0] &&
        counters1.goodPktsSent.l[0]   == counters2.goodPktsSent.l[0] &&
        counters1.ucPktsSent.l[0]     == counters2.ucPktsSent.l[0] &&
        counters1.brdcPktsSent.l[0]   == counters2.brdcPktsSent.l[0] &&
        counters1.mcPktsSent.l[0]     == counters2.mcPktsSent.l[0] &&
        counters1.goodOctetsRcv.l[0]  == counters2.goodOctetsRcv.l[0] &&
        counters1.goodPktsRcv.l[0]    == counters2.goodPktsRcv.l[0] &&
        counters1.ucPktsRcv.l[0]      == counters2.ucPktsRcv.l[0] &&
        counters1.brdcPktsRcv.l[0]    == counters2.brdcPktsRcv.l[0] &&
        counters1.mcPktsRcv.l[0]      == counters2.mcPktsRcv.l[0];
    return retval;
}

/**
* @internal printCounters function
* @endinternal
*
* @brief   Checks if counters are equal
*
* @param[in] counters                 -  to print
*                                       None
*/
static GT_VOID printCounters
(
    IN CPSS_PORT_MAC_COUNTER_SET_STC counters
)
{
    PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", counters.goodOctetsSent.l[0]);
    PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", counters.goodPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", counters.ucPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", counters.brdcPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", counters.mcPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", counters.goodOctetsRcv.l[0]);
    PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", counters.goodPktsRcv.l[0]);
    PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", counters.ucPktsRcv.l[0]);
    PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", counters.brdcPktsRcv.l[0]);
    PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", counters.mcPktsRcv.l[0]);
    PRV_UTF_LOG0_MAC("\n");

}

/**
* @internal resetCounters function
* @endinternal
*
* @brief   Resets counters for all ports including IP counters
*         Clears Rx Tx tables
*/
static GT_VOID resetCounters
(
    GT_VOID
)
{
    GT_U32 portIter;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS rc;

    rc = prvTgfIpRoutingModeGet(&routingMode);
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* Reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: "
            "port=%d", prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* Reset IP couters and set ROUTE_ENTRY mode for all ports */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d",
                prvTgfPortsArray[portIter]);
        }
    }
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

/**
* @internal prvTgfIpv6UcSaMismatchCheckConfig function
* @endinternal
*
* @brief   AUTODOC: Set Configuration:
*         1. Create two VLANs SENDER and NEXTHOP
*         2. Save unicast routing state for sender port
*         3. Enable unicast routing on sender port
*         4. Assign virtual router to sender VLAN
*         5. Enable unicast routing in sender VLAN
*         6. Create FDB entries for DA/SA MACs
*         7. Save ARP table entries
*         8. Write ARP table entries for DA/SA MACs
*         9. Save route entries
*         10. Set up route entries to route packets between two VLANs
*         11. Add prefix rules for SIP and DIP which use the route entries
*         12. Save SA check service state
*         13. Enable SA check service
*/
GT_VOID prvTgfIpv6UcSaMismatchCheckConfig
(
    GT_VOID
)
{
    GT_ETHERADDR arpMacAddr;
    GT_STATUS rc;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U8 sndVlanTagArray[] = {1, 1, 1};
    GT_U8 hopVlanTagArray[] = {1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: Set up VLANs */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
        prvTgfPortsArray, NULL, sndVlanTagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryWithPortsSet: vlanid=%d",
        PRV_TGF_SEND_VLANID_CNS);
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
        &prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS], NULL,
        hopVlanTagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryWithPortsSet: vlanid=%d",
        PRV_TGF_NEXTHOP_VLANID_CNS);

    /* AUTODOC: Save Unicast IPv6 Routing on sender port enabled state */
    rc = prvTgfIpPortRoutingEnableGet(
        PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, &sndPortRoutingEnabled);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnableGet: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Enable Unicast IPv6 Routing on sender port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnable: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Update VRF Id PRV_TGF_SEND_VLANID_CNS */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: vlanid=%d",
        PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: Enable IPv6 Unicast Routing on sender Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpVlanRoutingEnable: vlanid=%d, enable=%d",
        PRV_TGF_SEND_VLANID_CNS, GT_TRUE);

    /* AUTODOC: Add FDB entry for SA/DA mac*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
        prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = 
        prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    macEntry.isStatic                 = GT_TRUE;
    macEntry.daCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                  = GT_TRUE;
    macEntry.sourceId                 = 0;
    macEntry.userDefined              = 0;
    macEntry.daQosIndex               = 0;
    macEntry.saQosIndex               = 0;
    macEntry.daSecurityLevel          = 0;
    macEntry.saSecurityLevel          = 0;
    macEntry.appSpecificCpuCode       = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite");

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
        prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    macEntry.dstInterface.devPort.portNum   = 
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite");

    /* AUTODOC: Save addresses from ARP table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        &arpEntrySrc);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: "
        "Idx=%d\n", PRV_TGF_ARP_SRC_ENTRY_IDX_CNS);

    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, PRV_TGF_ARP_DST_ENTRY_IDX_CNS,
        &arpEntryDst);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: "
        "Idx=%d\n", PRV_TGF_ARP_DST_ENTRY_IDX_CNS);

    /* AUTODOC: Write to ARP table new addresses */
    convertMac(prvTgfPacketL2Part.saMac, &arpMacAddr); 
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_SRC_ENTRY_IDX_CNS, &arpMacAddr);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        arpMacAddr.arEther[0], arpMacAddr.arEther[1], arpMacAddr.arEther[2],
        arpMacAddr.arEther[3], arpMacAddr.arEther[4], arpMacAddr.arEther[5]);

    convertMac(prvTgfPacketL2Part.daMac, &arpMacAddr); 
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_DST_ENTRY_IDX_CNS, &arpMacAddr);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_DST_ENTRY_IDX_CNS,
        arpMacAddr.arEther[0], arpMacAddr.arEther[1], arpMacAddr.arEther[2],
        arpMacAddr.arEther[3], arpMacAddr.arEther[4], arpMacAddr.arEther[5]);

    /* AUTODOC: Save route entries */
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum,
        PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS, &sipRouteEntry, 1);    
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum,
        PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS, &dipRouteEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS);

    /* AUTODOC: Write new route entries */
    cpssOsMemSet(&routeEntry, 0, sizeof(PRV_TGF_IP_UC_ROUTE_ENTRY_STC));
    routeEntry.cmd = CPSS_PACKET_CMD_ROUTE_E;
    routeEntry.siteId = CPSS_IP_SITE_ID_INTERNAL_E;
    routeEntry.nextHopVlanId = PRV_TGF_SEND_VLANID_CNS;
    routeEntry.nextHopARPPointer = PRV_TGF_ARP_SRC_ENTRY_IDX_CNS;
    routeEntry.nextHopInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS,
        &routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);

    routeEntry.nextHopVlanId = PRV_TGF_NEXTHOP_VLANID_CNS;
    routeEntry.nextHopARPPointer = PRV_TGF_ARP_DST_ENTRY_IDX_CNS;
    routeEntry.nextHopInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS,
        &routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS);

    /* AUTODOC: Write new prefix rules */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    convertIpv6(&prvTgfPacketIpv6Part.srcAddr, &sip);
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 1; 
    nextHopInfo.ipLttEntry.routeEntryBaseIndex =
        PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(PRV_TGF_LPM_DB_ID_CNS, 0, sip, 128, 
        &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd");

    convertIpv6(&prvTgfPacketIpv6Part.dstAddr, &dip);
    nextHopInfo.ipLttEntry.routeEntryBaseIndex =
        PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS;
    rc = prvTgfIpLpmIpv6UcPrefixAdd(PRV_TGF_LPM_DB_ID_CNS, 0, dip, 128, 
        &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd");

    /* AUTODOC: Save the unicast SIP/SA check state */
    rc = prvTgfIpBridgeServiceEnableGet(prvTgfDevNum,
        PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E,
        &mismatchServiceEnabled);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpBridgeServiceEnableGet: Service=%d, Mode=%d\n",
        PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E);

    /* AUTODOC: Enable SIP/SA check */
    prvTgfIpBridgeServiceEnable(prvTgfDevNum,
        PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E,
        GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "prvTgfIpBridgeServiceEnable: Service=%d, Mode=%d, Enable=%d\n",
        PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E,
        GT_TRUE);

}

/**
* @internal prvTgfIpv6UcSaMismatchCheckGenerate function
* @endinternal
*
* @brief   AUTODOC: Perform the test:
*         SA check enabled, ARP entry correct   - packet passed
*         SA check enabled, ARP entry incorrect  - packet dropped
*         SA check disabled, ARP entry correct  - packet passed
*         SA check disabled, ARP entry incorrect - packet passed
*/
GT_VOID prvTgfIpv6UcSaMismatchCheckGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   packetArrivedCntrs;
    GT_ETHERADDR                    arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;

    /* Reset counters */
    resetCounters();

    /* AUTODOC: Form packet and enable capturing */
    /* Setup nexthop portInterface for capturing */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = 
        prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];

    /* Enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
        TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "tgfTrafficGeneratorPortTxEthCaptureSet: port=%d\n",
         prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS]);

    /* Number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) /
        sizeof(prvTgfPacketPartArray[0]);

    /* Calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet");

    /* Build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo,
        PRV_TGF_BURST_COUNT_DEFAULT_CNS, 0,
        NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetup");

    /* Setup counters for arrived packet */
    packetArrivedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) *
        PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.goodPktsSent.l[0]   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.ucPktsSent.l[0]     = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.brdcPktsSent.l[0]   = 0;
    packetArrivedCntrs.mcPktsSent.l[0]     = 0;
    packetArrivedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) *
        PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.goodPktsRcv.l[0]    = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.ucPktsRcv.l[0]      = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.brdcPktsRcv.l[0]    = 0;
    packetArrivedCntrs.mcPktsRcv.l[0]      = 0;

    /* AUTODOC: Case with SA check enabled and correct ARP entry */
    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
        case PRV_TGF_NEXTHOP_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

    resetCounters();

    /* AUTODOC: Case with SA check enabled and incorrect ARP entry */
    /* Set wrong ARP entry */
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_SRC_ENTRY_IDX_CNS, &wrongMac);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        wrongMac.arEther[0], wrongMac.arEther[1], wrongMac.arEther[2],
        wrongMac.arEther[3], wrongMac.arEther[4], wrongMac.arEther[5]);

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        /* Packet is Hard dropped */
        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

    resetCounters();

    /* AUTODOC: Case with SA check disabled and incorrect ARP entry */
    /* Disable check SA in prefix rules */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 1; 
    nextHopInfo.ipLttEntry.routeEntryBaseIndex =
        PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(PRV_TGF_LPM_DB_ID_CNS, 0, sip, 128, 
        &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        /* Packet must not be dropped */
        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
        case PRV_TGF_NEXTHOP_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

    resetCounters();

    /* AUTODOC: Case with SA check disabled and correct ARP entry */
    /* Set back to ARP correct SA and see packet passes */
    convertMac(prvTgfPacketL2Part.saMac, &arpMacAddr); 
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_SRC_ENTRY_IDX_CNS, &arpMacAddr);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        arpMacAddr.arEther[0], arpMacAddr.arEther[1], arpMacAddr.arEther[2],
        arpMacAddr.arEther[3], arpMacAddr.arEther[4], arpMacAddr.arEther[5]);

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        /* Packet must not be dropped */
        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
        case PRV_TGF_NEXTHOP_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

    resetCounters();

    /* AUTODOC: Disable capture on nexthop port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
        TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "tgfTrafficGeneratorPortTxEthCaptureSet: port=%d\n",
        prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS]);

}

/**
* @internal prvTgfIpv6UcSaMismatchCheckRestore function
* @endinternal
*
* @brief   AUTODOC: Restore previous configuration
*         1. Restore SA check state
*         2. Delete prefix rules
*         3. Restore route entries
*         4. Restore ARP entries
*         5. Flush FDB
*         6. Disable routing in sender VLAN
*         7. Restore routing enabled state for sender port
*         8. Invalidate VLANs
*/
GT_VOID prvTgfIpv6UcSaMismatchCheckRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Restore the unicast SIP/SA check state */
    rc = prvTgfIpBridgeServiceEnable(prvTgfDevNum,
        PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E,
        mismatchServiceEnabled);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "prvTgfIpBridgeServiceEnableGet: Service=%d, Mode=%d, Enable=%d\n",
        PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E,
        mismatchServiceEnabled);

    /* AUTODOC: Delete prefix rules */
    rc = prvTgfIpLpmIpv6UcPrefixDel(PRV_TGF_LPM_DB_ID_CNS, 0, sip, 128);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel");
    rc = prvTgfIpLpmIpv6UcPrefixDel(PRV_TGF_LPM_DB_ID_CNS, 0, dip, 128);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel");

    /* AUTODOC: Restore route entries */
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS,
        &sipRouteEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS,
        &dipRouteEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS);

    /* AUTODOC: Restore entries in ARP table */
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        &arpEntrySrc);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        arpEntrySrc.arEther[0], arpEntrySrc.arEther[1], arpEntrySrc.arEther[2],
        arpEntrySrc.arEther[3], arpEntrySrc.arEther[4], arpEntrySrc.arEther[5]);

    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_DST_ENTRY_IDX_CNS,
        &arpEntryDst);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        arpEntryDst.arEther[0], arpEntryDst.arEther[1], arpEntryDst.arEther[2],
        arpEntryDst.arEther[3], arpEntryDst.arEther[4], arpEntryDst.arEther[5]);

    /* AUTODOC: Flush FDB entries added */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitting: "
        "includeStatic=%d\n", GT_TRUE);

    /* AUTODOC: Disable UC routing on sender VLAN */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpVlanRoutingEnable: vlanid=%d, enable=%d",
        PRV_TGF_SEND_VLANID_CNS, GT_FALSE);

    /* AUTODOC: Restore routing enabled state on sender port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, sndPortRoutingEnabled);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnable: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Invalidate VLANs */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_SEND_VLANID_CNS);
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_NEXTHOP_VLANID_CNS);

}


