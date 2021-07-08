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
* @file prvTgfTunnelTermPortGroupIpv4overIpv4.c
*
* @brief Tunnel Term: Ipv4 over Ipv4 PortGroup - Basic
*
* @version   21
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
#include <tunnel/prvTgfTunnelTermPortGroupIpv4overIpv4.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            CPSS_MAX_PORT_GROUPS_CNS

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS,
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS
};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* the TTI Rule index */
static GT_U32        prvTgfTtiRuleIndex        = 1;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* the Mask for MAC address */
static TGF_MAC_ADDR  prvTgfFullMaskMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* the Mask for IP address */
static TGF_IPV4_ADDR prvTgfFullMaskIp = {255, 255, 255, 255};

/* the Virtual Router exists flag */
static GT_BOOL       prvUtfVrExists = GT_FALSE;

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
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    0x5EB5,             /* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};
/* packet's IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OverIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x16,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x73E2,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
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

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4BaseConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                     isTagged  = GT_FALSE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence;

    /* AUTODOC: define tested ports - port in each group: */
    /* AUTODOC:   0, 18, 37, 54, 65, 83, 103, 120 */
    /* AUTODOC: Iterate thru send port (from each port group) */

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

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
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* add ports to vlan member */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter], isTagged);
    }

    /* set Port VID Precedence */
    precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    /* AUTODOC: set SOFT VID Precedence for sendPort */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum, prvTgfSendPort, precedence);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfSendPort, precedence);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfSendPort, &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: set PVID 5 for sendPort */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfSendPort, PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfSendPort, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
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
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4LttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
static GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4LttRouteConfigurationSet
(
    GT_U32      prvUtfVrfId
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U8                                   sendPortIndex;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths;
    GT_U32                                  ii;

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    rc = prvTgfDefPortsArrayPortToPortIndex(prvTgfSendPort,&sendPortIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArrayPortToPortIndex: %d", prvTgfSendPort);

    /* AUTODOC: enable Unicast IPv4 Routing on sendPort */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfSendPort);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfSendPort);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

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
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];
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
    if ((0 != prvUtfVrfId) && (GT_FALSE == prvUtfVrExists))
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
        prvUtfVrExists = GT_TRUE;
    }

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4RouteConfigurationSet
(
    IN GT_U32      prvUtfVrfId
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            prvTgfTunnelTermPortGroupIpv4overIpv4LttRouteConfigurationSet(prvUtfVrfId);
            break;

        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4TtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] prvUtfVrfId              - virtual router index,
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       None
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4TtiConfigurationSet
(
    IN  GT_U32                 prvUtfVrfId,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp
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

    /* AUTODOC: enable the TTI lookup for IPv4 at the sendPort */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfSendPort, PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: enable IPv4 TTI lookup for only tunneled packets received on sendPort */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfSendPort, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

    /* set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */

    /* set TTI Action */
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
    ttiAction.interfaceInfo.devPort.portNum = prvTgfSendPort;
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

    /* set TTI Pattern */
    cpssOsMemCpy(ttiPattern.ipv4.common.mac.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiPattern.ipv4.srcIp.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiPattern.ipv4.destIp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));

    ttiPattern.ipv4.common.pclId             = 1;
    ttiPattern.ipv4.common.srcIsTrunk        = 0;
    ttiPattern.ipv4.common.srcPortTrunk      = 4;
    ttiPattern.ipv4.common.vid               = PRV_TGF_SEND_VLANID_CNS;
    ttiPattern.ipv4.common.isTagged          = GT_TRUE;
    ttiPattern.ipv4.common.dsaSrcIsTrunk     = GT_FALSE;
    ttiPattern.ipv4.common.dsaSrcPortTrunk   = 0;
    ttiPattern.ipv4.common.dsaSrcDevice      = prvTgfDevNum;
    ttiPattern.ipv4.common.sourcePortGroupId = 0;
    ttiPattern.ipv4.tunneltype               = 0;
    ttiPattern.ipv4.isArp                    = GT_FALSE;


    /* set TTI Mask */
    cpssOsMemCpy(&ttiMask, &ttiPattern, sizeof(ttiPattern));
    cpssOsMemCpy(ttiMask.ipv4.common.mac.arEther, prvTgfFullMaskMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiMask.ipv4.srcIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiMask.ipv4.destIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    ttiMask.ipv4.common.srcIsTrunk = 1;
    ttiMask.ipv4.common.srcPortTrunk = 0xF;

    /* AUTODOC: add TTI rule 1 on portGroup with: */
    /* AUTODOC:   key IPv4, cmd FORWARD, redirectCmd TO_ROUTER_LOOKUP */
    /* AUTODOC:   pattern pclId=1, srcPort=0, vid=5, dsaSrcPort=0 */
    /* AUTODOC:   pattern MAC=00:00:00:00:34:02 */
    /* AUTODOC:   pattern srcIp=10.10.10.10, dstIp=4.4.4.4 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_IPV4 */
    rc = prvTgfTtiPortGroupRuleSet(prvTgfTtiRuleIndex, portGroupsBmp, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortGroupRuleSet: %d", prvTgfDevNum);

    /* check TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    /* AUTODOC: check added TTI Rule */
    rc = prvTgfTtiPortGroupRuleGet(prvTgfDevNum, portGroupsBmp, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortGroupRuleGet: %d", prvTgfDevNum);

    PRV_UTF_LOG3_MAC(" Pattern&Mask .ipv4.common.vid = %d, %d\n" \
                     " Action .vlanPrecedence = %d\n",
                     ttiPattern.ipv4.common.vid,
                     ttiMask.ipv4.common.vid,
                     ttiAction.vlanPrecedence);
}

/**
* @internal prvTgfTunnelTermPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Set TTI Rule Valid Status
*/
GT_VOID prvTgfTunnelTermPortGroupRuleValidStatusSet
(
    GT_BOOL   validStatus
)
{
    GT_STATUS       rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting TTI Rule Valid Status =======\n");

    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, validStatus);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
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
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* reset IP couters and set ROUTE_ENTRY mode */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter % 4);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


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

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send IPv4 over IPv4 tunneled packet from sendPort with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    /* AUTODOC:   passenger srcIP=2.2.2.2, dstIP=1.1.1.3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfSendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfSendPort);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get IPv4 packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:06 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        if (prvTgfPortsArray[portIter] == prvTgfSendPort)
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
        else if (prvTgfPortsArray[portIter] == prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS])
        {

            /* packetSize is changed. The new packet will be without VLAN_TAG and IPV4_HEADER */
            expectedPacketSize = (packetSize - TGF_VLAN_TAG_SIZE_CNS - TGF_IPV4_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS);
            /* check if there is need for padding */
            if (expectedPacketSize < 64)
                expectedPacketSize = 64;
            expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.brdcPktsSent.l[0]   = 0;
            expectedCntrs.mcPktsSent.l[0]     = 0;
            expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
            expectedCntrs.brdcPktsRcv.l[0]    = 0;
            expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else
        {
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
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac */
    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);


    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
        prvTgfCountersIpGet(prvTgfDevNum, portIter % 4, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - expect no traffic
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerateExpectNoTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
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

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* reset IP couters and set ROUTE_ENTRY mode */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter % 4);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


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

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfSendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfSendPort);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        if (prvTgfPortsArray[portIter] == prvTgfSendPort)
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
        else if (prvTgfPortsArray[portIter] == prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS])
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
        else
        {
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
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);


    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter % 4, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4ConfigurationRestore function
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
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4ConfigurationRestore
(
    GT_U32      prvUtfVrfId
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 3. Restore TTI Configuration
     */

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiPortGroupRuleValidStatusSet(CPSS_PORT_GROUP_UNAWARE_MODE_CNS, prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortGroupRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: restore the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiPortGroupMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortGroupMacModeSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d",prvTgfDevNum);

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

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: invalidate VLANs 5,6 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4PortConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration per port
*
* @param[in] sendPort                 - send port
*                                       None
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4PortConfigurationRestore
(
    GT_U32      sendPort
)
{
    GT_STATUS   rc        = GT_OK;
    GT_U8       sendPortIndex;

    /* AUTODOC: disable the IPv4 TTI lookup for only tunneled packets received on sendPort */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(sendPort, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for IPv4 at sendPort */
    rc = prvTgfTtiPortLookupEnableSet(sendPort, PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    rc = prvTgfDefPortsArrayPortToPortIndex(sendPort,&sendPortIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArrayPortToPortIndex: %d", prvTgfSendPort);

    /* AUTODOC: disable Unicast IPv4 Routing on sendPort */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, sendPort);

    /* AUTODOC: restore default Precedence on sendPort */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                           sendPort,
                                           CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, sendPort,
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);

}


