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
* @file prvTgfTunnelTermPbrDualLookup.c
*
* @brief Tunnel: Test Dual Lookup Policy Based Routing Pointer
*
* @version   2
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
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermPbrDualLookup.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS            6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* nextHop port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS     3

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32 prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32  prvTgfRouteEntryBaseIndex = 8;

/* the leaf index in the RAM PBR section- range of PBR is 0 ...8K-1*/
static GT_U32   prvTgfPbrLeafIndexArray_dev_trafficExpected[5] = {0,(_2K-1),_2K, _4K,(_8K-1)};
static GT_U32   prvTgfPbrLeafIndexArray_dev_noTrafficExpected[5] = {1,(_2K-2),_2K+1, _4K+1,(_8K-2)};

static GT_U32   prvTgfPbrLeafIndexArray_ac3x_trafficExpected[5] = {0,(_2K-1),_2K, _4K,(6655)};
static GT_U32   prvTgfPbrLeafIndexArray_ac3x_noTrafficExpected[5] = {1,(_2K-2),_2K+1, _4K+1,(6654)};

static GT_U32   *prvTgfPbrLeafIndexArray_trafficExpected;
static GT_U32   *prvTgfPbrLeafIndexArray_noTrafficExpected;

/* default LPM DB Id */
static GT_U32   prvTgfLpmDBId   = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* the TTI Rule indexes */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 ,3)
#define PRV_TGF_TTI1_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1 ,0)


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    0x5EA0,             /* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* packet's IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OverIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x73CD,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
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
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OverIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PACKET Ipv4 to send */
static TGF_PACKET_STC prvTgfPacketIpv4Info = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static PRV_TGF_TTI_ACTION_2_STC ttiAction = {0};
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;

static PRV_TGF_TTI_ACTION_2_STC ttiActionGet = {0};
static PRV_TGF_TTI_RULE_UNT     ttiPatternGet;
static PRV_TGF_TTI_RULE_UNT     ttiMaskGet;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermPbrDualLookupBridgeConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/

static GT_VOID prvTgfTunnelTermPbrDualLookupBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc         = GT_FAIL;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermPbrDualLookupRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] pbrLeafArrayIndex        - the index we are checking form the array
*                                       None
*/
static GT_VOID prvTgfTunnelTermPbrDualLookupRouteConfigurationSet
(
    GT_U32 prvUtfVrfId,
    GT_U32 pbrLeafArrayIndex
)
{
    GT_STATUS                               rc = GT_FAIL;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_LPM_LEAF_ENTRY_STC              leafEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths = 1;
    GT_U32                                  ii;

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");


    /* AUTODOC: Enable Routing */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* AUTODOC: Create the Route entry (Next hop) in Route table and Router ARP Table */

    /* AUTODOC: write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    regularEntryPtr = &routeEntriesArray[0];

    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_EGR_VLANID_CNS;
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;

    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC:  Create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* clear IPv4 entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* clear IPv6 entry */
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;
            }
            if (ii == 1)
            {
                ipLttEntryPtr = &defIpv6UcRouteEntryInfo.ipLttEntry;
            }
            /* set defUcRouteEntryInfo */
            ipLttEntryPtr->routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipLttEntryPtr->numOfPaths               = numOfPaths;
            ipLttEntryPtr->routeEntryBaseIndex      = prvTgfRouteEntryBaseIndex;
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        }

        /* AUTODOC: create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    cpssOsMemSet(&leafEntry, 0, sizeof(PRV_TGF_LPM_LEAF_ENTRY_STC));
    /* AUTODOC: write a leaf entry to the PBR area in RAM */
    leafEntry.entryType                = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    leafEntry.index                    = prvTgfRouteEntryBaseIndex;
    leafEntry.ucRPFCheckEnable         = GT_FALSE;
    leafEntry.sipSaCheckMismatchEnable = GT_FALSE;
    leafEntry.ipv6MCGroupScopeLevel    = 0;
    leafEntry.priority                 = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E;

    /* AUTODOC: Write a PBR leaf */
    PRV_UTF_LOG1_MAC("prvTgfLpmLeafEntryWrite : (prvTgfPbrLeafIndex[%d) =======\n" ,
                        prvTgfPbrLeafIndexArray_trafficExpected[pbrLeafArrayIndex]);
    rc = prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndexArray_trafficExpected[pbrLeafArrayIndex], &leafEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLeafEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermPbrDualLookupTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] pbrLeafArrayIndex        - the index we are checking form the array
