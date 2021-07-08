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
* @file prvTgfBasicIpv6McRouting.c
*
* @brief Basic IPV6 MC Routing
*
* @version   20
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfBasicIpv6McRouting.h>
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

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x33, 0x33, 0x00, 0x00, 0x22, 0x22},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
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

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U16                               vid;
    CPSS_PORTS_BMP_STC                   defPortMembers;
    GT_IPV6ADDR                          prefix;
    GT_U32                               prefixLen;
    CPSS_IPV6_PREFIX_SCOPE_ENT           addressScope;
    OUT CPSS_PACKET_CMD_ENT              scopeCommand;
    OUT CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv6McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS           rc = GT_OK;
    GT_U8               tagArray[]  = {1, 1, 1, 1};
    CPSS_PORTS_BMP_STC  portMembers = {{0, 0}};

    /* AUTODOC: SETUP CONFIGURATION: */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portMembers);

    /* AUTODOC: create VLAN 80 with untagged port 2 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray + 2, NULL, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

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

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID 80 for port 2 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* save default port members for MC group */
    rc = prvTgfBrgMcEntryRead(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                              &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryRead: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* create bmp with ports for MC group */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS]);

    /* write MC Group entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS, &portMembers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* create a static macEntry in SEND VLAN */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_SEND_VLANID_CNS,
                                          PRV_TGF_DEF_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

/**
* @internal prvTgfBasicIpv6McRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfBasicIpv6McRoutingLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_IPV6ADDR                     ipSrc = {{0}};
    GT_IPV6ADDR                     ipGrp = {{0}};
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC              *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;
    GT_U32                          ii;
    CPSS_IP_PROTOCOL_STACK_ENT     protocol = CPSS_IP_PROTOCOL_IPV6_E;

    /* AUTODOC: enable MC IPv6 Routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv6 MC Routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
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
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex + 1;

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
    for (ii = 0; ii < 8; ii++)
    {
        ipGrp.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
        ipGrp.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
        ipSrc.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.srcAddr[ii] >> 8);
        ipSrc.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.srcAddr[ii];
    }

    /* AUTODOC: add IPv6 MC prefix: */
    /* AUTODOC:   ipGrp=ff00:0000:0000:0000:0000:0000:0000:2222/128 */
    /* AUTODOC:   ipSrc=2222:0000:0000:0000:0000:0000:0000:2222/128 */
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 128,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv6McEntryAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv6McRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingRouteConfigurationSet
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

            prvTgfBasicIpv6McRoutingLttRouteConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}

