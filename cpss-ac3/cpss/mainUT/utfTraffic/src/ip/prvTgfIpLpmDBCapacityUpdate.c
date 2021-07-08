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
* @file prvTgfIpLpmDBCapacityUpdate.c
*
* @brief Basic IPV4 UC ECMP Routing
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
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpLpmDBCapacityUpdate.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* port number to send traffic to */
#define PRV_TGF_NEXTHOPE_1_PORT_IDX_CNS   2

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS   3

/* default number of packets to send */
static GT_U32        prvTgfBurstCount          = 1;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* Base Indexes for the Route */
static GT_U32        prvTgfRouteEntryBaseIndex = 1;

/* Base Indexes for the ARP */
static GT_U8         prvTgfArpEntryBaseIndex   = 1;

/* Router ARP Table Configuration */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x11, 0x22, 0x00};

/* IPv4 prefix */
static TGF_IPV4_ADDR prvTgfIpv4Preffix    = {1, 1, 1, 3};


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
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
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

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
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC capcityCfg;
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmDBCapacityUpdateBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: create VLANs 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLANs 5 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* create a static macEntry in SEND VLAN with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum =
                                    prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    macEntry.isStatic                     = GT_TRUE;
    macEntry.daRoute                      = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfBrgFdbMacEntrySet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfIpLpmDBCapacityUpdateLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfIpLpmDBCapacityUpdateLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write a ARP MAC 00:00:00:11:22:00 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfArpEntryBaseIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* setup default values for the route entries */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd               = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId     = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum =
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

    /* setup the route entries by incremental values */
    regularEntryPtr->countSet          = CPSS_IP_CNT_SET0_E;
    regularEntryPtr->nextHopARPPointer = prvTgfArpEntryBaseIndex;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex,
                                     routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: rewrite UC route entry for default prefix - nexthop VLAN 6, port 2 */
    regularEntryPtr->nextHopInterface.devPort.portNum =
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_1_PORT_IDX_CNS];
    rc = prvTgfIpUcRouteEntriesWrite(0, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex,
                                    routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS],
                                 regularEntryPtr->nextHopInterface.devPort.portNum,
                                 "prvTgfIpUcRouteEntriesRead: wrong port number");

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    cpssOsMemCpy(ipAddr.arIP, prvTgfIpv4Preffix, sizeof(ipAddr.arIP));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpLpmDBCapacityUpdatePbrRouteConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
static GT_VOID prvTgfIpLpmDBCapacityUpdatePbrRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    /*GT_ETHERADDR                            arpMacAddr;*/
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");


    /* -------------------------------------------------------------------------
     * 1. PCL Config
     */

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,
        0, /* pclId */
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = CPSS_NET_CONTROL_E;
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = PRV_TGF_NEXTHOPE_VLANID_CNS;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = prvTgfArpEntryBaseIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum =
                                                    prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfIpv4Preffix, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_TRUE,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpLpmDBCapacityUpdateRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateRouteConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            prvTgfIpLpmDBCapacityUpdatePbrRouteConfigurationSet();
            break;
        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            /*PRV_UTF_LOG0_MAC("==== LTT mode does not supported ====\n");*/
            prvTgfIpLpmDBCapacityUpdateLttRouteConfigurationSet();
            break;
        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:
            break;
    }
}