*                                       None
*/
static GT_VOID prvTgfTunnelTermPbrDualLookupTtiConfigurationSet
(
    GT_U32  pbrLeafArrayIndex
)
{
    GT_STATUS                rc = GT_FAIL;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable the TTI lookup for IPv4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action to match all packets */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
        cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    ttiAction.redirectCommand                 = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.routerLttPtr                    = prvTgfPbrLeafIndexArray_noTrafficExpected[pbrLeafArrayIndex];

    ttiAction.tunnelTerminate                 = GT_TRUE;
    ttiAction.ttPassengerPacketType           = PRV_TGF_TTI_PASSENGER_IPV4_E;

    ttiAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiAction.tag0VlanPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    ttiAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;

    ttiAction.continueToNextTtiLookup         = GT_TRUE;

    /* AUTODOC: set TTI0 rule with redirect command NO_REDIRECT and invalid routerLttPtr + 1 */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    ttiAction.redirectCommand                 = PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
    ttiAction.routerLttPtr                    = prvTgfPbrLeafIndexArray_trafficExpected[pbrLeafArrayIndex];

    /* AUTODOC: set TTI1 rule with redirect command REDIRECT_TO_ROUTER and valid routerLttPtr */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI1_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRule2Get(prvTgfDevNum,PRV_TGF_TTI1_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPatternGet, &ttiMaskGet, &ttiActionGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Get: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(ttiAction.routerLttPtr, ttiActionGet.routerLttPtr, "prvTgfTtiRule2Get: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermPbrDualLookupTtiRedirectCmd function
* @endinternal
*
* @brief   Change TTI redirect command, lttRouterPtr for given TTI rule index
*
* @param[in] isValidLttPtr            -
*                                      GT_TRUE: set routerLttPtr to prvTgfPbrLeafIndex
*                                      GT_FALSE: set routerLttPtr to invalid (prvTgfPbrLeafIndex + 1)
* @param[in] ruleIndex                - index of TTI rule
* @param[in] pbrLeafArrayIndex        - the index we are checking form the array
*                                       None
*/
static GT_VOID prvTgfTunnelTermPbrDualLookupTtiRedirectCmd
(
    GT_BOOL isValidLttPtr,
    GT_U32  ruleIndex,
    GT_U32  pbrLeafArrayIndex
)
{
    GT_STATUS rc = GT_FAIL;

    ttiAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
    ttiAction.routerLttPtr = isValidLttPtr ? prvTgfPbrLeafIndexArray_trafficExpected[pbrLeafArrayIndex] : prvTgfPbrLeafIndexArray_noTrafficExpected[pbrLeafArrayIndex];

    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRule2Get(prvTgfDevNum,ruleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPatternGet, &ttiMaskGet, &ttiActionGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Get: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(ttiAction.routerLttPtr, ttiActionGet.routerLttPtr, "prvTgfTtiRule2Get: %d", prvTgfDevNum);

}

/**
* @internal prvTgfTunnelTermPbrDualLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect traffic
*                                      GT_FALSE: expect no traffic
*                                       None
*/
static GT_VOID prvTgfTunnelTermPbrDualLookupTrafficGenerate
(
    GT_BOOL expectTraffic
)
{
    GT_STATUS rc               = GT_FAIL;
    GT_U32    portIter         = 0;
    GT_U32    prvTgfBurstCount = 1;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* AUTODOC: GENERATE TRAFFIC */

    /*  Reset all Ethernet port's counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);


    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* reset IP couters and set ROUTE_ENTRY mode */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 1 ethernet packet from port 0 with: VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfPacketIpv4Info, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

    /* AUTODOC: verify to get packet on port 3 with: */
    UTF_VERIFY_EQUAL3_STRING_MAC(portCntrs.goodPktsRcv.l[0], expectTraffic ? prvTgfBurstCount : 0,
        "got wrong counters: port[%d], expected [%d], received[%d]\n",
         prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], prvTgfBurstCount, portCntrs.goodPktsRcv.l[0]);
}


/**
* @internal prvTgfTunnelTermPbrDualLookupConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
static GT_VOID prvTgfTunnelTermPbrDualLookupConfigurationRestore
(
    GT_U32      prvUtfVrfId
)
{
    GT_STATUS   rc        = GT_FAIL;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* AUTODOC: invalidate TTI rule 0 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for IPv4 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterDel: %d",
                                     prvTgfDevNum);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}


/**
* @internal prvTgfTunnelTermPbrDualLookup function
* @endinternal
*
* @brief   Test main func
*/
GT_VOID prvTgfTunnelTermPbrDualLookup
(
    GT_VOID
)
{

    GT_U32      prvUtfVrfId = 0;
    GT_U32      pbrLeafArrayIndex;

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
    {
        prvTgfPbrLeafIndexArray_trafficExpected = prvTgfPbrLeafIndexArray_ac3x_trafficExpected;
        prvTgfPbrLeafIndexArray_noTrafficExpected = prvTgfPbrLeafIndexArray_ac3x_noTrafficExpected;
    }
    else
    {
        prvTgfPbrLeafIndexArray_trafficExpected = prvTgfPbrLeafIndexArray_dev_trafficExpected;
        prvTgfPbrLeafIndexArray_noTrafficExpected = prvTgfPbrLeafIndexArray_dev_noTrafficExpected;
    }

    /* AUTODOC: build ethernet packet with vid 5 */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    prvTgfTunnelTermPbrDualLookupBridgeConfigSet();

    for(pbrLeafArrayIndex=0;pbrLeafArrayIndex<5;pbrLeafArrayIndex++)
    {
        /*------------------------------------------------------------*/
        /* AUTODOC: configure IP routing, LPM and LTT entries */
        prvTgfTunnelTermPbrDualLookupRouteConfigurationSet(prvUtfVrfId,pbrLeafArrayIndex);

        /*
            AUTODOC: set TTI0 rule with redirect command NO_REDIRECT and valid routerLttPtr
            AUTODOC: set TTI1 rule with redirect command REDIRECT_TO_ROUTER and valid routerLttPtr
            AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and traffic on port
        */
        prvTgfTunnelTermPbrDualLookupTtiConfigurationSet(pbrLeafArrayIndex);
        prvTgfTunnelTermPbrDualLookupTrafficGenerate(GT_TRUE);
        /*------------------------------------------------------------*/

        /*
            AUTODOC: set TTI1 routerLttPtr to invalid (routerLttPtr + 1)
            AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and no traffic on port
        */
        prvTgfTunnelTermPbrDualLookupTtiRedirectCmd(GT_FALSE, PRV_TGF_TTI1_INDEX_CNS,pbrLeafArrayIndex);
        prvTgfTunnelTermPbrDualLookupTrafficGenerate(GT_FALSE);
        /*------------------------------------------------------------*/

        /*
            AUTODOC: set TTI0 redirect Cmd to REDIRECT_TO_ROUTER and valid routerLttPtr
            AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and no traffic on port
        */
        prvTgfTunnelTermPbrDualLookupTtiRedirectCmd(GT_TRUE, PRV_TGF_TTI0_INDEX_CNS,pbrLeafArrayIndex);
        prvTgfTunnelTermPbrDualLookupTrafficGenerate(GT_FALSE);
        /*------------------------------------------------------------*/

        /*
            AUTODOC: set TTI0 redirect Cmd to REDIRECT_TO_ROUTER invalid routerLttPtr + 1
            AUTODOC: set TTI1 redirect Cmd to REDIRECT_TO_ROUTER and valid routerLttPtr
            AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and receive traffic on port
        */
        prvTgfTunnelTermPbrDualLookupTtiRedirectCmd(GT_FALSE, PRV_TGF_TTI0_INDEX_CNS,pbrLeafArrayIndex);
        prvTgfTunnelTermPbrDualLookupTtiRedirectCmd(GT_TRUE, PRV_TGF_TTI1_INDEX_CNS,pbrLeafArrayIndex);
        prvTgfTunnelTermPbrDualLookupTrafficGenerate(GT_TRUE);

        /* AUTODOC: test configuration restore */
        prvTgfTunnelTermPbrDualLookupConfigurationRestore(prvUtfVrfId);
    }
}