/**
* @internal prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSet function
* @endinternal
*
* @brief   Set Scope Checking Configuration
*
* @param[in] ipv6SourceSiteIdMode     - IPv6 source Site Id Mode
* @param[in] ipv6DestSiteIdMode       - IPv6 destination Site Id Mode
* @param[in] ipv6MCGroupScopeLevel    - IPv6 Multicast group scope level
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
* @param[in] scopeCommand             - action to be done on a packet that match the above
*                                      scope configuration.
*                                       None
*/
GT_VOID prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSet
(
    CPSS_IP_SITE_ID_ENT         ipv6SourceSiteIdMode,
    CPSS_IP_SITE_ID_ENT         ipv6DestSiteIdMode,
    CPSS_IPV6_PREFIX_SCOPE_ENT  ipv6MCGroupScopeLevel,
    GT_BOOL                     borderCrossed,
    CPSS_PACKET_CMD_ENT         scopeCommand
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_BOOL                             isValid;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC       mcRouteEntryArray[1];
    PRV_TGF_IP_LTT_ENTRY_STC            mcRouteLttEntry;
    GT_IPV6ADDR                         ipSrc = {{0}};
    GT_IPV6ADDR                         ipGrp = {{0}};
    GT_U8                               ii;

    /* AUTODOC: IPV6 MC SCOPE CHECKING SETUP CONFIGURATION: */

    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum,
                                PRV_TGF_SEND_VLANID_CNS,
                                &portsMembers,
                                &portsTagging,
                                &isValid,
                                &vlanInfo,
                                &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead");

    /*  The source Site ID assigned to the packet is derived from its eVLAN assignment.
        The eVLAN is configured with a <Site ID> that reflects whether the eVLAN interface
        is “Internal” or “External” to the administrative site */
    vlanInfo.ipv6SiteIdMode = ipv6SourceSiteIdMode;

    /* AUTODOC: Configure Source Site ID INTERNAL */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 PRV_TGF_SEND_VLANID_CNS,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite");

    /* AUTODOC: create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex + 1;

    /* IPv6 Scope checking can be enabled/disabled on a per-Route entry basis */
    mcRouteEntryArray[0].scopeCheckingEnable  = GT_TRUE;

    /*  The destination Site ID is assigned to the packet based on the route entry associated with its DIP.
        The Route entry matching the DIP lookup is configured with a <Site ID> that reflects whether the DIP
        is “Internal” or “External” to the administrative site */
    mcRouteEntryArray[0].siteId               = ipv6DestSiteIdMode;

    /* AUTODOC: add MC route entry with index 10, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex,
                                     1,
                                     CPSS_IP_PROTOCOL_IPV6_E,
                                     mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* IPv6 Multicast Scope Level assignment */
    mcRouteLttEntry.ipv6MCGroupScopeLevel = ipv6MCGroupScopeLevel;

    /* fill a destination IP address for the LPM */
    for (ii = 0; ii < 8; ii++)
    {
        ipGrp.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
        ipGrp.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
        ipSrc.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.srcAddr[ii] >> 8);
        ipSrc.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.srcAddr[ii];
    }

    /* AUTODOC: add IPv6 MC prefix: */
    /* AUTODOC:   ipGrp=ff00:0000:0000:0000:0000:0000:0000:2222/128 */
    /* AUTODOC:   ipSrc=2222:0000:0000:0000:0000:0000:0000:2222/128 */
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 128,
                                   &mcRouteLttEntry, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv6McEntryAdd: %d", prvTgfDevNum);

    /* AUTODOC: add IPv6 MC prefix of a scope type GLOBAL */
    rc = prvTgfIpv6AddrPrefixScopeSet(prvTgfDevNum, ipSrc, 16, CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv6AddrPrefixScopeSet");

    /* AUTODOC: set the ipv6 Multicast scope commands */
    rc = prvTgfIpv6McScopeCommandSet(prvTgfDevNum, CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E, CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E,
                                     borderCrossed, scopeCommand, CPSS_IPV6_MLL_SELECTION_RULE_GLOBAL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv6McScopeCommandSet");

    return;

}



/**
* @internal prvTgfBasicIpv6McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectNoTraffic          - whether to expect no traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv6McRoutingTrafficGenerate
(
    GT_BOOL  expectNoTraffic
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

    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    /* set MLL counters mode for MC subscriber 1 */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV6_E;
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

    /* AUTODOC: send IPv6 packet from port 2 with: */
    /* AUTODOC:   DA=33:33:cc:dd:ee:ff, SA=00:00:00:00:00:33, VID=5 */
    /* AUTODOC:   dstIp=ff00:0000:0000:0000:0000:0000:0000:2222/128 */
    /* AUTODOC:   srcIp=2222:0000:0000:0000:0000:0000:0000:2222/128 */
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

    if (expectNoTraffic == GT_TRUE)
    {
        /* AUTODOC: verify routed packet on ports 0,1,3 */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS],
                0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS],
                0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS],
                0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        return;
    }

    /* AUTODOC: verify routed packet on ports 0,1,3 */
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
* @internal prvTgfBasicIpv6McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingConfigurationRestore
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

    /* AUTODOC: flush all MC Ipv6 prefix */
    rc = prvTgfIpLpmIpv6McEntriesFlush(prvTgfLpmDBId, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntriesFlush");

    /* AUTODOC: disable MC IPv6 routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable MC IPv6 routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
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

    /* restore MC Group entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                               &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

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
* @internal prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSave function
* @endinternal
*
* @brief   Save Scope Checking configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSave
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: SAVE SCOPE CHECKING CONFIGURATION: */
    /* ------------------------------------------------------------------------- */

    /* AUTODOC: save IPv6 MC prefix of a scope type GLOBAL */
    rc = prvTgfIpv6AddrPrefixScopeGet(prvTgfDevNum, &prvTgfRestoreCfg.prefix, &prvTgfRestoreCfg.prefixLen, &prvTgfRestoreCfg.addressScope, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv6AddrPrefixScopeGet");

    /* AUTODOC: save the ipv6 Multicast scope commands */
    rc = prvTgfIpv6McScopeCommandGet(prvTgfDevNum, CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E, CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E,
                                     GT_TRUE, &prvTgfRestoreCfg.scopeCommand, &prvTgfRestoreCfg.mllSelectionRule);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv6McScopeCommandGet");

    return;
}

/**
* @internal prvTgfBasicIpv6McRoutingScopeCheckingConfigurationRestore function
* @endinternal
*
* @brief   Restore Scope Checking configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingScopeCheckingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE SCOPE CHECKING CONFIGURATION: */
    /* ------------------------------------------------------------------------- */

    /* AUTODOC: restore IPv6 MC prefix of a scope type GLOBAL */
    rc = prvTgfIpv6AddrPrefixScopeSet(prvTgfDevNum, prvTgfRestoreCfg.prefix, prvTgfRestoreCfg.prefixLen, prvTgfRestoreCfg.addressScope, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv6AddrPrefixScopeSet");

    /* AUTODOC: restore the ipv6 Multicast scope commands */
    rc = prvTgfIpv6McScopeCommandSet(prvTgfDevNum, CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E, CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E,
                                     GT_TRUE, prvTgfRestoreCfg.scopeCommand, prvTgfRestoreCfg.mllSelectionRule);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv6McScopeCommandSet");

    return;
}

/**
* @internal prvTgfIpv6McHwAccessAndMallocNumberCalculation function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*
* @param[out] hwWriteNumberPtr         - number of hw write accesses for given ipv6 mc prefix.
* @param[out] mallocNumberPtr          - number of malloc allocations for given ipv6 mc prefix.
*                                       None
*/
static GT_VOID  prvTgfIpv6McHwAccessAndMallocNumberCalculation
(
    IN GT_IPV6ADDR                  ipGrp,
    IN GT_IPV6ADDR                  ipSrc,
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

    /* AUTODOC: add IPv6 MC prefix: */
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, ipSrcPrefixLen,
                                   mcRouteLttEntryPtr, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: %d", prvTgfDevNum);

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
    rc =  prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId,0,ipGrp,128,ipSrc,ipSrcPrefixLen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryDel: %d", prvTgfDevNum);
}


/**
* @internal prvTgfIpv6McAddPrefixAndValidateRollback function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
static GT_VOID prvTgfIpv6McAddPrefixAndValidateRollback
(
    IN GT_IPV6ADDR                  ipGrp,
    IN GT_IPV6ADDR                  ipSrc,
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
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, srcPrefixLen,
                                   mcRouteLttEntryPtr, GT_TRUE, GT_FALSE);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: %d", prvTgfDevNum);

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

    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    /* generate traffic for previous prefix:*/
     prvTgfBasicIpv6McRoutingTrafficGenerate(PRV_TGF_DEFAULT_BOOLEAN_CNS);
}


