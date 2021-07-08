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
* @file prvTgfBasicIpv6UcRouting.c
*
* @brief Basic IPV6 UC Routing
*
* @version   36
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
#include <common/tgfLpmGen.h>
#include <ip/prvTgfBasicIpv6UcRouting.h>
#include <cpss/common/cpssTypes.h>
#include <bridge/prvTgfVlanManipulation.h>
#include <gtOs/gtOsMem.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>


extern GT_STATUS appDemoHwAccessFailerBind(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
);

extern GT_STATUS appDemoHwAccessCounterGet(
    OUT GT_U32   *hwAccessCounterPtr
);

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

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

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* if firstCallToLpmConfig= GT_TRUE then save default status
   of bypassing lookup stages 8-31 */
static GT_BOOL       firstCallToLpmConfig=GT_TRUE;

/* errata flag (FE-)
  no exception for an LPM search of more than 4B (ipv6) when
  working in bypass mode                                      */
static GT_BOOL  lpmExceptionStatusErrata   = GT_FALSE;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
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
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};
/* packet's IPv6 - used for bulk tests */
static TGF_PACKET_IPV6_STC prvTgfPacket2Ipv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6546, 0, 0, 0, 0, 0, 0x0000, 0x4212}, /* TGF_IPV6_ADDR srcAddr */
    {0x2233, 0, 0, 0, 0, 0, 0x5678, 0x90ab}  /* TGF_IPV6_ADDR dstAddr */
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
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet - used for bulk tests */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacket2Ipv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_INLIF_PORT_MODE_ENT                 sendPortInlifMode;
    GT_BOOL                                     defaultBypassEnable;
    GT_BOOL                                     defaultSourceEportSipLookupEnable;
    GT_BOOL                                     defaultSourceEportSipSaEnable;
} prvTgfRestoreCfg;

