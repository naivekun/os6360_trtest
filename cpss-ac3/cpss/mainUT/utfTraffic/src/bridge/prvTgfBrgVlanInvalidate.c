/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvTgfBrgVlanInvalidate.c
*
* @brief Check Sip5.20 VLAN entry invalidation. Sip5.20 has no valid bit on
* Egress VLAN - so VLAN entry invalidation may works uncorrectly without
* port members reset.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfHelpers.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfMirror.h>
#include <common/tgfPortGen.h>
#include <common/tgfIpGen.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <bridge/prvTgfBrgVlanInvalidate.h>


/* port indexes in prvTgfPortsArray */
#define  PRV_TGF_SEND_PORT_INDEX_CNS            1
#define  PRV_TGF_NEXTHOP_PORT_INDEX_CNS         2


/* VLAN IDs */
#define  PRV_TGF_SEND_VLANID_CNS                5
#define  PRV_TGF_NEXTHOP_VLANID_CNS             6


/* Index of entry on ARP table */
#define  PRV_TGF_ROUTER_ARP_INDEX_CNS           0
/* VRF ID */
#define  PRV_TGF_VRF_ID_CNS                     1
/* Base route entry index */
#define  PRV_TGF_ROUTE_ENTRY_BASE_INDEX_CNS     1
/* LPM DB ID */
#define  PRV_TGF_LPM_DB_ID_CNS                  0
/* Transmit burst count */
#define  PRV_TGF_BURST_COUNT_CNS                1


/* Destination MAC address */
#define  PRV_TGF_DST_MAC_ADDR_CNS               { 0x00, 0x00, 0x00, 0xAA, 0xBB, 0x03 }
/* Source MAC address */
#define  PRV_TGF_SRC_MAC_ADDR_CNS               { 0x00, 0x00, 0x00, 0xAA, 0xBB, 0x02 }
/* Destination IP address */
#define  PRV_TGF_DST_IP_ADDR_CNS                { 1, 1, 1, 3 }
/* Source IP address */
#define  PRV_TGF_SRC_IP_ADDR_CNS                { 1, 1, 1, 2 }


/* MAC address used on ARP table */
static TGF_MAC_ADDR prvTgfArpMac = PRV_TGF_DST_MAC_ADDR_CNS;


/*-------------------------------- packet -----------------------------------*/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    PRV_TGF_DST_MAC_ADDR_CNS,           /* MAC DA */
    PRV_TGF_SRC_MAC_ADDR_CNS            /* MAC SA */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,    /* etherType        */
    0, 0, PRV_TGF_SEND_VLANID_CNS       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS     /* etherType */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                                  /* version       */
    5,                                  /* headerLen     */
    0,                                  /* typeOfService */
    0x2A,                               /* totalLen      */
    0,                                  /* id            */
    0,                                  /* flags         */
    0,                                  /* offset        */
    0x40,                               /* timeToLive    */
    0x04,                               /* protocol      */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum          */
    PRV_TGF_SRC_IP_ADDR_CNS,            /* srcAddr       */
    PRV_TGF_DST_IP_ADDR_CNS             /* dstAddr       */
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
    sizeof(prvTgfPayloadDataArr),       /* dataLength */
    prvTgfPayloadDataArr                /* dataPtr    */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part        },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart   },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart },
    { TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part      },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart   }
};

/* LENGTH of TAGGED packet */
#define  PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPacketPartArray)

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]),   /* numOfParts */
    prvTgfPacketPartArray                                               /* partsArray */
};
/*----------------------------- end of packet -------------------------------*/