/**
* @internal prvTgfIpv6McRollBackCheck function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
GT_VOID  prvTgfIpv6McRollBackCheck(GT_VOID)
{
    GT_IPV6ADDR         ipSrc = {{0}};
    GT_IPV6ADDR         ipGrp = {{0}};
    GT_U32    srcPrefixLength = 0;
    GT_U32    seed = 0;
    GT_U32    maxHwWriteNumber = 0;
    GT_U32    maxMallocNumber = 0;
    GT_U32    failedSeqNumber = 0;
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;
    GT_U32    i = 0;

    /* get random seed */
    seed = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(seed);
    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = 19;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the LPM */
    ipGrp.arIP[0] = 0xff;
    ipGrp.arIP[1] = 0x0;
    ipGrp.arIP[2] = 0x0;
    ipGrp.arIP[3] = 0x0;
    ipGrp.arIP[4] = 0x0;
    ipGrp.arIP[5] = 0x0;
    ipGrp.arIP[6] = 0x0;
    for (i = 7; i < 16; i++)
    {
        ipGrp.arIP[i] = cpssOsRand()% 255;
    }
    for (i = 0; i < 16; i++)
    {
         ipSrc.arIP[i] = cpssOsRand()% 255;
    }
    while(0 == srcPrefixLength) /* the prefix len should not be zero */
    {
        srcPrefixLength = cpssOsRand()% 128;
    }

    /* calculate number of hw write and malloc operations for given ipv6 mc prefix */
    prvTgfIpv6McHwAccessAndMallocNumberCalculation(ipGrp,ipSrc,srcPrefixLength,
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
    prvTgfIpv6McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
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
    prvTgfIpv6McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
                                             failedSeqNumber,GT_FALSE);
    /* fill a destination IP address for the LPM */
    ipGrp.arIP[0] = 0xff;
    ipGrp.arIP[1] = 0x77;
    for (i = 2; i < 16; i++)
    {
        ipGrp.arIP[i] = cpssOsRand()% 255;
    }

    /* calculate number of hw write and malloc operations for given ipv6 mc prefix */
    prvTgfIpv6McHwAccessAndMallocNumberCalculation(ipGrp,ipSrc,srcPrefixLength,
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
    prvTgfIpv6McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
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
    prvTgfIpv6McAddPrefixAndValidateRollback(ipGrp,ipSrc,srcPrefixLength,&mcRouteLttEntry,
                                             failedSeqNumber,GT_FALSE);
}