/* indication that we check the egress port as dsa tag port */
static GT_BOOL              testEgressPortAsDsa = GT_FALSE;
/* The DSA tag type for the egress port - relevant when testEgressPortAsDsa = GT_TRUE */
static TGF_DSA_TYPE_ENT     testEgressDsaType = TGF_DSA_LAST_TYPE_E;

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     prvTgfBasicIpv6UcRoutingEgrCascadePort_DsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x00,0x10,0x06,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x12,0x00,0x00,0x10,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x00,0x10,0x06,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x92,0x00,0x00,0x10,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0xa0,0x00,0x00,0x00,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0x02,0x00,0x00,0x00,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv6UcRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv6UcRoutingBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {0, 1, 0, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    GT_U32  phyPort;
    GT_HW_DEV_NUM hwDevNum;

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set hwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(prvTgfBasicIpv6UcRoutingEgrCascadePort_DsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(prvTgfBasicIpv6UcRoutingEgrCascadePort_DsaWordsVfd[1].patternPtr[3], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[1].patternPtr[3], hwDevNum);
 
    /* AUTODOC: SETUP CONFIGURATION: */
    phyPort = prvTgfPortsArray[3] & 0x7f; /*DSA*/
    /* set up vfd array (0-1 word of DSA tag) according to device port mapping  */
    /* Trg Phy Port [11:5] according to FS  */
    prvTgfBasicIpv6UcRoutingEgrCascadePort_DsaWordsVfd[1].patternPtr[3] |= phyPort << 5;
    prvTgfBasicIpv6UcRoutingEgrCascadePort_DsaWordsVfd[1].patternPtr[2] |= phyPort >> 3;

    phyPort = prvTgfPortsArray[3] & 0xff;/* eDSA*/

    prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[1].patternPtr[3] |= phyPort << 5;
    prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[1].patternPtr[2] |= phyPort >> 3;

    /* set up vfd array (2-3 word of DSA tag) according to device port mapping */
    /* Trg ePort [18:7] according to FS  */

    prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[3].patternPtr[3] |= phyPort << 7;
    prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[3].patternPtr[2] |= phyPort >> 1;

    /* AUTODOC: create VLANs 5 with untagged ports [0,2], tagged port 1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLANs 6 with tagged port 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
            prvTgfPortsArray + 3, NULL, tagArray + 3, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* create a macEntry in VLAN 5 */
    /*rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_SEND_VLANID_CNS, prvTgfDevNum,
            PRV_TGF_FDB_PORT_NUM_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);*/

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
* @internal prvTgfBasicIpv6UcRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
*                                       None
*/
static GT_VOID prvTgfBasicIpv6UcRoutingLttRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_BOOL  useBulk
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPV6ADDR                             ipAddr;
    GT_U32                                  bulkVrId[2];
    GT_IPV6ADDR                             bulkIpAddr[2];
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    bulkNextHopInfo[2];
    GT_U32                                  bulkPrefixLen[2];
    GT_BOOL                                 bulkOverride[2];
    GT_U32                                  ii;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths;

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    /* set port inlif mode for port mode (needed for ExMxPm only) */

    /* save current port inlif mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_SEND_PORT_IDX_CNS,&(prvTgfRestoreCfg.sendPortInlifMode));
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeGet);
    /* set port inlif mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,PRV_TGF_INLIF_PORT_MODE_PORT_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);

    cpssOsMemSet(&inlifEntry,0,sizeof(inlifEntry));
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.vrfId                          = prvUtfVrfId;

    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,PRV_TGF_SEND_PORT_IDX_CNS,&inlifEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifEntrySet);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

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
            ipLttEntryPtr->routeEntryBaseIndex      = prvTgfRouteEntryBaseIndex;
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
            ipLttEntryPtr->priority = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E;
        }

        /* disable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_FALSE);

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
     * 4. Create a new Ipv6 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: for regular test add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:ccdd:eeff/128 */
    /* AUTODOC: for bulk test add IPv6 UC prefix 2233:0000:0000:0000:0000:0000:5678:90ab/128 */
    if (useBulk == GT_FALSE)
    {
        /* fill a destination IPv6 address for the prefix */
        for (ii = 0; ii < 8; ii++)
        {
            ipAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
            ipAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
        }
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                        128, &nextHopInfo, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
    }
    else
    {
        bulkVrId[0] = prvUtfVrfId;
        bulkVrId[1] = prvUtfVrfId;
        /* fill a destination IPv6 address for the bulk of prefixes */
        for (ii = 0; ii < 8; ii++)
        {
            bulkIpAddr[0].arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
            bulkIpAddr[0].arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
            bulkIpAddr[1].arIP[ii * 2]     = (GT_U8)(prvTgfPacket2Ipv6Part.dstAddr[ii] >> 8);
            bulkIpAddr[1].arIP[ii * 2 + 1] = (GT_U8) prvTgfPacket2Ipv6Part.dstAddr[ii];
        }
        bulkPrefixLen[0] = bulkPrefixLen[1] = 128;
        cpssOsMemCpy(&bulkNextHopInfo[0], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemCpy(&bulkNextHopInfo[1], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        bulkOverride[0] = bulkOverride[1] = GT_FALSE;
        rc = prvTgfIpLpmIpv6UcPrefixBulkAdd(prvTgfLpmDBId, bulkVrId, bulkIpAddr, bulkPrefixLen, bulkNextHopInfo, bulkOverride, GT_FALSE, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixBulkAdd: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfBasicIpv6LpmConfigurationSet function
* @endinternal
*
* @brief   Set LPM basic configuration
*
* @param[in] portNum                  - port number to disable the SIP lookup on
* @param[in] bypassEnabled            - the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
* @param[in] sipLookupEnable          - GT_TRUE:  enable SIP Lookup on the port
*                                      GT_FALSE: disable SIP Lookup on the port
*                                       None.
*/
GT_VOID prvTgfBasicIpv6LpmConfigurationSet
(
    GT_PORT_NUM portNum,
    GT_BOOL     bypassEnabled,
    GT_BOOL     sipLookupEnable
)
{
    GT_STATUS   rc = GT_OK;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* keep value for restore - For SIP5 devices LPM Bypass is supported */
        if(firstCallToLpmConfig==GT_TRUE)
        {
            rc = prvTgfLpmLastLookupStagesBypassEnableGet(prvTgfDevNum,&(prvTgfRestoreCfg.defaultBypassEnable));
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLastLookupStagesBypassEnableGet: %d", prvTgfDevNum);

            rc = prvTgfLpmPortSipLookupEnableGet(prvTgfDevNum, portNum, &(prvTgfRestoreCfg.defaultSourceEportSipLookupEnable));
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmPortSipLookupEnableGet: %d", prvTgfDevNum);

            rc = prvTgfIpPortSipSaEnableGet(prvTgfDevNum,portNum, &(prvTgfRestoreCfg.defaultSourceEportSipSaEnable));
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpPortSipSaEnableGet: %d", prvTgfDevNum);

            firstCallToLpmConfig=GT_FALSE;
        }

        rc = prvTgfLpmLastLookupStagesBypassEnableSet(prvTgfDevNum,bypassEnabled);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLastLookupStagesBypassEnableSet: %d", prvTgfDevNum);

        rc = prvTgfLpmPortSipLookupEnableSet(prvTgfDevNum,portNum,sipLookupEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmPortSipLookupEnableSet: %d", prvTgfDevNum);
    }

    return;
}

/**
* @internal prvTgfBasicIpv6UcRoutingPbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
*                                       None
*/
static GT_VOID prvTgfBasicIpv6UcRoutingPbrConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_BOOL  useBulk
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPV6ADDR                             ipAddr = {{0}};
    GT_U32                                  bulkVrId[2];
    GT_IPV6ADDR                             bulkIpAddr[2];
    GT_U32                                  bulkPrefixLen[2];
    GT_BOOL                                 bulkOverride[2];
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    bulkNextHopInfo[2];
    GT_U32                                  ii;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *defUcNextHopInfoPtr = NULL;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcNextHopInfo;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. PCL Config
     */

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,
        PRV_TGF_PCL_PBR_ID_MAC(prvUtfVrfId), /* pclId */
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* enable IPv6 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* set next hop to all 0 and change only values other than 0 */
        cpssOsMemSet(&defIpv4UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* ipv4 uc and ipv6 uc default route */
        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                defUcNextHopInfoPtr = &defIpv4UcNextHopInfo;
            }
            else
            {
                defUcNextHopInfoPtr = &defIpv6UcNextHopInfo;
                defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.arpDaIndex =
                                                               prvTgfRouterArpIndex;
            }
            defUcNextHopInfoPtr->pclIpUcAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
            defUcNextHopInfoPtr->pclIpUcAction.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            defUcNextHopInfoPtr->pclIpUcAction.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
        }

        /* disable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_FALSE);

        prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId,
                                    prvUtfVrfId,
                                    &defIpv4UcNextHopInfo,
                                    &defIpv6UcNextHopInfo,
                                    NULL, NULL, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv6 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = CPSS_NET_CONTROL_E;
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = PRV_TGF_NEXTHOPE_VLANID_CNS;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = prvTgfRouterArpIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    if (useBulk == GT_FALSE)
    {
        /* fill a destination IPv6 address for the prefix */
        for (ii = 0; ii < 8; ii++)
        {
            ipAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
            ipAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
        }
        /* call CPSS function */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                        ipAddr, 128, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
    }
    else
    {
        bulkVrId[0] = prvUtfVrfId;
        bulkVrId[1] = prvUtfVrfId;
        /* fill a destination IPv6 address for the prefix */
        for (ii = 0; ii < 8; ii++)
        {
            bulkIpAddr[0].arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
            bulkIpAddr[0].arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
            bulkIpAddr[1].arIP[ii * 2]     = (GT_U8)(prvTgfPacket2Ipv6Part.dstAddr[ii] >> 8);
            bulkIpAddr[1].arIP[ii * 2 + 1] = (GT_U8) prvTgfPacket2Ipv6Part.dstAddr[ii];
        }
        cpssOsMemCpy(&bulkNextHopInfo[0], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemCpy(&bulkNextHopInfo[1], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        /* call CPSS function */
        bulkPrefixLen[0] = bulkPrefixLen[1] = 128;
        bulkOverride[0] = bulkOverride[1] = GT_TRUE;
        rc = prvTgfIpLpmIpv6UcPrefixBulkAdd(prvTgfLpmDBId, bulkVrId, bulkIpAddr, bulkPrefixLen, bulkNextHopInfo, bulkOverride, GT_FALSE, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixBulkAdd: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfBasicIpv6UcRoutingTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag
*                                       None
*/
GT_STATUS prvTgfBasicIpv6UcRoutingTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
)
{
    testEgressPortAsDsa = GT_TRUE;
    testEgressDsaType   = egressDsaType;
    return GT_OK;
}

/**
* @internal prvTgfBasicIpv6UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
*                                       None
*/
GT_VOID prvTgfBasicIpv6UcRoutingRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_BOOL  useBulk
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

            prvTgfBasicIpv6UcRoutingPbrConfigurationSet(prvUtfVrfId, useBulk);
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            prvTgfBasicIpv6UcRoutingLttRouteConfigurationSet(prvUtfVrfId, useBulk);
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* currenly due to an errata FE-
           no exception for an LPM search of more than 4B (ipv6) when working in bypass mode   */
        lpmExceptionStatusErrata = GT_TRUE;
    }
}

/**
* @internal prvTgfBasicIpv6UcRoutingRouteSipLookupConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration - SIP Lookup
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfBasicIpv6UcRoutingRouteSipLookupConfigurationSet
(
    GT_U32   prvUtfVrfId
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR ipAddr = {{0}};
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32      ii;

    /* AUTODOC: define next hop for prefix same as in prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet  */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: add IPV6 UC prefix 6545:0000:0000:0000:0000:0000:0000:3212/128  */
    for (ii = 0; ii < 8; ii++)
    {
        ipAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.srcAddr[ii] >> 8);
        ipAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.srcAddr[ii];
    }
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                    128, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);


   /* AUTODOC:  enable SIP/SA Check on eport */
    rc = prvTgfIpPortSipSaEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpPortSipSaEnableSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfBasicIpv6UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfBasicIpv6UcRoutingTrafficGenerate
(
    GT_BOOL  useSecondPkt,
    GT_BOOL  expectTraffic,
    GT_PHYSICAL_PORT_NUM           sendPortNum
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          sizeDSATag = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[5];
    GT_U8                           vfdNum = 1;
    GT_U32                          expectedTrigers = 1;
    GT_U32                          ii = 1;
    TGF_VFD_INFO_STC                *currentDsaWordsVfdPtr;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode for all ports */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    if (useSecondPkt == GT_TRUE)
    {
        partsCount = sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]);
        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfPacket2PartArray, partsCount, &packetSize);
    }
    else
    {
        partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    if (useSecondPkt == GT_TRUE)
    {
        packetInfo.partsArray = prvTgfPacket2PartArray;
    }
    else
    {
        packetInfo.partsArray = prvTgfPacketPartArray;
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    if(testEgressPortAsDsa == GT_TRUE)
    {
        sizeDSATag = (TGF_eDSA_TAG_SIZE_CNS *((testEgressDsaType == TGF_DSA_4_WORD_TYPE_E) ? 1 : 0)) +
                     (TGF_DSA_TAG_SIZE_CNS * ((testEgressDsaType == TGF_DSA_2_WORD_TYPE_E) ? 1 : 0)) -
                     TGF_VLAN_TAG_SIZE_CNS;

        prvTgfEgressPortCascadeSet(&portInterface, GT_TRUE, testEgressDsaType);
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
        {
            currentDsaWordsVfdPtr = &prvTgfBasicIpv6UcRoutingEgrCascadePort_DsaWordsVfd[0];
            if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
            {
                /* <SrcTagged> not indicate tag1 , but tag 0 only (tag 1 was recognized at ingress tag 0 not)*/
                currentDsaWordsVfdPtr->patternPtr[0] |= 1 << 5;/*bit 29 word 0*/

                /* those device not support setting the 'default port vid1' and are with value 0 */
                currentDsaWordsVfdPtr->patternPtr[2] &= 0xF0;
                currentDsaWordsVfdPtr->patternPtr[3]  = 0x00;
            }
        }
        else
        {
            currentDsaWordsVfdPtr = &prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[0];
        }

       /* update the DSA with runtime info */
        {
            GT_U8   dsaBytes[16];
            GT_U8*  dsaBytesPtr = &dsaBytes[0];
            TGF_PACKET_DSA_TAG_STC tmpDsa;
            GT_U32  ii,jj,kk;
            GT_U32  currentOffset = 0;
            GT_U32  jjMax = 4;

            for(jj = 0 ; jj < jjMax ; jj++)
            {
                kk = 0;
                for(ii=0; ii< currentDsaWordsVfdPtr[jj].cycleCount ;ii++,kk++,currentOffset++)
                {
                    dsaBytes[currentOffset] = currentDsaWordsVfdPtr[jj].patternPtr[kk];
                }
            }

            for(/*currentOffset continue*/;currentOffset < 16 ; currentOffset++)
            {
                dsaBytes[currentOffset] = 0;
            }

            rc = prvTgfTrafficGeneratorPacketDsaTagParse(dsaBytesPtr,
                &tmpDsa);
            UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse: %d",
                                      prvTgfDevNum);

            /* update the DSA tag format */
            tmpDsa.dsaInfo.forward.source.portNum = sendPortNum;
            tmpDsa.dsaInfo.forward.origSrcPhy.portNum = tmpDsa.dsaInfo.forward.source.portNum;

            tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];
            tmpDsa.dsaInfo.forward.dstEport = tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum;


            /* convert the DSA into into bytes back */
            rc = prvTgfTrafficGeneratorPacketDsaTagBuild(&tmpDsa,dsaBytesPtr);
            UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagBuild: %d",
                                      prvTgfDevNum);

            if(testEgressDsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                /* the device want bit 25 in word 3 to be set to 1 <Src Tag0 Is Outer Tag> */
                dsaBytes[(3*4)+0] |= 1 << 1;

                if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                {                    
                    /* <Skip SA lookup> bit 30 in word 2 always '1' in control learning */
                    dsaBytes[(2*4)+0] |= 1 << 6;
                }
            }

            currentOffset = 0;

            for(jj = 0 ; jj < jjMax ; jj++)
            {
                kk = 0;
                for(ii=0; ii< currentDsaWordsVfdPtr[jj].cycleCount ;ii++,kk++,currentOffset++)
                {
                    currentDsaWordsVfdPtr[jj].patternPtr[kk] = dsaBytes[currentOffset];
                }
            }

        }

        vfdArray[ii++] = currentDsaWordsVfdPtr[0];
        vfdArray[ii++] = currentDsaWordsVfdPtr[1];
        expectedTrigers = (1 << ii) - 1;
        if(testEgressDsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            vfdArray[ii++] = currentDsaWordsVfdPtr[2];
            vfdArray[ii++] = currentDsaWordsVfdPtr[3];
            expectedTrigers = (1 << ii) - 1;
        }

        vfdNum = (GT_U8)ii;
    }

    /* AUTODOC: send IPv6 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=6545:0000:0000:0000:0000:0000:0000:3212 */
    /* AUTODOC:   dstIP=1122:0000:0000:0000:0000:0000:ccdd:eeff for regular test */
    /* AUTODOC:   dstIP=1122:0000:0000:0000:0000:0000:ccdd:eeff or 2233:0000:0000:0000:0000:0000:5678:90ab for bulk test */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, sendPortNum);

    cpssOsTimerWkAfter(200);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: verify routed packet on port 3 with: */
        /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:06 */
        if(prvTgfPortsArray[portIter] == sendPortNum) 
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
         else if(portIter == PRV_TGF_NEXTHOPE_PORT_IDX_CNS) 
         {

                if(expectTraffic==GT_TRUE)
                {
                    /* packetSize is not changed */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS + sizeDSATag) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS + sizeDSATag) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                else
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
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    if(expectTraffic==GT_TRUE)
    {

        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, vfdNum, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(expectedTrigers, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

    }
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
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

    /* restore values */
    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(&portInterface, GT_FALSE, testEgressDsaType);
        testEgressPortAsDsa = GT_FALSE;
        testEgressDsaType = TGF_DSA_LAST_TYPE_E;
    }
}


