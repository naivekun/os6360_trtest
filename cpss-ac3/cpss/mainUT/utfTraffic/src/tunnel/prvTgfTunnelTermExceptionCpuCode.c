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
* @file prvTgfTunnelTermExceptionCpuCode.c
*
* @brief Tunnel Term: CPU codes for exception and masking of IP header errors
* tested APIs: cpssDxChTtiExceptionCpuCodeSet, cpssDxChIpHeaderErrorMaskSet
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermExceptionCpuCode.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLANID_CNS       6

/* virtual bridge VLAN Id */
#define PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS  0x1005
#define PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS 0x1092

#define PRV_TGF_DEFAULT_VLAN_CNS          0x92

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS     3

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* packet passenger DIP */
#define PRV_CPSS_PKT_PASSENGER_DIP      {1,  1,  1,  3}

/* packet tunnel DIP */
#define PRV_CPSS_PKT_TUNNEL_DIP         {4,  4,  4,  4}

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U32         prvTgfVlanArray[3] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOP_VLANID_CNS, 0};

/* VLANs array */
static GT_U32         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOP_VLANID_CNS, PRV_TGF_NEXTHOP_VLANID_CNS
};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* stored default Vlan ID */
static GT_U16        prvTgfDefVlanId = 0;

/* the TTI Rule index */
static GT_U32        prvTgfTtiRuleBaseIndex    = 1;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* the Mask for MAC address */
static TGF_MAC_ADDR  prvTgfFullMaskMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* the Mask for IP address */
static TGF_IPV4_ADDR prvTgfFullMaskIp = {255, 255, 255, 255};

/* the Virtual Router index */
static GT_U32        prvUtfVrfId                = 0;

/* the expected CPU code */
static CPSS_NET_RX_CPU_CODE_ENT ipv4SipAddrErrorCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 10;

/******************************* Test packet **********************************/

/* L2 part of unicast packet */
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
/* Unicast packet's IPv4 */

/* L2 part of unicast packet */
static TGF_IPV4_ADDR prvTgfPacketPassengerDip = PRV_CPSS_PKT_PASSENGER_DIP;

static TGF_PACKET_IPV4_STC prvTgfPacketIpv4SipAddrErrorPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    {255, 255, 255, 255},/* srcAddr */
    PRV_CPSS_PKT_TUNNEL_DIP    /* dstAddr */
};

static TGF_PACKET_IPV4_STC prvTgfPacketIpv4HeaderErrorPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 3,  3,  3,  3},   /* srcAddr */
    PRV_CPSS_PKT_TUNNEL_DIP    /* dstAddr */
};

static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OverIpv4SipAddrErrorPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    PRV_CPSS_PKT_PASSENGER_DIP    /* dstAddr */
};

static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OverIpv4HeaderErrorPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    PRV_CPSS_PKT_PASSENGER_DIP,   /* srcAddr is dstAddr */
    PRV_CPSS_PKT_PASSENGER_DIP    /* dstAddr */
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

/* PARTS of IPv4 SIP address error packet */
static TGF_PACKET_PART_STC prvTgfIpv4SipAddrErrorPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4SipAddrErrorPart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OverIpv4SipAddrErrorPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of IPv4 header error packet */
static TGF_PACKET_PART_STC prvTgfIpv4HeaderErrorPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4HeaderErrorPart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OverIpv4HeaderErrorPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
    CPSS_NET_RX_CPU_CODE_ENT      ipv4SipAddrErrorCpuCode;
    CPSS_PACKET_CMD_ENT           ipv4SipAddrErrorCmd;
    PRV_TGF_IP_EXC_CMD_ENT        ipv4HeaderErrorCmd;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermExceptionCpuCodeBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeBaseConfigurationSet
(
    GT_U32     vrfId
)
{
    GT_BOOL                     isTagged  = GT_FALSE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = 2;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence;

    /* save virtual router index */
    prvUtfVrfId = vrfId;

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */
    /* AUTODOC: get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* AUTODOC: set vlan entry */
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
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, (GT_U16)prvTgfVlanArray[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* AUTODOC: add ports to vlan member */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, (GT_U16)prvTgfVlanPerPortArray[portIter],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter], isTagged);
    }

    /* AUTODOC: set SOFT VID Precedence for port 0 */
    precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], precedence);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], precedence);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* AUTODOC: create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
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

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermExceptionCpuCodeRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths;
    GT_U32                                  ii;


    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. AUTODOC: Set Inlif Entry - that needs for Puma devices
     */

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
    inlifEntry.vrfId             = prvUtfVrfId;

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

    /* AUTODOC: call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, 4096, &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

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

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: write a UC Route entry to the Route Table */
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
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOP_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
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

        /* define max number of paths */
        if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            numOfPaths = 0;
        }
        else
        {
            numOfPaths = 1;
        }

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
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
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
     * 4. AUTODOC: Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* AUTODOC: fill a nexthop info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketPassengerDip, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermExceptionCpuCodeTtiBasicRule function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] pclId                    - pclId
