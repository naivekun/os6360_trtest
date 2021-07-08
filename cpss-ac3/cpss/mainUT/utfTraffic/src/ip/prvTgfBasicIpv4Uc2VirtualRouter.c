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
* @file prvTgfBasicIpv4Uc2VirtualRouter.c
*
* @brief Basic IPV4 UC 2 virtual Router
*
* @version   25
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
#include <ip/prvTgfBasicIpv4Uc2VirtualRouter.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* number of VLANs */
#define PRV_TGF_VLAN_COUNT_CNS            2

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* second port number to send traffic to */
#define PRV_TGF_SEND_2_PORT_IDX_CNS         2

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS     1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* default number of iterations */
static GT_U32        prvTgfIterCount   = 2;

/* VLANs array */
static GT_U8         prvTgfVlanArrays[PRV_TGF_VLAN_COUNT_CNS] =
{
    PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS
};

/* NextHop VLANs array */
static GT_U8         prvTgfNextHopVlanArrays[PRV_TGF_VLAN_COUNT_CNS] =
{
    PRV_TGF_NEXTHOPE_VLANID_CNS,
    PRV_TGF_SEND_VLANID_CNS
};

/* tx ports array */
static GT_U8         prvTgfTxPortsIdxArray[PRV_TGF_VLAN_COUNT_CNS] =
{
    PRV_TGF_SEND_PORT_IDX_CNS,
    PRV_TGF_SEND_2_PORT_IDX_CNS
};

/* NextHop ports array */
static GT_U8         prvTgfNextHopPortsIdxArray[PRV_TGF_VLAN_COUNT_CNS] =
{
    PRV_TGF_NEXTHOPE_PORT_IDX_CNS,
    PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS
};

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS
};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex[PRV_TGF_VLAN_COUNT_CNS] = {0, 1};

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex[PRV_TGF_VLAN_COUNT_CNS] = {25, 55};

/* the Virtual Route entry index for UC Route entry Table */
static GT_U32        prvTgfVirtualRouteEntryBaseIndex[PRV_TGF_VLAN_COUNT_CNS] = {30, 60};

/* the Virtual Route index */
GT_U32        prvTgfVirtualRouteIndex[PRV_TGF_VLAN_COUNT_CNS] = {1, 2};

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMacArr[PRV_TGF_VLAN_COUNT_CNS] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x33},
                                                                {0x00, 0x00, 0x00, 0x00, 0x00, 0x44}};

/* dst MAC address */
static TGF_MAC_ADDR  prvTgfDstMacArr[PRV_TGF_VLAN_COUNT_CNS] = {{0x00, 0x00, 0x00, 0x00, 0x34, 0x02},
                                                                {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}};

/* Destination IP addresses for packets */
static TGF_IPV4_ADDR  prvTgfDstAddrArr[2] =
{
    {1, 1, 1, 3},
    {1, 1, 1, 4}
};

/* stored default inlifMode for port 0*/
static PRV_TGF_INLIF_PORT_MODE_ENT     inlifMode;

/* stored default inlifMode for port 3 */
static PRV_TGF_INLIF_PORT_MODE_ENT     inlifMode1;

/* stored default Vlan ID for port 0 */
static GT_U16   prvTgfDefVlanId = 0;

/* stored default Vlan ID for port 2 */
static GT_U16   prvTgfDefVlanId1 = 0;

/* stored default Inlif entry */
PRV_TGF_INLIF_ENTRY_STC       inlifEntryRestore1;