/**
* @internal prvTgfIpLpmDBCapacityUpdateRouteConfigurationUpdate function
* @endinternal
*
* @brief   Update Route Configuration
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateRouteConfigurationUpdate
(
    GT_VOID
)
{
    GT_STATUS                                  rc = GT_OK;
    GT_BOOL                                    partitionEnable = GT_FALSE;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;

    /* AUTODOC: UPDATE CONFIGURATION: */

    /* clear values */
    indexesRange.firstIndex = 0;
    indexesRange.lastIndex  = 0;
    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = 0;
    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 0;
    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = 0;

    cpssOsBzero((GT_CHAR*) &prvTgfRestoreCfg, sizeof(prvTgfRestoreCfg));

    /* get IpLpmDBCapacity */
    rc = prvTgfIpLpmDBCapacityGet(prvTgfLpmDBId,
                               &partitionEnable,
                               &(prvTgfRestoreCfg.indexesRange),
                               &(prvTgfRestoreCfg.capcityCfg));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDBCapacityGet");

    PRV_UTF_LOG0_MAC("======= CapacityGet before CapacityUpdate =======\n");
    PRV_UTF_LOG1_MAC("   partitionEnable = %d\n", partitionEnable);
    PRV_UTF_LOG1_MAC("   indexesRange.firstIndex = %d\n", prvTgfRestoreCfg.indexesRange.firstIndex);
    PRV_UTF_LOG1_MAC("   indexesRange.lastIndex = %d\n", prvTgfRestoreCfg.indexesRange.lastIndex);
    PRV_UTF_LOG1_MAC("   numOfIpv4Prefixes = %d\n",
            prvTgfRestoreCfg.capcityCfg.numOfIpv4Prefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv4McSourcePrefixes = %d\n",
            prvTgfRestoreCfg.capcityCfg.numOfIpv4McSourcePrefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv6Prefixes = %d\n",
            prvTgfRestoreCfg.capcityCfg.numOfIpv6Prefixes);

    /* indexesRange is relevant only fo partitionEnable == GT_TRUE */
    if(GT_FALSE == partitionEnable)
    {
        indexesRange.firstIndex = 0;
    }
    PRV_UTF_LOG1_MAC("   indexesRange.firstIndex = %d\n", indexesRange.firstIndex);
        indexesRange.firstIndex = 0;

    indexesRange.lastIndex = indexesRange.firstIndex + 9;
    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = 4;
    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 4;
    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = 4;

    /* AUTODOC: update LpmDB capacity to: */
    /* AUTODOC:   indexesRange [0 - 9] */
    /* AUTODOC:   numOfIpv4Prefixes=4 */
    /* AUTODOC:   numOfIpv4McSourcePrefixes=4 */
    /* AUTODOC:   numOfIpv6Prefixes=4 */
    rc = prvTgfIpLpmDBCapacityUpdate(prvTgfLpmDBId,
                                  &indexesRange,
                                  &tcamLpmManagerCapcityCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDBCapacityUpdate");

    /* clear values */
    indexesRange.firstIndex = 0;
    indexesRange.lastIndex  = 0;
    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = 0;
    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 0;
    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = 0;

    /* AUTODOC: verify LpmDB capacity after CapacityUpdate */
    rc = prvTgfIpLpmDBCapacityGet(prvTgfLpmDBId,
                               &partitionEnable,
                               &indexesRange,
                               &tcamLpmManagerCapcityCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDBCapacityGet");

    PRV_UTF_LOG0_MAC("======= CapacityGet after CapacityUpdate =======\n");
    PRV_UTF_LOG1_MAC("   partitionEnable = %d\n", partitionEnable);
    PRV_UTF_LOG1_MAC("   indexesRange.firstIndex = %d\n", indexesRange.firstIndex);
    PRV_UTF_LOG1_MAC("   indexesRange.lastIndex = %d\n", indexesRange.lastIndex);
    PRV_UTF_LOG1_MAC("   numOfIpv4Prefixes = %d\n",
            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv4McSourcePrefixes = %d\n",
            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv6Prefixes = %d\n\n",
            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes);
}

/**
* @internal prvTgfIpLpmDBCapacityUpdateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateTrafficGenerate
(
    GT_BOOL isPacketMatched
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U8                           expPktsRcv;
    GT_U8                           expPktsSent;
    GT_U8                           expEgressPortIdx = PRV_TGF_SEND_PORT_IDX_CNS;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* reset IP couters and set ROUTE_ENTRY mode */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* update the packet dip */
    if (isPacketMatched)
    {
        expEgressPortIdx = PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS;
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr,
                     prvTgfIpv4Preffix,
                     sizeof(prvTgfPacketIpv4Part.dstAddr));
    }

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
    /* AUTODOC:   VID=5, srcIP=2.2.2.4, dstIP=1.1.1.3 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[expEgressPortIdx],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: verify routed packet on port 3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        expPktsRcv = expPktsSent = (GT_U8) (portIter == PRV_TGF_SEND_PORT_IDX_CNS ||
                               portIter == expEgressPortIdx);

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[portIter], expPktsRcv, expPktsSent,
                prvTgfPacketInfo.totalLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[expEgressPortIdx];

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_EXPECTED_NO_MATCH_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: verify packet has MAC DA as ARP MAC */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters %d:\n", portIter);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfIpLpmDBCapacityUpdateConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC  routeEntriesArray[1];

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore route entry index 1*/
     cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
     routeEntriesArray[0].cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

     rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex,routeEntriesArray, 1);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore default LpmDB Capacity */
    rc = prvTgfIpLpmDBCapacityUpdate(prvTgfLpmDBId,
                                     &(prvTgfRestoreCfg.indexesRange),
                                     &(prvTgfRestoreCfg.capcityCfg));
    if (GT_OK != rc)
    {
        /* when running the tests with 'stop on fail' (<fContinue> = false), we still need to do all
           the next cleanups (after this line) .*/
        /* --> line removed : --> UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDBCapacityUpdate");*/
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBCapacityUpdate FAILED, rc = [%d]", rc);
        /*log the failure into the DB*/
        rc = utfEqualVerify(GT_OK, rc, __LINE__, __FILE__);
        if(GT_OK != rc)
        {
            /* do nothing since we still need to do all the next cleanups*/
        }
    }

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfIpv4Preffix, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {
        /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
        rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable Trunk Hash Ip Mode */
    rc = prvTgfTrunkHashIpModeSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashIpModeSet: %d", prvTgfDevNum);


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

    /* invalidate NEXTHOPE VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");
}