/**
* @internal prvTgfBasicIpv6UcRoutingTrafficGenerate4BytesMode function
* @endinternal
*
* @brief   Generate traffic expect no traffic
*/
GT_VOID prvTgfBasicIpv6UcRoutingTrafficGenerate4BytesMode
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    TGF_VFD_INFO_STC                *currentDsaWordsVfdPtr;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    PRV_TGF_LPM_EXCEPTION_STATUS_ENT  hitException,expectedHitException;
    GT_U32                            lpmEngine;


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode for all ports */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

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

    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(&portInterface, GT_TRUE, testEgressDsaType);


        if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
        {
            currentDsaWordsVfdPtr = &prvTgfBasicIpv6UcRoutingEgrCascadePort_DsaWordsVfd[0];
            if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
            {
                /* <SrcTagged> not indicate tag1 , but tag 0 only (tag 1 was recognized at ingress tag 0 not)*/
                currentDsaWordsVfdPtr->patternPtr[0] |= 1 << 5;/*bit 29 word 0*/

                /* those device not support setting the 'default port vid1' and are with value 0 */
                currentDsaWordsVfdPtr->patternPtr[2] &= 0xF0;
                currentDsaWordsVfdPtr->patternPtr[3]  = 0x00;
            }
        }
        else
        {
            currentDsaWordsVfdPtr = &prvTgfBasicIpv6UcRoutingEgrCascadePort_eDsaWordsVfd[0];
        }

    }

    /* AUTODOC: send IPv6 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=6545:0000:0000:0000:0000:0000:0000:3212 */
    /* AUTODOC:   dstIP=1122:0000:0000:0000:0000:0000:ccdd:eeff for regular test */
    /* AUTODOC:   dstIP=1122:0000:0000:0000:0000:0000:ccdd:eeff or 2233:0000:0000:0000:0000:0000:5678:90ab for bulk test */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    cpssOsTimerWkAfter(200);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: verify routed packet on port 3 with: */
        /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:06 */
        switch (portIter) {
            case PRV_TGF_SEND_PORT_IDX_CNS:
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
                break;

            case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:
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
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    if(lpmExceptionStatusErrata == GT_TRUE)
    {
        /* no exception for an LPM search of more than 4B (ipv6) when
           working in bypass mode */
        expectedHitException = PRV_TGF_LPM_EXCEPTION_HIT_E;
    }
    else
    {
        /* check that we got an exception due to enable configuration of
           bypassing lookup stages 8-31*/

        expectedHitException = PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_0_IPV6_PACKETS_E;
        /*expectedHitException = PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_1_IPV6_PACKETS_E;*/
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        lpmEngine = 0;
    }
    else
    {
        /*calculate correct LPM engine*/
      rc = prvTgfLpmEngineGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_FALSE,&lpmEngine);
      UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfLpmEngineGet");
    }

    
    rc = prvTgfLpmExceptionStatusGet(prvTgfDevNum,lpmEngine,&hitException);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfLpmExceptionStatusGet");

    UTF_VERIFY_EQUAL2_STRING_MAC(hitException, expectedHitException,
                                 "\n   got another hitException[%d] then expectedHitException[%d]",
                                 hitException, expectedHitException);

    /* restore values */
    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(&portInterface, GT_FALSE, testEgressDsaType);
        testEgressPortAsDsa = GT_FALSE;
        testEgressDsaType = TGF_DSA_LAST_TYPE_E;
    }

     /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

}