* @param[in] macAddrPtr               - pointer to mac address
* @param[in] sipPtr                   - pointer to SIP
* @param[in] dipPtr                   - pointer to DIP
* @param[in] vid                      - vlan id
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
static GT_VOID prvTgfTunnelTermExceptionCpuCodeTtiBasicRule
(
    IN GT_U32                       pclId,
    IN TGF_MAC_ADDR                 *macAddrPtr,
    IN TGF_IPV4_ADDR                *sipPtr,
    IN TGF_IPV4_ADDR                *dipPtr,
    IN GT_U16                       vid,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiMaskPtr
)
{
    /* AUTODOC: set TTI Pattern */
    cpssOsMemCpy(ttiPatternPtr->ipv4.common.mac.arEther, macAddrPtr, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiPatternPtr->ipv4.srcIp.arIP, sipPtr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiPatternPtr->ipv4.destIp.arIP, dipPtr, sizeof(TGF_IPV4_ADDR));

    ttiPatternPtr->ipv4.common.pclId            = pclId;
    ttiPatternPtr->ipv4.common.srcIsTrunk       = 0;
    ttiPatternPtr->ipv4.common.srcPortTrunk     = 0;
    ttiPatternPtr->ipv4.common.vid              = vid;
    ttiPatternPtr->ipv4.common.isTagged         = GT_TRUE;
    ttiPatternPtr->ipv4.common.dsaSrcIsTrunk    = GT_FALSE;
    ttiPatternPtr->ipv4.common.dsaSrcPortTrunk  = 0;
    ttiPatternPtr->ipv4.common.dsaSrcDevice     = prvTgfDevNum;
    ttiPatternPtr->ipv4.tunneltype              = 0;
    ttiPatternPtr->ipv4.isArp                   = GT_FALSE;

    /* AUTODOC: set TTI Mask */
    cpssOsMemCpy(ttiMaskPtr, ttiPatternPtr, sizeof(PRV_TGF_TTI_RULE_UNT));
    cpssOsMemCpy(ttiMaskPtr->ipv4.common.mac.arEther, prvTgfFullMaskMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiMaskPtr->ipv4.srcIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiMaskPtr->ipv4.destIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    ttiMaskPtr->ipv4.common.srcIsTrunk = 1;
}

/**
* @internal prvTgfTunnelTermExceptionCpuCodeTtiBasicConfiguration function
* @endinternal
*
* @brief   Set TTI Basic Configuration
*
* @param[in] vid                      - vlan id
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
static GT_VOID prvTgfTunnelTermExceptionCpuCodeTtiBasicConfiguration
(
    IN GT_U16                       vid,
    INOUT PRV_TGF_TTI_RULE_UNT      *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT      *ttiMaskPtr,
    PRV_TGF_TTI_EXCEPTION_ENT       exceptionType
)
{
    GT_STATUS                rc;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */
    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */

    /* AUTODOC: enable the TTI lookup for IPv4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    /* AUTODOC: set lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: enable IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet");

    /* build rule*/
    switch (exceptionType)
    {
        case PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E:
            prvTgfTunnelTermExceptionCpuCodeTtiBasicRule(
                1,/*pclId*/
                &prvTgfPacketL2Part.daMac,
                &prvTgfPacketIpv4SipAddrErrorPart.srcAddr,
                &prvTgfPacketIpv4SipAddrErrorPart.dstAddr,
                vid,
                ttiPatternPtr, ttiMaskPtr);
            break;

        case PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E:
            prvTgfTunnelTermExceptionCpuCodeTtiBasicRule(
                1,/*pclId*/
                &prvTgfPacketL2Part.daMac,
                &prvTgfPacketIpv4HeaderErrorPart.srcAddr,
                &prvTgfPacketIpv4HeaderErrorPart.dstAddr,
                vid,
                ttiPatternPtr, ttiMaskPtr);
            break;

        default:
            return;
    }
    return;
}
/**
* @internal prvTgfTunnelTermExceptionCpuCodeTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeTtiConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ttiAction = {0};
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */

    /* AUTODOC: set TTI Action */
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.tunnelTerminate               = GT_TRUE;
    ttiAction.passengerPacketType           = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.copyTtlFromTunnelHeader       = GT_FALSE;
    ttiAction.mirrorToIngressAnalyzerEnable = GT_FALSE;
    ttiAction.policerIndex                  = 0;
    ttiAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiAction.vlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    ttiAction.vlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.vlanId                        = 0;
    ttiAction.nestedVlanEnable              = GT_FALSE;
    ttiAction.tunnelStart                   = GT_FALSE;
    ttiAction.tunnelStartPtr                = 0;
    ttiAction.routerLookupPtr               = 0;
    ttiAction.vrfId                         = prvUtfVrfId;
    ttiAction.sourceIdSetEnable             = GT_FALSE;
    ttiAction.sourceId                      = 0;
    ttiAction.bindToPolicer                 = GT_FALSE;
    ttiAction.qosPrecedence                 = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.qosTrustMode                  = PRV_TGF_TTI_QOS_KEEP_PREVIOUS_E;
    ttiAction.qosProfile                    = 0;
    ttiAction.modifyUpEnable                = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
    ttiAction.modifyDscpEnable              = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
    ttiAction.up                            = 0;
    ttiAction.remapDSCP                     = GT_FALSE;
    ttiAction.vntl2Echo                     = GT_FALSE;
    ttiAction.bridgeBypass                  = GT_FALSE;
    ttiAction.actionStop                    = GT_FALSE;
    ttiAction.activateCounter               = GT_FALSE;
    ttiAction.counterIndex                  = 0;

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */
    /* AUTODOC: set basic configuration for TTI */
    prvTgfTunnelTermExceptionCpuCodeTtiBasicConfiguration(PRV_TGF_SEND_VLANID_CNS, &ttiPattern, &ttiMask, PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E);

    /* AUTODOC: add TTI rule 1 with IPv4 key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD */
    /* AUTODOC:   IPv4 passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    /* AUTODOC:   srcIp=255.255.255.255, dstIp=4.4.4.4 */
    rc = prvTgfTtiRuleSet(prvTgfTtiRuleBaseIndex, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */
    /* AUTODOC: set basic configuration for TTI */
    prvTgfTunnelTermExceptionCpuCodeTtiBasicConfiguration(PRV_TGF_SEND_VLANID_CNS, &ttiPattern, &ttiMask, PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E);

    /* AUTODOC: add TTI rule 1 with IPv4 key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD */
    /* AUTODOC:   IPv4 passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    /* AUTODOC:   srcIp=3.3.3.3, dstIp=4.4.4.4 */
    rc = prvTgfTtiRuleSet(prvTgfTtiRuleBaseIndex + 1, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* AUTODOC: Save CPU code for SIP address error */
    rc = prvTgfTtiExceptionCpuCodeGet(prvTgfDevNum,
                                      PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E,
                                      &prvTgfRestoreCfg.ipv4SipAddrErrorCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCpuCodeGet");

    /* AUTODOC: Set CPU code for SIP address error */
    rc = prvTgfTtiExceptionCpuCodeSet(prvTgfDevNum,
                                      PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E,
                                      ipv4SipAddrErrorCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCpuCodeSet");

    /* AUTODOC: Get packet command for SIP address error */
    rc = prvTgfTtiExceptionCmdGet(prvTgfDevNum,
                                  PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E,
                                  &prvTgfRestoreCfg.ipv4SipAddrErrorCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdGet");

    /* AUTODOC: Set packet command for SIP address error */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum,
                                  PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E,
                                  CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    /* AUTODOC: Get packet command for passenger IPv4 header error */
    rc = prvTgfIpExceptionCommandGet(prvTgfDevNum,
                                     PRV_TGF_IP_EXCP_HDR_ERROR_E,
                                     CPSS_IP_PROTOCOL_IPV4_E,
                                     CPSS_IP_UNICAST_E,
                                     &prvTgfRestoreCfg.ipv4HeaderErrorCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandGet");

    /* AUTODOC: Set packet command for passenger IPv4 header error */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum,
                                     PRV_TGF_IP_EXCP_HDR_ERROR_E,
                                     CPSS_IP_PROTOCOL_IPV4_E,
                                     CPSS_IP_UNICAST_E,
                                     PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet");
}

/**
* @internal prvTgfTunnelTermExceptionCpuCodeMaskHeaderErrorType function
* @endinternal
*
* @brief   Mask/unmask IPv4 UC header error type
*
* @param[in] errorType                - type of IPv4 UC error
* @param[in] mask                     - GT_TRUE:  the error
*                                      GT_FALSE: unmask the error
*                                       None
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeMaskHeaderErrorType
(
    PRV_TGF_IP_HEADER_ERROR_ENT     errorType,
    GT_BOOL                         mask
)
{
    GT_STATUS rc;

    rc = prvTgfIpHeaderErrorMaskSet(prvTgfDevNum, errorType,
                                    CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E, mask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpHeaderErrorMaskSet");
}

/**
* @internal prvTgfTunnelTermExceptionCpuCodeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] test                     - 0: send IPv4 packet with SIP address error
*                                      1: send IPv4 packet with header error - expect trapping to CPU
*                                      2: send IPv4 packet with header error - expect forwarding
*                                       None
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeTrafficGenerate
(
    IN GT_U8    test
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          numOfExpectedRxPackets, numOfTrappedPackets;
    GT_BOOL                         getFirst;
    static GT_U8                    packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                          buffLen;
    GT_U32                          packetActualLength = 0;
    TGF_NET_DSA_STC                 rxParam;
    GT_U8                           dev = 0;
    GT_U8                           queue = 0;
    GT_U32                          cpuCode;
    GT_U32                          expectedCpuCode;

    /* AUTODOC: GENERATE TRAFFIC: */

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS]);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfIpv4SipAddrErrorPacketPartArray) / sizeof(prvTgfIpv4SipAddrErrorPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfIpv4SipAddrErrorPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    switch (test)
    {
        case 1:
            numOfExpectedRxPackets = 0;
            numOfTrappedPackets = prvTgfBurstCount;
            packetInfo.partsArray = prvTgfIpv4HeaderErrorPacketPartArray;
            break;

        case 2:
            numOfExpectedRxPackets = prvTgfBurstCount;
            numOfTrappedPackets = 0;
            packetInfo.partsArray = prvTgfIpv4HeaderErrorPacketPartArray;
            break;

        case 0:
        default:
            numOfExpectedRxPackets = 0;
            numOfTrappedPackets = prvTgfBurstCount;
            packetInfo.partsArray = prvTgfIpv4SipAddrErrorPacketPartArray;
    }

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:05, SA=00:00:00:00:00:15 */
    /* AUTODOC:   VID=5, Ethertype=0x8847, MPLS Label=0x1000 */
    /* AUTODOC:   Passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:12 */

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS]);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get Ethernet packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:12 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

                /* AUTODOC: packetSize is not changed */
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
                break;

        case PRV_TGF_NEXTHOP_PORT_IDX_CNS:

                /* packetSize is changed */
                expectedPacketSize = (packetSize - TGF_IPV4_HEADER_SIZE_CNS - TGF_VLAN_TAG_SIZE_CNS - TGF_CRC_LEN_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * numOfExpectedRxPackets;
                expectedCntrs.goodPktsSent.l[0]   = numOfExpectedRxPackets;
                expectedCntrs.ucPktsSent.l[0]     = numOfExpectedRxPackets;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * numOfExpectedRxPackets;
                expectedCntrs.goodPktsRcv.l[0]    = numOfExpectedRxPackets;
                expectedCntrs.ucPktsRcv.l[0]      = numOfExpectedRxPackets;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
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

    getFirst = GT_TRUE;
    buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    expectedCpuCode = ((test == 0) ? ipv4SipAddrErrorCpuCode : CPSS_NET_IP_HDR_ERROR_E);
    for (; numOfTrappedPackets > 0; numOfTrappedPackets--)
    {
        /* AUTODOC: get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet\n");

        /* AUTODOC: check the CPU code */
        cpuCode = (GT_U32)rxParam.cpuCode;
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuCode, cpuCode,
                                     "Wrong CPU code, expected: %d received %d",
                                     expectedCpuCode, cpuCode);
        getFirst = GT_FALSE;
    }
    /* AUTODOC: get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet\n");
}

/**
* @internal prvTgfTunnelTermExceptionCpuCodeConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = 3;
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 3. Restore TTI Configuration
     */

    /* AUTODOC: Restore packet command for passenger IPv4 header error */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum,
                                     PRV_TGF_IP_EXCP_HDR_ERROR_E,
                                     CPSS_IP_PROTOCOL_IPV4_E,
                                     CPSS_IP_UNICAST_E,
                                     prvTgfRestoreCfg.ipv4HeaderErrorCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet");

    /* AUTODOC: Restore packet command for IPv4 header error */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum,
                                  PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E,
                                  prvTgfRestoreCfg.ipv4SipAddrErrorCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    /* AUTODOC: Restore CPU code for SIP address error */
    rc = prvTgfTtiExceptionCpuCodeSet(prvTgfDevNum,
                                      PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E,
                                      prvTgfRestoreCfg.ipv4SipAddrErrorCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCpuCodeSet");

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleBaseIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rule 2 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleBaseIndex + 1, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for IPv4 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* AUTODOC: fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketPassengerDip, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
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

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: restore default Precedence on port 0 */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);

    /* AUTODOC: invalidate VLANs 5,6, 0x1005, 0x1092 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* AUTODOC: invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate((GT_U16)prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

     /* AUTODOC: disable PVID forcing on port 0 */
    rc = prvTgfBrgVlanPortForcePvidEnable(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