/**
* @internal prvTgfBrgVlanInvalidateConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfBrgVlanInvalidateConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *entryPtr;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U8                                   tagArray[] = { 1 };


    /*------------------------------------------------------------------------*/
    /* 0. Create 2 VLAN entries and add ports to their                        */
    /*------------------------------------------------------------------------*/

    /* AUTODOC: set VLAN entry for port 1, VLAN 5 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray + PRV_TGF_SEND_PORT_INDEX_CNS, NULL, tagArray, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC: set VLAN entry for port 2, VLAN 6 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
            prvTgfPortsArray + PRV_TGF_NEXTHOP_PORT_INDEX_CNS, NULL, tagArray, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");


    /*------------------------------------------------------------------------*/
    /* 1. Enable Routing                                                      */
    /*------------------------------------------------------------------------*/

    /* AUTODOC: enable Unicast IPv4 Routing on port 1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_INDEX_CNS, CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_INDEX_CNS]);

    /* AUTODOC: update VRF ID on VLAN 5 */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, PRV_TGF_VRF_ID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
            prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on VLAN 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
            prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);


    /*------------------------------------------------------------------------*/
    /* 2. Create the Route entry (Next hop) in Route table and                */
    /*    Router ARP Table                                                    */
    /*------------------------------------------------------------------------*/

    /* AUTODOC: write a ARP MAC 00:00:00:AA:AA:03 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ROUTER_ARP_INDEX_CNS, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d",
            prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    entryPtr = &(routeEntriesArray[0]);

    entryPtr->cmd                   = CPSS_PACKET_CMD_ROUTE_E;
    entryPtr->cpuCodeIndex          = 0;
    entryPtr->ingressMirror         = GT_FALSE;
    entryPtr->isTunnelStart         = GT_FALSE;
    entryPtr->nextHopVlanId         = PRV_TGF_NEXTHOP_VLANID_CNS;
    entryPtr->nextHopARPPointer     = PRV_TGF_ROUTER_ARP_INDEX_CNS;
    entryPtr->nextHopInterface.type             = CPSS_INTERFACE_PORT_E;
    entryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    entryPtr->nextHopInterface.devPort.portNum  =
                                      prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_INDEX_CNS];

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 2 */
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_BASE_INDEX_CNS,
            routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d",
            prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac,
            sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType          = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId = PRV_TGF_SEND_VLANID_CNS;
    macEntry.isStatic               = GT_TRUE;
    macEntry.daCommand              = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand              = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                = GT_TRUE;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   =
                                      prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_INDEX_CNS];

    /* AUTODOC: add FDB entry with MAC 00:00:00:AA:BB:03, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d",
            prvTgfDevNum);


    /*------------------------------------------------------------------------*/
    /* 3. Create Virtual Router with default LTT entry                        */
    /*------------------------------------------------------------------------*/

    /* set defIpv4UcRouteEntryInfo */
    cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0, sizeof(defIpv4UcRouteEntryInfo));

    ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;

    ipLttEntryPtr->routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    ipLttEntryPtr->routeEntryBaseIndex      = PRV_TGF_ROUTE_ENTRY_BASE_INDEX_CNS;
    ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
    ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
    ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    /* AUTODOC: disable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_FALSE);

    /* AUTODOC: create Virtual Router with default LTT entry */
    rc = prvTgfIpLpmVirtualRouterAdd(PRV_TGF_LPM_DB_ID_CNS, PRV_TGF_VRF_ID_CNS,
            &defIpv4UcRouteEntryInfo, NULL, NULL, NULL, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd: %d",
            prvTgfDevNum);
}

/**
* @internal prvTgfBrgVlanInvalidateConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfBrgVlanInvalidateConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = { 1 };


    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset ports from VLAN entry 5 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* reset ports from VLAN entry 6 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
             prvTgfDevNum, PRV_TGF_NEXTHOP_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* delete Virtual Router */
    rc = prvTgfIpLpmVirtualRouterDel(PRV_TGF_LPM_DB_ID_CNS, PRV_TGF_VRF_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel");
}

/**
* @internal prvTgfBrgVlanInvalidateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgVlanInvalidateTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      i;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs = {{{0}}};

    for (i = 0; i < 2; i++) {
        if (i == 0)
        {
            PRV_UTF_LOG0_MAC("------------- WITHOUT INVALIDATION --------------\n");
        }
        else
        {
            PRV_UTF_LOG0_MAC("-------------- WITH INVALIDATION ----------------\n");
        }

        /* AUTODOC: reset Eth counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        /* AUTODOC: setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                PRV_TGF_BURST_COUNT_CNS, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

        /* AUTODOC: invalidate VLAN entry (on second iteration) */
        if (i == 1)
        {
            rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate");
        }

        /* AUTODOC: transmit packet to port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_INDEX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

        /* AUTODOC: read port counters on port 2 */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_INDEX_CNS], GT_TRUE,
                &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth");

        /* verify counter */
        if (i == 0)
        {
            /* without invalidation */
            UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_BURST_COUNT_CNS,
                    portCntrs.goodPktsSent.l[0],
                    "Wrong counters value: expected [1], sent [%d]",
                    portCntrs.goodPktsSent.l[0]);
        }
        else
        {
            /* with invalidation */
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                    "Wrong counters value: expected [0], sent [%d]",
                    portCntrs.goodPktsSent.l[0]);
        }
    }
}

