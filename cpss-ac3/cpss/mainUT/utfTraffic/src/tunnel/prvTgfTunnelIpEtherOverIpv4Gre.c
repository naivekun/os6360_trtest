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
* @file prvTgfTunnelIpEtherOverIpv4Gre.c
*
* @brief Packet received as IP-Ethernet-over-IPv4-GRE tunnel
* Packet is TT
* Passenger Ethernet packet MAC DA is Router MAC address
* Triggers IPv4 UC router, Nexthop entry has TS Pointer, and TS Type is IP
* TS entry is IPv4-GRE
* Packet is egressed as IPv4-over-GRE-IPv4
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfDitGen.h>
#include <common/tgfPacketGen.h>
#include <tunnel/prvTgfTunnelIpEtherOverIpv4Gre.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           3

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* Ethernet over IPv4 */
static TGF_PACKET_STC prvTgfEthernetOverIpv4PacketInfo;

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x66

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 5;

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32        prvTgfRouterArpTunnelStartLineIndex = 8;

/* type of the tunnel */
static CPSS_TUNNEL_TYPE_ENT prvTgfTunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x88, 0x77, 0x11, 0x11, 0x55, 0x66};

/* Tunnel destination IP */
static TGF_IPV4_ADDR prvTgfTunnelDstIp = {1, 1, 1, 2};

/* Tunnel source IP */
static TGF_IPV4_ADDR prvTgfTunnelSrcIp = {2, 2, 3, 3};

/* Passenger destination IP */
static TGF_IPV4_ADDR prvTgfPassengerDstIp = {1, 1, 1, 3};

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* Router Mac Sa index for tunnel start */
static GT_U32  routerMacSaIndex = 53;

/* IPv4 TS : L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketTunnelStartL2Part = {
    {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee},                /* daMac */
    {0x00, 0x99, 0x88, 0x77, 0x66, 0x55}                 /* saMac */
};

/* Tunnel TTL */
static GT_U32 prvTgfTunnelTtl = 33;