/**
* @internal prvTgfBasicIpv6UcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useBulk                  - whether to use bulk operation to delete the prefixes
*                                       None
*/
GT_VOID prvTgfBasicIpv6UcRoutingConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_BOOL  useBulk
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR ipAddr = {{0}};
    GT_U32      bulkVrId[2];
    GT_IPV6ADDR bulkIpAddr[2];
    GT_U32      bulkPrefixLen[2];
    GT_U32      ii;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;

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

    if (useBulk == GT_FALSE)
    {
        /* fill the destination IP address for IPv6 prefix in Virtual Router */
        for (ii = 0; ii < 8; ii++)
        {
            ipAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
            ipAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
        }

        /* AUTODOC: delete the IPv6 prefix */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 128);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
    }
    else
    {
        bulkVrId[0] = prvUtfVrfId;
        bulkVrId[1] = prvUtfVrfId;
        /* fill the destination IP address for IPv6 prefix in Virtual Router */
        for (ii = 0; ii < 8; ii++)
        {
            bulkIpAddr[0].arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
            bulkIpAddr[0].arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
            bulkIpAddr[1].arIP[ii * 2]     = (GT_U8)(prvTgfPacket2Ipv6Part.dstAddr[ii] >> 8);
            bulkIpAddr[1].arIP[ii * 2 + 1] = (GT_U8) prvTgfPacket2Ipv6Part.dstAddr[ii];
        }
        bulkPrefixLen[0] = bulkPrefixLen[1] = 128;

        /* AUTODOC: delete bulk of 2 IPv6 prefixes */
        rc = prvTgfIpLpmIpv6UcPrefixBulkDel(prvTgfLpmDBId, bulkVrId, bulkIpAddr, bulkPrefixLen, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixBulkDel: %d", prvTgfDevNum);
    }

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {

        /* AUTODOC: disable Unicast IPv6 Routing on port 0 */
        rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* AUTODOC: disable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* restore port inlif mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,prvTgfRestoreCfg.sendPortInlifMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);

    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterDel: %d",
                                     prvTgfDevNum);
    }

    /* disable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /*For SIP5 devices LPM Bypass is supported */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
       if(firstCallToLpmConfig==GT_FALSE)
       {
           rc = prvTgfLpmLastLookupStagesBypassEnableSet(prvTgfDevNum,prvTgfRestoreCfg.defaultBypassEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLastLookupStagesBypassEnableSet: %d", prvTgfDevNum);

           rc = prvTgfLpmPortSipLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.defaultSourceEportSipLookupEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmPortSipLookupEnableSet: %d", prvTgfDevNum);

           rc = prvTgfIpPortSipSaEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.defaultSourceEportSipSaEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpPortSipSaEnableSet: %d", prvTgfDevNum);

           firstCallToLpmConfig=GT_TRUE;
       }
    }

}