/* stored default Inlif entry */
PRV_TGF_INLIF_ENTRY_STC       inlifEntryRestore2;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
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
    4,                                          /* version */
    5,                                          /* headerLen */
    0,                                          /* typeOfService */
    0x2A,                                       /* totalLen */
    0,                                          /* id */
    0,                                          /* flags */
    0,                                          /* offset */
    0x40,                                       /* timeToLive */
    0xFF,                                       /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,          /* csum */
    { 2,  2,  2,  2},                           /* srcAddr */
    { 1,  1,  1,  3}                            /* dstAddr */
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
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId              - base value for vrfId
*                                       None
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterBaseConfigurationSet
(
    GT_U32      prvUtfVrfId
)
{
    GT_BOOL                     isTagged  = GT_TRUE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    /*  update prvTgfVirtualRouteIndex accordingly prvUtfVrfId value */
    for (vlanIter = 0; vlanIter < PRV_TGF_VLAN_COUNT_CNS; vlanIter++)
    {
        prvTgfVirtualRouteIndex[vlanIter] = prvUtfVrfId + vlanIter;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    for (vlanIter = 0; vlanIter < PRV_TGF_VLAN_COUNT_CNS; vlanIter++)
    {
        vlanInfo.vrfId                = prvTgfVirtualRouteIndex[vlanIter];

        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, prvTgfVlanArrays[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArrays[vlanIter]);
    }

    /* add ports to vlan member */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter], isTagged);
    }

     /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);

     /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS], &prvTgfDefVlanId1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);


    /* AUTODOC: set PVID 6 for port 2 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    /* AUTODOC: enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* AUTODOC: enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_NEXTHOPE_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           ucRouteEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    routeEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defUcRouteEntryInfo;
    GT_U32                                  numOfPaths;
    GT_IPADDR                               ipAddr;
    GT_U32                                  vlanIter;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
     */

    /* save inlif entry for restore */
    rc = prvTgfInlifEntryGet(PRV_TGF_INLIF_TYPE_PORT_E,
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                        &inlifEntryRestore1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifEntryGet");

    rc = prvTgfInlifEntryGet(PRV_TGF_INLIF_TYPE_PORT_E,
                        prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS],
                        &inlifEntryRestore2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifEntryGet");


    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_TRUE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;

    /* SCT Section */
    inlifEntry.ipv4IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.ipv6IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipSecurityProfile              = 0;
    inlifEntry.ipv4IgmpToCpuEnable            = GT_FALSE;
    inlifEntry.ipv6IcmpToCpuEnable            = GT_FALSE;
    inlifEntry.udpBcRelayEnable               = GT_FALSE;
    inlifEntry.arpBcToCpuEnable               = GT_FALSE;
    inlifEntry.arpBcToMeEnable                = GT_FALSE;
    inlifEntry.ripv1MirrorEnable              = GT_FALSE;
    inlifEntry.ipv4LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6NeighborSolicitationEnable = GT_FALSE;

    /* Misc Section */
    inlifEntry.mirrorToAnalyzerEnable = GT_FALSE;
    inlifEntry.mirrorToCpuEnable      = GT_FALSE;


    /* save inlif port 0 mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_SEND_PORT_IDX_CNS, &inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeGet");

    /* save inlif port 2 mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_SEND_2_PORT_IDX_CNS, &inlifMode1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeGet");

    /* AUTODOC: set inlif mode to support single input logical interface for port 0  */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,
                                   PRV_TGF_INLIF_PORT_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeSet");

    /* AUTODOC: set inlif mode to support single input logical interface for port 2  */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_2_PORT_IDX_CNS,
                                   PRV_TGF_INLIF_PORT_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeSet");

    /* AUTODOC: config Inlif entry on port 0 with: */
    /* AUTODOC:   bridgeEnable=autoLearnEnable=naMessageToCpuEnable=TRUE */
    /* AUTODOC:   unkDaUcCmd=unregNonIpMcCmd=unregIpMcCmd=FORWARD */
    /* AUTODOC:   unregIpv4BcCmd=unregNonIpv4BcCmd=FORWARD, unkSaUcCmd=BRIDGE */
    inlifEntry.vrfId             = prvTgfVirtualRouteIndex[0];
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);


    /* AUTODOC: config Inlif entry on port 2 with: */
    /* AUTODOC:   bridgeEnable=autoLearnEnable=naMessageToCpuEnable=TRUE */
    /* AUTODOC:   unkDaUcCmd=unregNonIpMcCmd=unregIpMcCmd=FORWARD */
    /* AUTODOC:   unregIpv4BcCmd=unregNonIpv4BcCmd=FORWARD, unkSaUcCmd=BRIDGE */
    inlifEntry.vrfId             = prvTgfVirtualRouteIndex[1];
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS],
                             &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);


    /* make iteration */
    for (vlanIter = 0; vlanIter < prvTgfIterCount; vlanIter++)
    {
        /* -------------------------------------------------------------------------
         * 1. Enable Routing
         */

        /* AUTODOC: enable Unicast IPv4 Routing on port 0/2 */
        rc = prvTgfIpPortRoutingEnable(prvTgfTxPortsIdxArray[vlanIter],
                                       CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfTxPortsIdxArray[vlanIter]]);

        /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5/6 */
        rc = prvTgfIpVlanRoutingEnable(prvTgfVlanArrays[vlanIter], CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfVlanArrays[vlanIter]);

        /* -------------------------------------------------------------------------
        * 2. Create a new Virtual Router for the LPM DB
        */

        /* set a route entry with CMD_TRAP_TO_CPU as default ipv4 unicast
           index 30 vlan 5 port 0  arpIndex 0 --> default for VR 1
           index 60 vlan 6 port 18 arpIndex 1 --> default for VR 2  */
        cpssOsMemSet(&ucRouteEntry, 0, sizeof(ucRouteEntry));

        ucRouteEntry.cmd                              = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        ucRouteEntry.nextHopVlanId                    = 0;
        ucRouteEntry.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        ucRouteEntry.nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
        ucRouteEntry.nextHopInterface.devPort.portNum = prvTgfPortsArray[prvTgfTxPortsIdxArray[vlanIter]];
        ucRouteEntry.nextHopARPPointer                = prvTgfRouterArpIndex[vlanIter];

        /* AUTODOC: add UC route entry as default IPv4 UC with: */
        /* AUTODOC: cmd TRAP_TO_CPU, nexthop VLAN 5/6 and nexthop port 0/2 baseIndex 30/60 */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfVirtualRouteEntryBaseIndex[vlanIter], &ucRouteEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        cpssOsMemSet(&defUcRouteEntryInfo, 0, sizeof(defUcRouteEntryInfo));

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", prvTgfDevNum);

        if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
             /* for NON puma devices number of paths starts from 0 */
            numOfPaths = 0;
        }
        else
        {
            /* for puma devices number of paths starts from 1 */
            numOfPaths = 1;
        }


        /* set defUcRouteEntryInfo */
        defUcRouteEntryInfo.ipLttEntry.routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        defUcRouteEntryInfo.ipLttEntry.numOfPaths               = numOfPaths;
        defUcRouteEntryInfo.ipLttEntry.routeEntryBaseIndex      = prvTgfVirtualRouteEntryBaseIndex[vlanIter];
        defUcRouteEntryInfo.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;
        defUcRouteEntryInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        defUcRouteEntryInfo.ipLttEntry.ipv6MCGroupScopeLevel    = 0; /*CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E*/

        /* AUTODOC: create Virtual Router with created default route entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId,
                                         prvTgfVirtualRouteIndex[vlanIter],
                                         &defUcRouteEntryInfo, /*defIpv4UcRouteEntryInfo*/
                                         NULL,                    /*defIpv6UcRouteEntryInfo*/
                                         NULL,                    /*defIpv4McRouteLttEntry*/
                                         NULL,                    /*defIpv6McRouteLttEntry*/
                                         NULL);                   /*defFcoeRouteLttEntry*/
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd: %d", prvTgfDevNum);

        /* -------------------------------------------------------------------------
         * 3. Create the Route entry in Route Table (Next hop Table)
         */

        /* create a ARP MAC address to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMacArr[vlanIter], sizeof(TGF_MAC_ADDR));

        /* AUTODOC: write a ARP MAC 00:00:00:00:00:33 or 00:00:00:00:00:44 to the Router ARP Table */
        rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex[vlanIter], &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* create a UC Route entry for the Route Table with CMD_ROUTE */
        cpssOsMemSet(&ucRouteEntry, 0, sizeof(ucRouteEntry));

        ucRouteEntry.cmd                              = CPSS_PACKET_CMD_ROUTE_E;
        ucRouteEntry.nextHopVlanId                    = prvTgfNextHopVlanArrays[vlanIter];
        ucRouteEntry.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        ucRouteEntry.nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
        ucRouteEntry.nextHopInterface.devPort.portNum = prvTgfPortsArray[prvTgfNextHopPortsIdxArray[vlanIter]];
        ucRouteEntry.nextHopARPPointer                = prvTgfRouterArpIndex[vlanIter];

        /* AUTODOC: add UC IPv4 route entry with: */
        /* AUTODOC: cmd ROUTE, nexthop VLAN 6/5 and nexthop port 3/1 arpIndex 0/1*/
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex[vlanIter], &ucRouteEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: reading and checkiug the UC Route entry from the Route Table */
        cpssOsMemSet(&ucRouteEntry, 0, sizeof(ucRouteEntry));

        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex[vlanIter], &ucRouteEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

        PRV_UTF_LOG4_MAC("cmd = %d, nextHopVlanId = %d, portNum = %d, ARPPtr = %d\n",
                         ucRouteEntry.cmd,
                         ucRouteEntry.nextHopVlanId,
                         ucRouteEntry.nextHopInterface.devPort.portNum,
                         ucRouteEntry.nextHopARPPointer);

        /* -------------------------------------------------------------------------
         * 4. Create prefix in LPM Table
         */

        /* set LPM prefix as the DIP of test packet 1.1.1.3 or  1.1.1.4*/
        cpssOsMemCpy(ipAddr.arIP, prvTgfDstAddrArr[vlanIter], sizeof(ipAddr.arIP));

        /* set Route Entry Index the same as in Route Table */
        cpssOsMemSet(&routeEntryInfo, 0, sizeof(routeEntryInfo));
        routeEntryInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex[vlanIter];
        routeEntryInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

        /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 in VR 1 or 1.1.1.4/32 in VR 2 */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvTgfVirtualRouteIndex[vlanIter],
                                        ipAddr, 32, &routeEntryInfo, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

        /* -------------------------------------------------------------------------
         * 5. Set the FDB entry With DA_ROUTE
         */

        /* create a macEntry with .daRoute = GT_TRUE */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfDstMacArr[vlanIter], sizeof(TGF_MAC_ADDR));

        macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId         = prvTgfVlanArrays[vlanIter];
        macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[prvTgfTxPortsIdxArray[vlanIter]];
        macEntry.isStatic                       = GT_TRUE;
        macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.daRoute                        = GT_TRUE;
        macEntry.sourceId                       = 0;
        macEntry.userDefined                    = 0;
        macEntry.daQosIndex                     = 0;
        macEntry.saQosIndex                     = 0;
        macEntry.daSecurityLevel                = 0;
        macEntry.saSecurityLevel                = 0;
        macEntry.appSpecificCpuCode             = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

        /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 0 and
                    add FDB entry with MAC 00:00:00:00:34:03, VLAN 6, port 2  */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          vlanIter    = 0;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* make iteration */
    for (vlanIter = 0; vlanIter < prvTgfIterCount; vlanIter++)
    {
        PRV_UTF_LOG2_MAC("======= Generating Traffic from port %d, VLAN %d =======\n",
                         prvTgfPortsArray[prvTgfTxPortsIdxArray[vlanIter]], prvTgfVlanArrays[vlanIter]);

        /* -------------------------------------------------------------------------
         * 1. Setup counters and enable capturing
         */

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

        /* setup nexthope portInterface for capturing - first loop on port 3 second loop on port 1 */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfNextHopPortsIdxArray[vlanIter]];

        /* enable capture on nexthope port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfNextHopPortsIdxArray[vlanIter]]);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


        /* -------------------------------------------------------------------------
         * 2. Generating Traffic
         */

        /* number of parts in packet */
        partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* build packet */
        packetInfo.totalLen   = packetSize;
        packetInfo.numOfParts = partsCount;
        packetInfo.partsArray = prvTgfPacketPartArray;

         /* fill destination IP address for packet - first loop 1.1.1.3 second loop 1.1.1.4 */
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfDstAddrArr[vlanIter],
                     sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* set MAC DA and VlanId - first loop DA=00:00:00:00:34:02 vlan 5,
                                   second loop DA=00:00:00:00:34:03 vlan 6 */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDstMacArr[vlanIter], sizeof(TGF_MAC_ADDR));
        prvTgfPacketVlanTagPart.vid = prvTgfVlanArrays[vlanIter];

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* AUTODOC: send first packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
        /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=1.1.1.3 */
        /* AUTODOC: send second packet from port 2 with: */
        /* AUTODOC:   DA=00:00:00:00:34:03, SA=00:00:00:00:00:22 */
        /* AUTODOC:   VID=6, srcIP=1.1.1.1, dstIP=1.1.1.4 */

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfTxPortsIdxArray[vlanIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfTxPortsIdxArray[vlanIter]]);


        /* -------------------------------------------------------------------------
         * 3. Get Ethernet Counters
         */

        cpssOsTimerWkAfter(100);
        /* read and check ethernet counters */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            GT_BOOL isOk;

            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* AUTODOC: verify routed first packet on port 3, second packet on port 1 */
            switch (portIter) {
                 case PRV_TGF_SEND_PORT_IDX_CNS:

                     if(vlanIter == 0) /* first packet sent from port 0 */
                     {
                         /* packetSize is not changed */
                        expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                        expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                        expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                     }
                    else /* second packet sent from port 2 and not from port 0 */
                    {
                        expectedCntrs.goodOctetsSent.l[0] = 0;
                        expectedCntrs.goodPktsSent.l[0]   = 0;
                        expectedCntrs.ucPktsSent.l[0]     = 0;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = 0;
                        expectedCntrs.goodPktsRcv.l[0]    = 0;
                        expectedCntrs.ucPktsRcv.l[0]      = 0;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                    }

                    break;

                case PRV_TGF_SEND_2_PORT_IDX_CNS:

                   if(vlanIter == 1) /* second packet sent from port 2 */
                     {
                         /* packetSize is not changed */
                        expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                        expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                        expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                     }
                    else /* first packet sent from port 0 and not from port 2 */
                    {
                        expectedCntrs.goodOctetsSent.l[0] = 0;
                        expectedCntrs.goodPktsSent.l[0]   = 0;
                        expectedCntrs.ucPktsSent.l[0]     = 0;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = 0;
                        expectedCntrs.goodPktsRcv.l[0]    = 0;
                        expectedCntrs.ucPktsRcv.l[0]      = 0;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                    }
                    break;

                case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

                    if(vlanIter == 0) /* first packet arrive to port 3 */
                    {
                        /* packetSize is not changed */
                        expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                        expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                        expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                    }
                    else /* second packet do not arrive to port 3 */
                    {
                        expectedCntrs.goodOctetsSent.l[0] = 0;
                        expectedCntrs.goodPktsSent.l[0]   = 0;
                        expectedCntrs.ucPktsSent.l[0]     = 0;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = 0;
                        expectedCntrs.goodPktsRcv.l[0]    = 0;
                        expectedCntrs.ucPktsRcv.l[0]      = 0;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                    }
                    break;

                case PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS:

                    if(vlanIter == 1) /* second packet arrive to port 1 */
                    {
                        /* packetSize is not changed */
                        expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                        expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                        expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                    }
                    else /* first packet do not arrive to port 1 */
                    {
                        expectedCntrs.goodOctetsSent.l[0] = 0;
                        expectedCntrs.goodPktsSent.l[0]   = 0;
                        expectedCntrs.ucPktsSent.l[0]     = 0;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.mcPktsSent.l[0]     = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = 0;
                        expectedCntrs.goodPktsRcv.l[0]    = 0;
                        expectedCntrs.ucPktsRcv.l[0]      = 0;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;
                        expectedCntrs.mcPktsRcv.l[0]      = 0;
                    }
                    break;

                default:

                    /* for other ports */
                    expectedCntrs.goodOctetsSent.l[0] = 0;
                    expectedCntrs.goodPktsSent.l[0]   = 0;
                    expectedCntrs.ucPktsSent.l[0]     = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.ucPktsRcv.l[0]      = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;

                    break;
            }

            isOk =
                portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
                portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
                portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
                portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
                portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
                portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
                portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
                portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
                portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
                portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

            UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

            /* print expected values if bug */
            if (isOk != GT_TRUE) {
                PRV_UTF_LOG0_MAC("Expected values:\n");
                PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
                PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
                PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
                PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
                PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
                PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
                PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
                PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
                PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
                PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
                PRV_UTF_LOG0_MAC("\n");
            }
        }


        /* -------------------------------------------------------------------------
         * 4. Get Trigger Counters
         */

        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMacArr[vlanIter], sizeof(TGF_MAC_ADDR));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                prvTgfArpMacArr[vlanIter][0], prvTgfArpMacArr[vlanIter][1], prvTgfArpMacArr[vlanIter][2],
                prvTgfArpMacArr[vlanIter][3], prvTgfArpMacArr[vlanIter][4], prvTgfArpMacArr[vlanIter][5]);


        /* -------------------------------------------------------------------------
         * 5. Get IP Counters
         */

        /* AUTODOC: get and print ip counters values */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
            PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
            prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
        }
        PRV_UTF_LOG0_MAC("\n");

        /* disable capture on nexthope port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfNextHopPortsIdxArray[vlanIter]]);
    }
}

/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

     /* AUTODOC: restore inlif port mode for port 0 */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfInlifPortModeSet");
     /* AUTODOC: restore inlif port mode for port 2 */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_2_PORT_IDX_CNS,inlifMode1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfInlifPortModeSet");

    /* AUTODOC: restore inlif entry for port 0 */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             &inlifEntryRestore1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: restore inlif entry for port 2 */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS],
                             &inlifEntryRestore2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);


    /* make iteration */
    for (vlanIter = 0; vlanIter < prvTgfIterCount; vlanIter++)
    {
        /* fill the destination IP address for LPM prefix */
        cpssOsMemCpy(ipAddr.arIP, prvTgfDstAddrArr[vlanIter], sizeof(ipAddr.arIP));

        /* AUTODOC: delete the LPM prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,prvTgfVirtualRouteIndex[vlanIter], ipAddr, 32);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

        /* AUTODOC: restore virtual router */

        /* delete virtual router */
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvTgfVirtualRouteIndex[vlanIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", prvTgfDevNum);

        /* AUTODOC: disable Unicast IPv4 Routing on port 0/2 */
        rc = prvTgfIpPortRoutingEnable(prvTgfTxPortsIdxArray[vlanIter],
                                       CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfTxPortsIdxArray[vlanIter]]);

        /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5/6 */
        rc = prvTgfIpVlanRoutingEnable(prvTgfVlanArrays[vlanIter], CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfVlanArrays[vlanIter]);
    }

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* delete Ports from VLANs */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* delete port from VLAN */
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                       prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: delete VLANs 5,6 */
    for (vlanIter = 0; vlanIter < PRV_TGF_VLAN_COUNT_CNS; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArrays[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArrays[vlanIter]);
    }

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfDefVlanId);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfDefVlanId);


      /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS], prvTgfDefVlanId1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                               prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS], prvTgfDefVlanId1);

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_NEXTHOPE_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

}