/* default value for ipv4 total length field */
static GT_U16 prvTgfTunnelTotalLength = 0x5c;

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT                    macMode;
    GT_U32                                      ethType1;
    GT_U32                                      routerMacSaIndex;
    GT_ETHERADDR                                macSaAddr;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS] = {0};
    TGF_PACKET_L2_STC *l2PartPtr;

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged\tagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, tagArray, PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* use L2 part of passenger */
    l2PartPtr = prvTgfEthernetOverIpv4PacketInfo.partsArray[9].partPtr;

    /* AUTODOC: add FDB entry with MAC 00:09:0A:0B:0C:22, VLAN 5, vid 6 , daRoute = GT_TRUE */
    rc = prvTgfBrgDefFdbMacEntryOnVidSet(l2PartPtr->daMac,
                                         PRV_TGF_VLANID_5_CNS, PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreLttRouteConfigurationSet
(
    GT_U32      prvUtfVrfId
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths = 0;
    GT_U32                                  ii;

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_VLANID_5_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_VLANID_5_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_5_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_TRUE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_VLANID_5_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = 0;
    regularEntryPtr->nextHopTunnelPointer       = prvTgfRouterArpTunnelStartLineIndex;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 2 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
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

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPassengerDstIp, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                    32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreIpv4TunnelStartConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreIpv4TunnelStartConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;
    GT_ETHERADDR                    macSaAddr;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E; /* eliminate Coverity warning */
    cpssOsMemSet(&tunnelEntry,0,sizeof(tunnelEntry));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId           = PRV_TGF_VLANID_5_CNS;
    tunnelEntry.ipv4Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.up               = 0;
    tunnelEntry.ipv4Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscp             = 0;
    tunnelEntry.ipv4Cfg.dontFragmentFlag = GT_FALSE;
    tunnelEntry.ipv4Cfg.ttl              = prvTgfTunnelTtl;
    tunnelEntry.ipv4Cfg.autoTunnel       = GT_FALSE;
    tunnelEntry.ipv4Cfg.autoTunnelOffset = 0;
    tunnelEntry.ipv4Cfg.ethType          = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.cfi              = 0;
    tunnelEntry.ipv4Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelEntry.ipv4Cfg.greFlagsAndVersion = 0;
    /* AUTODOC: in bobcat2 the greProtocolForEthernet is set implicitly to 0x0800 by the device , for ipv4 passenger over the GRE */
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: but in bobk we must set the IPV4 Ethertype , for ipv4 passenger over the GRE. */
        /* AUTODOC: as the device take it from the TS entry (regardless to passenger type)*/
        tunnelEntry.ipv4Cfg.greProtocolForEthernet = TGF_ETHERTYPE_0800_IPV4_TAG_CNS;
    }

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfPacketTunnelStartL2Part.daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfTunnelDstIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType=IPV4_OVER_IPV4 */
    /* AUTODOC:   vlanId=6, ttl=33, ethType=TUNNEL_GRE0_ETHER_TYPE */
    /* AUTODOC:   DA=88:77:11:11:55:66, srcIp=2.2.3.3, dstIp=1.1.1.2 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex, prvTgfTunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /*** Get a tunnel start entry ***/
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex, &tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VLANID_5_CNS, tunnelEntry.ipv4Cfg.vlanId,
                                 "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                                 tunnelEntry.ipv4Cfg.macDa.arEther[5]);

     /*  AUTODOC: configure tunnel start mac SA*/
    rc = prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],&prvTgfRestoreCfg.routerMacSaIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],routerMacSaIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaGet(prvTgfDevNum,routerMacSaIndex,&prvTgfRestoreCfg.macSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

    /* the mac SA for the TS header */
    macSaAddr.arEther[0] =  prvTgfPacketTunnelStartL2Part.saMac[0];
    macSaAddr.arEther[1] =  prvTgfPacketTunnelStartL2Part.saMac[1];
    macSaAddr.arEther[2] =  prvTgfPacketTunnelStartL2Part.saMac[2];
    macSaAddr.arEther[3] =  prvTgfPacketTunnelStartL2Part.saMac[3];
    macSaAddr.arEther[4] =  prvTgfPacketTunnelStartL2Part.saMac[4];
    macSaAddr.arEther[5] =  prvTgfPacketTunnelStartL2Part.saMac[5];

    rc = prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,routerMacSaIndex,&macSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");
}

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_MAC_MODE_ENT macMode = PRV_TGF_TTI_MAC_MODE_DA_E;
    PRV_TGF_TTI_RULE_UNT     pattern;
    PRV_TGF_TTI_RULE_UNT     mask;
    PRV_TGF_TTI_ACTION_2_STC ruleAction;
    PRV_TGF_TTI_IPV4_RULE_DEFAULT_INFO_STC defaultInfo;
    GT_U32  ethType1 = 0x6558;
    TGF_PACKET_L2_STC   *l2PartPtr;
    TGF_PACKET_IPV4_STC *ipv4PartPrt;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* AUTODOC: enable the TTI lookup for TTI_KEY_IPV4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* save the current gre ethernet type 1 */
    rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E,
                                  &prvTgfRestoreCfg.ethType1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeGet: %d", prvTgfDevNum);

    /* AUTODOC: for TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0 set ethertype to 0x6558 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                  PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E, ethType1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                 prvTgfDevNum);

    /* save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_IPV4 */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: configure default TTI action values */
    rc = prvTgfTtiDefaultAction2Get(&ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiDefaultAction2Get");

    /* AUTODOC: configure specific TTI action values */
    ruleAction.ttPassengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;

    /* use L2 part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_L2_E, &prvTgfEthernetOverIpv4PacketInfo, (GT_VOID *)&l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    /* use ipv4 part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &prvTgfEthernetOverIpv4PacketInfo, (GT_VOID *)&ipv4PartPrt);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    cpssOsMemCpy((GT_VOID*)defaultInfo.common.mac.arEther,
                 (GT_VOID*)l2PartPtr->daMac,
                 sizeof(GT_ETHERADDR));
    defaultInfo.common.srcPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    defaultInfo.common.vid = PRV_TGF_VLANID_5_CNS;
    cpssOsMemCpy(defaultInfo.destIp.arIP, ipv4PartPrt->dstAddr,
                 sizeof(GT_IPADDR));
    cpssOsMemCpy(defaultInfo.srcIp.arIP, ipv4PartPrt->srcAddr,
                 sizeof(GT_IPADDR));

    /* AUTODOC: configure default IPv4 pattern and mask values */
    rc = prvTgfTtiDefaultIpv4PatternMaskGet(defaultInfo, &pattern.ipv4, &mask.ipv4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiDefaultIpv4PatternMaskGet");

    /* AUTODOC: configure spesific IPv4 pattern and mask values */
    pattern.ipv4.tunneltype = 4;

    /* AUTODOC: add TTI rule 3 with: */
    /* AUTODOC:   key IPv4, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern pclId=1, srcPort=0, vid=5, tunneltype=4 */
    /* AUTODOC:   pattern MAC=00:00:00:00:34:02 */
    /* AUTODOC:   pattern srcIp=10.10.10.10, dstIp=4.4.4.4 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_ETHERNET_NO_CRC, port=1 */
    /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=0, tag1VlanCmd=MODIFY_ALL */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

}

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreBuildPacketSet function
* @endinternal
*
* @brief   Build packet
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_GRE_E
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreBuildPacketSet
(
    GT_VOID
)
{
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;
    TGF_PACKET_PAYLOAD_STC  *payloadPartPtr;

    /* DATA of packet - EtherType + IPv4 header */
    static GT_U8 prvTgfPayloadDataArr[] = {
    0x08, 0x00, 0x45, 0x00, 0x00, 0x44, 0x00, 0x00,
    0x00, 0x00, 0x40, 0x04, 0x74, 0xaf, 0x02, 0x02,
    0x02, 0x02, 0x01, 0x01, 0x01, 0x03, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
    };

    /* PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
        sizeof(prvTgfPayloadDataArr),                       /* dataLength */
        prvTgfPayloadDataArr                                /* dataPtr */
    };

    /* AUTODOC: get default IP-Ethernet-Over-IPv4-GRE packet */
    rc = prvTgfPacketEthOverIpv4PacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketDefaultPacketGet");

    /* change ipv4 part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &packetInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    ipv4PartPtr->totalLen = PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS;
    ipv4PartPtr->protocol = TGF_IPV4_GRE_PROTOCOL_CNS;
    ipv4PartPtr->csum = TGF_PACKET_AUTO_CALC_CHECKSUM_CNS;

    /* AUTODOC: change Ipv4 of tunnel */
    rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,GT_FALSE,0,ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");

    /* change payload part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_PAYLOAD_E, &packetInfo, (GT_VOID *)&payloadPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    payloadPartPtr->dataPtr = prvTgfPacketPayloadPart.dataPtr;
    payloadPartPtr->dataLength = prvTgfPacketPayloadPart.dataLength;

    /* AUTODOC: change payload */
    rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_PAYLOAD_E,GT_FALSE,0,payloadPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");

    prvTgfEthernetOverIpv4PacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfEthernetOverIpv4PacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfEthernetOverIpv4PacketInfo.partsArray =  packetInfo.partsArray;
}

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*/
static void prvTgfTunnelIpEtherOverIpv4GreCheckCaptureEgressTrafficOnPort
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC expectedPacketInfo;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;
    TGF_PACKET_L2_STC   *l2PartPtr;
    TGF_PACKET_IPV4_STC *ipv4PartPtr;
    TGF_PACKET_GRE_STC  *grePartPtr;
    TGF_PACKET_PAYLOAD_STC  *payloadPartPtr;
    GT_U32  byteNumMaskList[] = { 22, 23, 28, 29 }; /* list of bytes for which the comparison is prohibited
                                                       identification field - bits 22,23, checksum - bits 28,29 */

    /* DATA of packet - EtherType + IPv4 header */
    static GT_U8 prvTgfPayloadDataArr[] = {
    0x45, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x04, 0x74, 0xaf, 0x02, 0x02, 0x02, 0x02,
    0x01, 0x01, 0x01, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d, 0x2e, 0x2f
    };

    /* PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
        sizeof(prvTgfPayloadDataArr),                       /* dataLength */
        prvTgfPayloadDataArr                                /* dataPtr */
    };

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* copy the expected packet from the ingress packet */
    expectedPacketInfo.numOfParts = prvTgfEthernetOverIpv4PacketInfo.numOfParts;
    expectedPacketInfo.totalLen = prvTgfEthernetOverIpv4PacketInfo.totalLen;
    /* allocate proper memory for the 'expected' parts*/
    expectedPacketInfo.partsArray =
        cpssOsMalloc(expectedPacketInfo.numOfParts * sizeof(TGF_PACKET_PART_STC));

    if(expectedPacketInfo.partsArray == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_PTR,
                                     "cpssOsMalloc: failed");
        return;
    }

    /* copy expected parts from the original sent parts */
    for(ii = 0 ; ii < expectedPacketInfo.numOfParts ; ii++)
    {
        expectedPacketInfo.partsArray[ii] = prvTgfEthernetOverIpv4PacketInfo.partsArray[ii];
    }

    /* remove L2 of passenger, VLAN, Ethertype of passenger */
    ii = 1;
    while (expectedPacketInfo.partsArray[ii].type != TGF_PACKET_PART_L2_E)
        ii++;
    expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
    expectedPacketInfo.partsArray[ii].partPtr = NULL;
    while (expectedPacketInfo.partsArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
        ii++;
    expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
    expectedPacketInfo.partsArray[ii].partPtr = NULL;

    /* change L2 of tunnel */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_L2_E, &expectedPacketInfo, (GT_VOID *)&l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    cpssOsMemCpy(l2PartPtr->daMac, prvTgfPacketTunnelStartL2Part.daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(l2PartPtr->saMac, prvTgfPacketTunnelStartL2Part.saMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: change L2 of tunnel */
    rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_L2_E,GT_FALSE,0,l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");

    /* change ipv4 part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &expectedPacketInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    cpssOsMemCpy(ipv4PartPtr->dstAddr, prvTgfTunnelDstIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ipv4PartPtr->srcAddr, prvTgfTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

    ipv4PartPtr->timeToLive = (TGF_TTL)prvTgfTunnelTtl;
    ipv4PartPtr->totalLen = prvTgfTunnelTotalLength;
    ipv4PartPtr->protocol = TGF_IPV4_GRE_PROTOCOL_CNS;
    ipv4PartPtr->csum = TGF_PACKET_AUTO_CALC_CHECKSUM_CNS;

    /* AUTODOC: change Ipv4 of tunnel */
    rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,GT_FALSE,0,ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");

    /* change GRE part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_GRE_E, &expectedPacketInfo, (GT_VOID *)&grePartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    grePartPtr->protocol = TGF_ETHERTYPE_0800_IPV4_TAG_CNS;

    /* AUTODOC: change GRE of tunnel */
    rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_GRE_E,GT_FALSE,0,grePartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");


    /* change payload part */
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_PAYLOAD_E, &expectedPacketInfo, (GT_VOID *)&payloadPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    payloadPartPtr->dataPtr = prvTgfPacketPayloadPart.dataPtr;
    payloadPartPtr->dataLength = prvTgfPacketPayloadPart.dataLength;

    /* AUTODOC: change payload */
    rc = prvTgfPacketEthOverIpv4PacketHeaderDataSet(TGF_PACKET_PART_PAYLOAD_E,GT_FALSE,0,payloadPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketHeaderDataSet");

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &expectedPacketInfo,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            byteNumMaskList, /* skip check of bytes in these positions */
            sizeof(byteNumMaskList)/sizeof(byteNumMaskList[0]), /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* free the dynamic allocated memory */
    cpssOsFree(expectedPacketInfo.partsArray);
}


/**
* @internal prvTgfTunnelIpEtherOverIpv4GreTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:01:02:03:34:02,
*         macSa = 00:04:05:06:07:11,
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 1 Ip Ethernet over IPv4 Gre tunneled packets from port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:34:02, SA=00:04:05:06:07:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    /* AUTODOC:   GRE=00:00:65:58 */
    /* AUTODOC:   passenger DA=00:09:0A:0B:0C:22, SA=00:0D:0E:0F:00:25, VID=6 */

    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfEthernetOverIpv4PacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check captured egress traffic on port */
    prvTgfTunnelIpEtherOverIpv4GreCheckCaptureEgressTrafficOnPort();

    return;
}

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreConfigurationRestore
(
    GT_U32      prvUtfVrfId
)
{
    GT_STATUS rc = GT_OK;
    GT_IPADDR ipAddr;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: invalidate TTI rule 3 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC_MODE_DA for TTI_KEY_IPV4 */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for TTI_KEY_IPV4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore GRE ethernet type 1 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum,
                                  PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E, prvTgfRestoreCfg.ethType1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEthernetTypeSet: %d",
                                 prvTgfDevNum);

    prvTgfPacketRestoreDefaultParameters();

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* AUTODOC: restore Router MAC SA configuration */
    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],prvTgfRestoreCfg.routerMacSaIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,routerMacSaIndex,&prvTgfRestoreCfg.macSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPassengerDstIp, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_5_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterDel: %d",
                                     prvTgfDevNum);
    }

}