/**
* @internal prvTgfIpv6UcRollBackCheck function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
GT_VOID   prvTgfIpv6UcRollBackCheck(GT_VOID)
{
    GT_STATUS   rc        = GT_OK;
    GT_IPV6ADDR   ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32   prvUtfVrfId = 0;
    GT_U32   prvTgfLpmDBId = 0;
    GT_U32   prefixLength = 0;
    GT_U32   seed = 0;
    GT_U32   maxHwWriteNumber = 0;
    GT_U32   maxMallocNumber = 0;
    GT_U32   failedSeqNumber = 0;
    GT_U32   i = 0;
    /* get random seed */
    seed = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(seed);
    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 19;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    ipAddr.arIP[0] = 0x11;
    ipAddr.arIP[1] = 0x22;
    for (i = 2; i < 16; i++)
    {
        ipAddr.arIP[i] = cpssOsRand()% 255;
    }
    prvUtfVrfId = 0;
    prvTgfLpmDBId = 0;

    while(0 == prefixLength) /* the prefix len should not be zero */
    {
        prefixLength = cpssOsRand()% 128;
    }

    /* enable calculation of hw writes number */
    rc = appDemoHwAccessFailerBind(GT_TRUE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    /* enable calculation of malloc allocations */
    rc = osDbgMallocFailSet(GT_TRUE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);

    /* don't make validation function after prefix insertion */
    prvTgfIpValidityCheckEnable(GT_FALSE);
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                    prefixLength, &nextHopInfo, GT_TRUE, GT_FALSE);
    prvTgfIpValidityCheckEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

    /* get hw writes number during add prefix operation */
    rc = appDemoHwAccessCounterGet(&maxHwWriteNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessCounterGet: %d", prvTgfDevNum);
    /* get number of malloca allocations during add prefix operation*/
    rc = osMemGetMallocAllocationCounter(&maxMallocNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osMemGetMallocAllocationCounter: %d", prvTgfDevNum);

    rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, prefixLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel: %d", prvTgfDevNum);

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
    rc = appDemoHwAccessFailerBind(GT_TRUE,failedSeqNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    /* This operation is going to fail */
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                    prefixLength, &nextHopInfo, GT_TRUE, GT_FALSE);

    /* deactivate hw fail mechanism */
    rc = appDemoHwAccessFailerBind(GT_FALSE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);


    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId,prvUtfVrfId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    /* generate traffic for previous prefix:*/
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

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
    rc = osDbgMallocFailSet(GT_TRUE,failedSeqNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);
    /* This operation is going to fail */
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                    prefixLength, &nextHopInfo, GT_TRUE, GT_FALSE);
    /* deactivate allocation fail mechanism */
    rc = osDbgMallocFailSet(GT_FALSE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);

    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId,prvUtfVrfId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    /* generate traffic for previous prefix:*/
    prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

}


