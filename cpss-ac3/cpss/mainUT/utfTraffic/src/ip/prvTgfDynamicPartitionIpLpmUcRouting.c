/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfDynamicPartitionIpUcRouting.c
*
* DESCRIPTION:
*       IP LPM Engine
*
* FILE REVISION NUMBER:
*       $Revision: 16 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>



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
#define PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS  2

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS  3

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* number of loops done in random tests */
#define PRV_TGF_LOOPS_NUM  3000

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS
};


/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 19;

/* virtual router id */
static GT_U32        vrId                      = 0;

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
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypeIpv4Part = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypeIpv6Part = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

static TGF_IPV4_ADDR  dstAddr = {1,1,1,1};
static TGF_IPV4_ADDR  defIpv4DstAddr = {1,1,1,8};

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
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  8}    /* dstAddr */
};


static TGF_IPV6_ADDR  defIpv6DstAddr = {0x2001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1234};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                                                            /* version */
    0,                                                            /* trafficClass */
    0,                                                            /* flowLabel */
    0x1a,                                                         /* payloadLen */
    0x3b,                                                         /* nextHeader */
    0x40,                                                         /* hopLimit */
    { 0x1111,0x1111,0x1111,0x1111,0x1111,0x1111,0x1111,0x1111},   /* srcAddr */
    { 0x2001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1234}    /* dstAddr */
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
static TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypeIpv4Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv6PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypeIpv6Part},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* default LPM DB and Virtual Router configuration */
static struct
{
    GT_BOOL                                     isSupportIpv4Uc;
    GT_BOOL                                     isSupportIpv6Uc;
    GT_BOOL                                     isSupportIpv4Mc;
    GT_BOOL                                     isSupportIpv6Mc;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv6McRouteLttEntry;
    PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    GT_BOOL                                     partitionEnable;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmManagerCapcityCfg;
    GT_BOOL                                     isTcamManagerInternal;
} prvTgfDefLpmDBVrCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfDynamicPartitionIpLpmDbConfigurationSet function
* @endinternal
*
* @brief   Set IP LPM DB Base Configuration
*
* @param[in] lastLineIndex            - index of the last line in the TCAM,
*                                      used to create the new LPP DB
*                                       None
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbConfigurationSet
(
    GT_U32 lastLineIndex
)
{
    GT_STATUS                                    st = GT_OK;
    GT_U8                                        devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                       numOfDevs = 0;
    GT_U32                                       vrId;
    PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT              shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC   indexesRange;
    GT_BOOL                                      partitionEnable ;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC   tcamLpmManagerCapcityCfg;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv6McRouteLttEntry;

    /* cpssDxChIpLpmDBCreate with lpmDbId[19], partitionEnable= FALSE,
       (lastLineIndex+1) TCAM lines ((lastLineIndex+1)*4 tcam cells)
       and other valid parameters */
    vrId                                                = 0;
    protocolStack                                       = CPSS_IP_PROTOCOL_IPV4V6_E;
    indexesRange.firstIndex                             = 0;
    indexesRange.lastIndex                              = lastLineIndex;
    partitionEnable                                     = GT_FALSE;
    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 0;
    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 0;
    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 0;

    shadowType = PRV_TGF_IP_TCAM_ROUTER_BASED_SHADOW_E;

    st = prvTgfIpLpmDBCreate(prvTgfLpmDBId,shadowType,protocolStack,&indexesRange,partitionEnable,&tcamLpmManagerCapcityCfg,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate: %d", prvTgfLpmDBId);

    devList[numOfDevs++] = prvTgfDevNum;

    /* Call cpssDxChIpLpmDBDevListAdd for lpmDbId[19] */
    st = prvTgfIpLpmDBDevListAdd(prvTgfLpmDBId, devList, numOfDevs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", prvTgfLpmDBId, numOfDevs);

    defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defIpv4UcNextHopInfo.ipLttEntry.numOfPaths               = 0;
    defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex      = 0;
    defIpv4UcNextHopInfo.ipLttEntry.routeType                = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;

    defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defIpv6UcNextHopInfo.ipLttEntry.numOfPaths               = 0;
    defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex      = 0;
    defIpv6UcNextHopInfo.ipLttEntry.routeType                =  PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;

    defIpv4McRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    defIpv4McRouteLttEntry.numOfPaths = 0;
    defIpv4McRouteLttEntry.routeEntryBaseIndex = 1;
    defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    defIpv6McRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    defIpv6McRouteLttEntry.numOfPaths = 0;
    defIpv6McRouteLttEntry.routeEntryBaseIndex = 1;
    defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    /* Call cpssDxChIpLpmVirtualRouterAdd for lpmDbId[19],vrId[0] */
    st = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, vrId, &defIpv4UcNextHopInfo,&defIpv6UcNextHopInfo,&defIpv4McRouteLttEntry,&defIpv6McRouteLttEntry,NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"VirtualRouterAdd: lpmDBId = %d, vrId = %d", prvTgfLpmDBId, vrId);
}

/**
* @internal prvTgfDynamicPartitionIpLpmDbConfigurationRestore function
* @endinternal
*
* @brief   Restore IP LPM DB Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                                    st = GT_OK;
    GT_U32                                       vrId=0;

    /* restore configuration */

    /* Delete virtual router vrId[0] */
    st = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",prvTgfLpmDBId, vrId);

    /* Delete lpmDBId[19] */
    st = cpssDxChIpLpmDBDelete(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, prvTgfLpmDBId);

}

/*================================================================================*/
/*====================tgfDynamicPartitionIpLpmPushUpUcRouting=====================*/
/*================================================================================*/

/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcBaseConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                     isTagged  = GT_TRUE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

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
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter], isTagged);
    }

    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);
}



/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipv4Addr = {0};
    GT_IPV6ADDR                             ipv6Addr = {{0}};
    GT_U8                                   i;
    GT_U32                                  prefixLen;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
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
    inlifEntry.ipv6UcRouteEnable = GT_TRUE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;
    inlifEntry.vrfId             = 0;

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

    /* call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, 4096, &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4\IPv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4\IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    for (i = 1; i <= 12; i++)
    {
        /* write a ARP MAC address to the Router ARP Table */
        arpMacAddr.arEther[0] = 0;
        arpMacAddr.arEther[1] = 0;
        arpMacAddr.arEther[2] = 0;
        arpMacAddr.arEther[3] = 0;
        arpMacAddr.arEther[4] = 0x22;
        arpMacAddr.arEther[5] = i+10;

        /* AUTODOC: write 12 ARP MACs 00:00:00:00:22:[0xb..0x16] to ARP Table 1..12 */
        rc = prvTgfIpRouterArpAddrWrite(i, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* write a UC Route entry to the Route Table */
        cpssOsMemSet(&routeEntriesArray, 0, sizeof(routeEntriesArray));
        routeEntriesArray.cmd                        = CPSS_PACKET_CMD_ROUTE_E;
        routeEntriesArray.cpuCodeIndex               = 0;
        routeEntriesArray.appSpecificCpuCodeEnable   = GT_FALSE;
        routeEntriesArray.unicastPacketSipFilterEnable = GT_FALSE;
        routeEntriesArray.ttlHopLimitDecEnable       = GT_FALSE;
        routeEntriesArray.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
        routeEntriesArray.ingressMirror              = GT_FALSE;
        routeEntriesArray.qosProfileMarkingEnable    = GT_FALSE;
        routeEntriesArray.qosProfileIndex            = 0;
        routeEntriesArray.qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        routeEntriesArray.modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.countSet                   = PRV_TGF_COUNT_SET_CNS;
        routeEntriesArray.trapMirrorArpBcEnable      = GT_FALSE;
        routeEntriesArray.sipAccessLevel             = 0;
        routeEntriesArray.dipAccessLevel             = 0;
        routeEntriesArray.ICMPRedirectEnable         = GT_FALSE;
        routeEntriesArray.scopeCheckingEnable        = GT_FALSE;
        routeEntriesArray.siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntriesArray.mtuProfileIndex            = 0;
        routeEntriesArray.isTunnelStart              = GT_FALSE;
        routeEntriesArray.nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
        if (i < 11)
        {
            routeEntriesArray.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
        }
        else
        {
            routeEntriesArray.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS];
        }
        routeEntriesArray.nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
        routeEntriesArray.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
        routeEntriesArray.nextHopARPPointer          = i;
        routeEntriesArray.nextHopTunnelPointer       = 0;

        /* AUTODOC: add 12 UC route entries with: */
        /* AUTODOC:   cmd=CMD_ROUTE, nextHopVlan=6 */
        /* AUTODOC:   11..20 nextHopPorts=3, 21,22 nextHopPorts=2 */
        rc = prvTgfIpUcRouteEntriesWrite(i+10, &routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


        /* AUTODOC: read and check the UC Route entry from the Route Table */
        cpssOsMemSet(&routeEntriesArray, 0, sizeof(routeEntriesArray));

        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, i+10, &routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

        PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                         routeEntriesArray.nextHopVlanId,
                         routeEntriesArray.nextHopInterface.devPort.portNum);
    }
    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv6 prefixes in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 21;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    prefixLen = 16;

    for (i = 0; i < 8; i++)
    {
        ipv6Addr.arIP[2 * i]       = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] >> 8);
        ipv6Addr.arIP[(2 * i) + 1] = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] & 0xFF);
    }

    /* AUTODOC: add IPv6 UC prefix 2001:0000:0000:0000:0000:0000:0000:1234/16, routeIdx=11 */
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,
                                    vrId,
                                    ipv6Addr,
                                    prefixLen,
                                    &nextHopInfo,
                                    GT_FALSE,
                                    GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
    prefixLen = 8;
    /* call CPSS function */

    /* AUTODOC: add IPv6 UC prefix 2001:0000:0000:0000:0000:0000:0000:1234/8, routeIdx=12 */
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,
                                    vrId,
                                    ipv6Addr,
                                    prefixLen,
                                    &nextHopInfo,
                                    GT_FALSE,
                                    GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */
    prefixLen = 32;
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipv4Addr.arIP, dstAddr, sizeof(ipv4Addr.arIP));

    for (i = 1; i < 11; i++)
    {
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = i+10;
        if (i < 10)
        {
            prefixLen = prefixLen -2;
        }
        else
        {
            prefixLen = 32;
        }

        /* AUTODOC: add 9 IPv4 UC prefixes 1.1.1.1/30, routeIdx=[1..9] */
        /* AUTODOC: add 1 IPv4 UC prefix 1.1.1.1/32, routeIdx=10 */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId,
                                        vrId,
                                        ipv4Addr,
                                        prefixLen,
                                        &nextHopInfo,
                                        GT_FALSE,
                                        GT_FALSE);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    }

    /* -------------------------------------------------------------------------
     * 5. Set the FDB entry With DA_ROUTE
     */

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
* @internal prvTgfIpUcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isIpv4Traffic            -     GT_TRUE:  generate ipv4 traffic,
*                                      GT_FALSE: generate ipv6 traffic.
* @param[in] ipAddrPtr                -     (pointer to) the IP address in case we want to send
*                                      traffic different from the default
*                                       None
*/
GT_VOID prvTgfIpUcRoutingTrafficGenerate
(
   IN GT_BOOL isIpv4Traffic,
   IN GT_IP_ADDR_TYPE_UNT *ipAddrPtr
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
    TGF_PACKET_PART_STC             *prvTgfPacketPartArrayPtr;
    GT_U32                           outputCapturePort;
    GT_U8                           i=0;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                               tcamRowIndex;
    GT_U32                               tcamColumnIndex;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear capturing table */
    tgfTrafficTableRxPcktTblClear();

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    if (isIpv4Traffic == GT_TRUE)
    {
        if (ipAddrPtr!=NULL)
        {
             /* fill destination IP address for packet */
            cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, ipAddrPtr->ipv4Addr.arIP, sizeof(prvTgfPacketIpv4Part.dstAddr));
        }
        outputCapturePort = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
    }
    else
    {
        if (ipAddrPtr!=NULL)
        {
            /* fill destination IP address for packet */
            for (i = 0; i < 8; i++)
            {
                prvTgfPacketIpv6Part.dstAddr[i] = (ipAddrPtr->ipv6Addr.arIP[i * 2 +1]);
                prvTgfPacketIpv6Part.dstAddr[i] |= ((GT_U16)(ipAddrPtr->ipv6Addr.arIP[i * 2]))<<8;
            }
        }
        outputCapturePort = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS];
    }
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

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = outputCapturePort;

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    if (isIpv4Traffic == GT_TRUE)
    {
        partsCount = sizeof(prvTgfPacketIpv4PartArray) / sizeof(prvTgfPacketIpv4PartArray[0]);
        prvTgfPacketPartArrayPtr = prvTgfPacketIpv4PartArray;
    }
    else
    {
        partsCount = sizeof(prvTgfPacketIpv6PartArray) / sizeof(prvTgfPacketIpv6PartArray[0]);
        prvTgfPacketPartArrayPtr = prvTgfPacketIpv6PartArray;

    }

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArrayPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArrayPtr;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.8 */
    /* AUTODOC: send IPv6 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1111:1111:1111:1111:1111:1111:1111:1111 */
    /* AUTODOC:   dstIP=2001:0000:0000:0000:0000:0000:0000:1234 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify IPv4 routed packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:22:02, SA=00:00:00:00:00:06, VID=6 */
    /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.8 */
    /* AUTODOC: verify IPv6 routed packet on port 2 with: */
    /* AUTODOC:   DA=00:00:00:00:22:0b, SA=00:00:00:00:00:06, VID=6 */
    /* AUTODOC:   srcIP=1111:1111:1111:1111:1111:1111:1111:1111 */
    /* AUTODOC:   dstIP=2001:0000:0000:0000:0000:0000:0000:1234 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter)
        {
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
        case PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS:
            if (isIpv4Traffic == GT_TRUE)
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
            else
            {
                cpssOsMemSet(&expectedCntrs, 0 , sizeof(CPSS_PORT_MAC_COUNTER_SET_STC));
            }
            break;
        case PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS:
            if (isIpv4Traffic == GT_FALSE)
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
            else
            {
                cpssOsMemSet(&expectedCntrs, 0 , sizeof(CPSS_PORT_MAC_COUNTER_SET_STC));
            }
            break;
        default:
            /* for other ports */
            cpssOsMemSet(&expectedCntrs, 0 , sizeof(CPSS_PORT_MAC_COUNTER_SET_STC));
            break;
        }
        PRV_TGF_VERIFY_COUNTERS_MAC(isOk, expectedCntrs, portCntrs);

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isOk, expectedCntrs, portCntrs);

    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    vfdArray[0].patternPtr[0] = 0;
    vfdArray[0].patternPtr[1] = 0;
    vfdArray[0].patternPtr[2] = 0;
    vfdArray[0].patternPtr[3] = 0;
    vfdArray[0].patternPtr[4] = 0x22;

    if (isIpv4Traffic == GT_TRUE)
    {
        if (ipAddrPtr!=NULL)
        {
            rc = prvTgfIpLpmIpv4UcPrefixSearch(prvTgfLpmDBId, vrId,ipAddrPtr->ipv4Addr,32,&nextHopInfo,&tcamRowIndex,&tcamColumnIndex);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfIpLpmIpv4UcPrefixGet\n");
            vfdArray[0].patternPtr[5] = (GT_U8)nextHopInfo.ipLttEntry.routeEntryBaseIndex;
        }
        else
        {
            vfdArray[0].patternPtr[5] = 0xc;
        }
    }
    else
    {
        if (ipAddrPtr!=NULL)
        {
            rc = prvTgfIpLpmIpv6UcPrefixSearch(prvTgfLpmDBId, vrId,ipAddrPtr->ipv6Addr,128,&nextHopInfo,&tcamRowIndex,&tcamColumnIndex);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfIpLpmIpv6UcPrefixGet\n");
            vfdArray[0].patternPtr[5] = (GT_U8)nextHopInfo.ipLttEntry.routeEntryBaseIndex;
        }
        else
        {
            vfdArray[0].patternPtr[5] = 0x15;
        }
    }


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
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);
}


/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcRoutingTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfIpUcRoutingTrafficGenerate(GT_TRUE,NULL);/* ipv4 traffic */
    prvTgfIpUcRoutingTrafficGenerate(GT_FALSE,NULL);/* ipv6 traffic */
}

/**
* @internal prvTgfDynamicPartitionIpLpmPushUpUcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushUpUcRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipv4Addr  = {0};
    GT_IPV6ADDR ipv6Addr  = {{0}};
    GT_U32      prefixLen;
    GT_U8       i;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* retsore ipv4 route configuration */

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipv4Addr.arIP, dstAddr, sizeof(ipv4Addr.arIP));
    prefixLen = 34;
    for (i = 1; i <= 10; i++)
    {
        prefixLen = prefixLen - 2;

        /* AUTODOC: delete the IPv4 prefixes */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, prefixLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    }

    /* restore IPv6 route configuration */
    /* fill a destination IP address for the prefix */
    for (i = 0; i < 8; i++)
    {
        ipv6Addr.arIP[2 * i]       = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] >> 8);
        ipv6Addr.arIP[(2 * i) + 1] = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] & 0xFF);
    }

    prefixLen = 24;
    for (i = 11; i <= 12; i++)
    {
        prefixLen = prefixLen - 8;

        /* AUTODOC: delete the IPv6 prefixes */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, ipv6Addr, prefixLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel: %d", prvTgfDevNum);

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

    /* AUTODOC: invalidate vlan entries [5,6] */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }
    /* clear capturing table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d",
                                 prvTgfDevNum);

}



/*================================================================================*/
/*====================tgfDynamicPartitionIpLpmPushDownUcRouting===================*/
/*================================================================================*/

/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcBaseConfigurationSet
(
    GT_VOID
)
{
    prvTgfDynamicPartitionIpLpmPushUpUcBaseConfigurationSet();

}

/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipv4Addr;
    GT_U32                                  prefixLen;

    /* set the same configuration as in push up test */
    prvTgfDynamicPartitionIpLpmPushUpUcRouteConfigurationSet();

    /* delete ipv4 prefix 1.1.1.1/32*/
    prefixLen = 32;
    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipv4Addr.arIP, dstAddr, sizeof(ipv4Addr.arIP));

    /* AUTODOC: delete IPv4 prefix 1.1.1.1/32 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, prefixLen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* add ipv4 prefix 1.1.1.1/12 */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 20;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    prefixLen = 12;

    /* AUTODOC: add IPv4 prefix 1.1.1.1/12, routeIdx=20 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId,
                                    vrId,
                                    ipv4Addr,
                                    prefixLen,
                                    &nextHopInfo,
                                    GT_FALSE,
                                    GT_FALSE);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

}


/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcRoutingTrafficGenerate
(
    GT_VOID
)
{
   prvTgfDynamicPartitionIpLpmPushUpUcRoutingTrafficGenerate();
}


/**
* @internal prvTgfDynamicPartitionIpLpmPushDownUcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDynamicPartitionIpLpmPushDownUcRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipv4Addr  = {0};
    GT_IPV6ADDR ipv6Addr  = {{0}};
    GT_U32      prefixLen;
    GT_U8       i;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* retsore ipv4 route configuration */

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipv4Addr.arIP, dstAddr, sizeof(ipv4Addr.arIP));
    prefixLen = 32;
    for (i = 1; i <= 10; i++)
    {
        prefixLen = prefixLen - 2;

        /* AUTODOC: delete the IPv4 prefixes */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, prefixLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    }

    /* restore IPv6 route configuration */
    /* fill a destination IP address for the prefix */
    for (i = 0; i < 8; i++)
    {
        ipv6Addr.arIP[2 * i]       = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] >> 8);
        ipv6Addr.arIP[(2 * i) + 1] = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] & 0xFF);
    }

    prefixLen = 24;
    for (i = 11; i <= 12; i++)
    {
        prefixLen = prefixLen - 8;

        /* AUTODOC: delete the IPv6 prefixes */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, ipv6Addr, prefixLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel: %d", prvTgfDevNum);

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

    /* AUTODOC: invalidate vlan entries [5,6] */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }
    /* clear capturing table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d",
                                 prvTgfDevNum);

}


/*================================================================================*/
/*====================tgfDynamicPartitionIpLpmDefragUcRouting=====================*/
/*================================================================================*/


/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcBaseConfigurationSet
(
    GT_VOID
)
{
    prvTgfDynamicPartitionIpLpmPushUpUcBaseConfigurationSet();

}


/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipv4Addr = {0};
    GT_IPV6ADDR                             ipv6Addr = {{0}};
    GT_U8 i;
    /* set the same configuration as in push up test */
    prvTgfDynamicPartitionIpLpmPushUpUcRouteConfigurationSet();

    /* delete ipv4 prefix 1.1.1.1/32; 1.1.1.1/26; 1.1.1.1/22; 1.1.1.1/14*/

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipv4Addr.arIP, dstAddr, sizeof(ipv4Addr.arIP));

    /* AUTODOC: delete IPv4 prefix 1.1.1.1/32 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: delete IPv4 prefix 1.1.1.1/26 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, 26);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: delete IPv4 prefix 1.1.1.1/22 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, 22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: delete IPv4 prefix 1.1.1.1/14 */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, 14);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* add ipv6 prefix 2001::1234/12 */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 20;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    for (i = 0; i < 8; i++)
    {
        ipv6Addr.arIP[2 * i]       = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] >> 8);
        ipv6Addr.arIP[(2 * i) + 1] = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] & 0xFF);
    }

    /* AUTODOC: add IPv6 UC prefix 2001:0000:0000:0000:0000:0000:0000:1234/12, routeIdx=20 */
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,
                                    vrId,
                                    ipv6Addr,
                                    12,
                                    &nextHopInfo,
                                    GT_FALSE,
                                    GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
}


/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingTrafficGenerate
(
    GT_VOID
)
{
   prvTgfDynamicPartitionIpLpmPushUpUcRoutingTrafficGenerate();
}


/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipv4Addr  = {0};
    GT_IPV6ADDR ipv6Addr  = {{0}};
    GT_U32      prefixLen;
    GT_U8       i;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* retsore ipv4 route configuration */

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipv4Addr.arIP, dstAddr, sizeof(ipv4Addr.arIP));
    prefixLen = 32;
    for (i = 1; i <= 9; i++)
    {
        prefixLen = prefixLen - 2;

        /* AUTODOC: delete the IPv4 prefixes */
        if (prefixLen != 32 && prefixLen != 26 &&
            prefixLen != 22 && prefixLen != 14 )
        {
            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, vrId, ipv4Addr, prefixLen);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

        }

    }

    /* restore IPv6 route configuration */
    /* fill a destination IP address for the prefix */
    for (i = 0; i < 8; i++)
    {
        ipv6Addr.arIP[2 * i]       = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] >> 8);
        ipv6Addr.arIP[(2 * i) + 1] = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[i] & 0xFF);
    }

    /* delete ipv6 prefixes 2001::1234/16;2001::1234/12; 2001::1234/8 */
    for (prefixLen = 8; prefixLen <= 16; prefixLen += 4)
    {
        /* AUTODOC: delete the IPv6 prefixes */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, ipv6Addr, prefixLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel: %d", prvTgfDevNum);
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

    /* AUTODOC: invalidate vlan entries [5,6] */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }
    /* clear capturing table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d",
                                 prvTgfDevNum);

}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationCostumerTestScenarioSet function
* @endinternal
*
* @brief   Set Costumer Test Scenario Route Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRouteConfigurationCostumerTestScenarioSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray;
    GT_U8                                   i;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
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
    inlifEntry.ipv6UcRouteEnable = GT_TRUE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;
    inlifEntry.vrfId             = 0;

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

    /* call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, 4096, &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4\IPv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4\IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    for (i = 1; i <= 12; i++)
    {
        /* write a ARP MAC address to the Router ARP Table */
        arpMacAddr.arEther[0] = 0;
        arpMacAddr.arEther[1] = 0;
        arpMacAddr.arEther[2] = 0;
        arpMacAddr.arEther[3] = 0;
        arpMacAddr.arEther[4] = 0x22;
        arpMacAddr.arEther[5] = i+10;

        /* AUTODOC: write 12 ARP MACs 00:00:00:00:22:[0xb..0x16] to ARP Table 1..12 */
        rc = prvTgfIpRouterArpAddrWrite(i, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* write a UC Route entry to the Route Table */
        cpssOsMemSet(&routeEntriesArray, 0, sizeof(routeEntriesArray));
        routeEntriesArray.cmd                        = CPSS_PACKET_CMD_ROUTE_E;
        routeEntriesArray.cpuCodeIndex               = 0;
        routeEntriesArray.appSpecificCpuCodeEnable   = GT_FALSE;
        routeEntriesArray.unicastPacketSipFilterEnable = GT_FALSE;
        routeEntriesArray.ttlHopLimitDecEnable       = GT_FALSE;
        routeEntriesArray.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
        routeEntriesArray.ingressMirror              = GT_FALSE;
        routeEntriesArray.qosProfileMarkingEnable    = GT_FALSE;
        routeEntriesArray.qosProfileIndex            = 0;
        routeEntriesArray.qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        routeEntriesArray.modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.countSet                   = PRV_TGF_COUNT_SET_CNS;
        routeEntriesArray.trapMirrorArpBcEnable      = GT_FALSE;
        routeEntriesArray.sipAccessLevel             = 0;
        routeEntriesArray.dipAccessLevel             = 0;
        routeEntriesArray.ICMPRedirectEnable         = GT_FALSE;
        routeEntriesArray.scopeCheckingEnable        = GT_FALSE;
        routeEntriesArray.siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntriesArray.mtuProfileIndex            = 0;
        routeEntriesArray.isTunnelStart              = GT_FALSE;
        routeEntriesArray.nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
        if (i < 12)
        {
            routeEntriesArray.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
        }
        else
        {
            routeEntriesArray.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS];
        }
        routeEntriesArray.nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
        routeEntriesArray.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
        routeEntriesArray.nextHopARPPointer          = i;
        routeEntriesArray.nextHopTunnelPointer       = 0;

        /* AUTODOC: add 12 UC route entries with: */
        /* AUTODOC:   cmd=CMD_ROUTE, nextHopVlan=6 */
        /* AUTODOC:   11..20 nextHopPorts=3, 21,22 nextHopPorts=2 */
        rc = prvTgfIpUcRouteEntriesWrite(i+10, &routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


        /* AUTODOC: read and check the UC Route entry from the Route Table */
        cpssOsMemSet(&routeEntriesArray, 0, sizeof(routeEntriesArray));

        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, i+10, &routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

        PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                         routeEntriesArray.nextHopVlanId,
                         routeEntriesArray.nextHopInterface.devPort.portNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Set the FDB entry With DA_ROUTE
     */

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
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A specific test that fit a 8 lines TCAM
*         So in order for the test to check what we want we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=7
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenario
(
    GT_VOID
)
{
        GT_STATUS                               rc = GT_OK;
        GT_STATUS                               rc1 = GT_OK;

        IN  GT_U32      vrId = 0;
        GT_U32          enable_fragmentation = 1;
        GT_IPADDR       currentIp4AddrArray[10];
        GT_U32          prefixLengthIPv4Array[10];
        GT_U8           i=0;

        GT_IPV6ADDR   currentIp6AddrArray[10];
        GT_U32        prefixLengthIPv6Array[10];
        GT_BOOL       override = GT_FALSE;
        GT_BOOL       defragmentationEnable=GT_FALSE;
        GT_U32        routeEntryBaseIndexValue=0;

        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;

        /* AUTODOC: SETUP CONFIGURATION: */
        PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

        cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%12+11;
        nextHopInfo.ipLttEntry.numOfPaths          = 0;
        nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        /* test senarion for
        lpmDbPartitionEnable=0x0
        lpmDbFirstTcamLine=0
        lpmDbLastTcamLine=7
        maxNumOfIpv4Prefixes=1
        maxNumOfIpv4McEntries=1
        maxNumOfIpv6Prefixes=1*/

       /*
       127.0.0.1/32      0
       192.168.1.25/32   1
       127.0.0.0/8       17
       192.168.1.0/24    18
       */

    /* add the current prefix*/
    currentIp4AddrArray[0].arIP[0]=127;
    currentIp4AddrArray[0].arIP[1]=0;
    currentIp4AddrArray[0].arIP[2]=0;
    currentIp4AddrArray[0].arIP[3]=1;
    prefixLengthIPv4Array[0]=32;

    currentIp4AddrArray[1].arIP[0]=192;
    currentIp4AddrArray[1].arIP[1]=168;
    currentIp4AddrArray[1].arIP[2]=1;
    currentIp4AddrArray[1].arIP[3]=25;
    prefixLengthIPv4Array[1]=32;

    currentIp4AddrArray[2].arIP[0]=127;
    currentIp4AddrArray[2].arIP[1]=0;
    currentIp4AddrArray[2].arIP[2]=0;
    currentIp4AddrArray[2].arIP[3]=0;
    prefixLengthIPv4Array[2]=8;

    currentIp4AddrArray[3].arIP[0]=192;
    currentIp4AddrArray[3].arIP[1]=168;
    currentIp4AddrArray[3].arIP[2]=1;
    currentIp4AddrArray[3].arIP[3]=0;
    prefixLengthIPv4Array[3]=24;

    for (i=0;i<4;i++)
    {
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[i],
                          prefixLengthIPv4Array[i],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
        }
    }

    /*
    [0000] Add V4 16.1.1.1
    [0001] Add V4 17.1.1.1
    [0002] Add V4 18.1.1.1
    */

    /* start sequence */
    currentIp4AddrArray[0].arIP[0]=16;
    currentIp4AddrArray[0].arIP[1]=1;
    currentIp4AddrArray[0].arIP[2]=1;
    currentIp4AddrArray[0].arIP[3]=1;
    prefixLengthIPv4Array[0]=32;

    currentIp4AddrArray[1].arIP[0]=17;
    currentIp4AddrArray[1].arIP[1]=1;
    currentIp4AddrArray[1].arIP[2]=1;
    currentIp4AddrArray[1].arIP[3]=1;
    prefixLengthIPv4Array[1]=32;

    currentIp4AddrArray[2].arIP[0]=18;
    currentIp4AddrArray[2].arIP[1]=1;
    currentIp4AddrArray[2].arIP[2]=1;
    currentIp4AddrArray[2].arIP[3]=1;
    prefixLengthIPv4Array[2]=32;

    for (i=0;i<3;i++)
    {
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[i],
                          prefixLengthIPv4Array[i],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }
    }

    /*
       [0003] Add V6 3000::3000
    */

    /* add the current prefix*/
    currentIp6AddrArray[0].arIP[0]=0x30;
    currentIp6AddrArray[0].arIP[1]=0;
    currentIp6AddrArray[0].arIP[2]=0;
    currentIp6AddrArray[0].arIP[3]=0;
    currentIp6AddrArray[0].arIP[4]=0;
    currentIp6AddrArray[0].arIP[5]=0;
    currentIp6AddrArray[0].arIP[6]=0;
    currentIp6AddrArray[0].arIP[7]=0;
    currentIp6AddrArray[0].arIP[8]=0;
    currentIp6AddrArray[0].arIP[9]=0;
    currentIp6AddrArray[0].arIP[10]=0;
    currentIp6AddrArray[0].arIP[11]=0;
    currentIp6AddrArray[0].arIP[12]=0;
    currentIp6AddrArray[0].arIP[13]=0;
    currentIp6AddrArray[0].arIP[14]=0x30;
    currentIp6AddrArray[0].arIP[15]=0;
    prefixLengthIPv6Array[0]=128;

    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
    if (rc == GT_OK)
    {
        /* uncomment for debugging
        PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        */
    }
    else
    {
        PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
    }
   rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        return;
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        return;
    }

    /*
    [0004] Add V4 19.1.1.1
    [0005] Add V4 20.1.1.1
    [0006] Add V4 21.1.1.1
    [0007] Add V4 22.1.1.1
    [0008] Add V4 23.1.1.1
    [0009] Add V4 24.1.1.1
    [0010] Add V4 25.1.1.1
    [0011] Add V4 26.1.1.1
    */
    for (i=0;i<8;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 19+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0012] Del V6 3000::300A
    [0013] Del V6 3000::3009
    */
    currentIp6AddrArray[0].arIP[0]=0x30;
    currentIp6AddrArray[0].arIP[1]=0;
    currentIp6AddrArray[0].arIP[2]=0;
    currentIp6AddrArray[0].arIP[3]=0;
    currentIp6AddrArray[0].arIP[4]=0;
    currentIp6AddrArray[0].arIP[5]=0;
    currentIp6AddrArray[0].arIP[6]=0;
    currentIp6AddrArray[0].arIP[7]=0;
    currentIp6AddrArray[0].arIP[8]=0;
    currentIp6AddrArray[0].arIP[9]=0;
    currentIp6AddrArray[0].arIP[10]=0;
    currentIp6AddrArray[0].arIP[11]=0;
    currentIp6AddrArray[0].arIP[12]=0;
    currentIp6AddrArray[0].arIP[13]=0;
    currentIp6AddrArray[0].arIP[14]=0x30;
    currentIp6AddrArray[0].arIP[15]=0x0A;
    prefixLengthIPv6Array[0]=128;

    currentIp6AddrArray[1].arIP[0]=0x30;
    currentIp6AddrArray[1].arIP[1]=0;
    currentIp6AddrArray[1].arIP[2]=0;
    currentIp6AddrArray[1].arIP[3]=0;
    currentIp6AddrArray[1].arIP[4]=0;
    currentIp6AddrArray[1].arIP[5]=0;
    currentIp6AddrArray[1].arIP[6]=0;
    currentIp6AddrArray[1].arIP[7]=0;
    currentIp6AddrArray[1].arIP[8]=0;
    currentIp6AddrArray[1].arIP[9]=0;
    currentIp6AddrArray[1].arIP[10]=0;
    currentIp6AddrArray[1].arIP[11]=0;
    currentIp6AddrArray[1].arIP[12]=0;
    currentIp6AddrArray[1].arIP[13]=0;
    currentIp6AddrArray[1].arIP[14]=0x30;
    currentIp6AddrArray[1].arIP[15]=0x09;
    prefixLengthIPv6Array[1]=128;
    for (i=0;i<2;i++)
    {
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[i], prefixLengthIPv6Array[i]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[i].u32Ip[3],currentIp6AddrArray[i].u32Ip[2],currentIp6AddrArray[i].u32Ip[1],currentIp6AddrArray[i].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[i].u32Ip[3],
                                            currentIp6AddrArray[i].u32Ip[2],currentIp6AddrArray[i].u32Ip[1],currentIp6AddrArray[i].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0014] Add V4 27.1.1.1
    [0015] Add V4 28.1.1.1
    [0016] Add V4 29.1.1.1
    [0017] Add V4 30.1.1.1
    */
    for (i=0;i<4;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 27+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0018] Del V6 3000::3008
    [0019] Del V6 3000::3007
    [0020] Del V6 3000::3006
    */
    currentIp6AddrArray[0].arIP[0]=0x30;
    currentIp6AddrArray[0].arIP[1]=0;
    currentIp6AddrArray[0].arIP[2]=0;
    currentIp6AddrArray[0].arIP[3]=0;
    currentIp6AddrArray[0].arIP[4]=0;
    currentIp6AddrArray[0].arIP[5]=0;
    currentIp6AddrArray[0].arIP[6]=0;
    currentIp6AddrArray[0].arIP[7]=0;
    currentIp6AddrArray[0].arIP[8]=0;
    currentIp6AddrArray[0].arIP[9]=0;
    currentIp6AddrArray[0].arIP[10]=0;
    currentIp6AddrArray[0].arIP[11]=0;
    currentIp6AddrArray[0].arIP[12]=0;
    currentIp6AddrArray[0].arIP[13]=0;
    currentIp6AddrArray[0].arIP[14]=0x30;
    currentIp6AddrArray[0].arIP[15]=0x08;
    prefixLengthIPv6Array[0]=128;

    currentIp6AddrArray[1].arIP[0]=0x30;
    currentIp6AddrArray[1].arIP[1]=0;
    currentIp6AddrArray[1].arIP[2]=0;
    currentIp6AddrArray[1].arIP[3]=0;
    currentIp6AddrArray[1].arIP[4]=0;
    currentIp6AddrArray[1].arIP[5]=0;
    currentIp6AddrArray[1].arIP[6]=0;
    currentIp6AddrArray[1].arIP[7]=0;
    currentIp6AddrArray[1].arIP[8]=0;
    currentIp6AddrArray[1].arIP[9]=0;
    currentIp6AddrArray[1].arIP[10]=0;
    currentIp6AddrArray[1].arIP[11]=0;
    currentIp6AddrArray[1].arIP[12]=0;
    currentIp6AddrArray[1].arIP[13]=0;
    currentIp6AddrArray[1].arIP[14]=0x30;
    currentIp6AddrArray[1].arIP[15]=0x07;
    prefixLengthIPv6Array[1]=128;

    currentIp6AddrArray[2].arIP[0]=0x30;
    currentIp6AddrArray[2].arIP[1]=0;
    currentIp6AddrArray[2].arIP[2]=0;
    currentIp6AddrArray[2].arIP[3]=0;
    currentIp6AddrArray[2].arIP[4]=0;
    currentIp6AddrArray[2].arIP[5]=0;
    currentIp6AddrArray[2].arIP[6]=0;
    currentIp6AddrArray[2].arIP[7]=0;
    currentIp6AddrArray[2].arIP[8]=0;
    currentIp6AddrArray[2].arIP[9]=0;
    currentIp6AddrArray[2].arIP[10]=0;
    currentIp6AddrArray[2].arIP[11]=0;
    currentIp6AddrArray[2].arIP[12]=0;
    currentIp6AddrArray[2].arIP[13]=0;
    currentIp6AddrArray[2].arIP[14]=0x30;
    currentIp6AddrArray[2].arIP[15]=0x06;
    prefixLengthIPv6Array[2]=128;
    for (i=0;i<3;i++)
    {
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[i], prefixLengthIPv6Array[i]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[i].u32Ip[3],currentIp6AddrArray[i].u32Ip[2],currentIp6AddrArray[i].u32Ip[1],currentIp6AddrArray[i].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[i].u32Ip[3],
                                          currentIp6AddrArray[i].u32Ip[2],currentIp6AddrArray[i].u32Ip[1],currentIp6AddrArray[i].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0021] Del V4 56.1.1.1
    [0022] Del V4 55.1.1.1
    [0023] Del V4 54.1.1.1
    [0024] Del V4 53.1.1.1
    */
    for (i=4;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 52+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4]   1 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6]   2 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0025] Add V6 3000::3001
    [0026] Add V6 3000::3002
    [0027] Add V6 3000::3003
    [0028] Add V6 3000::3004
    [0029] Add V6 3000::3005
    [0030] Add V6 3000::3006
    */
    for (i=0;i<6;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15]=1+i;
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4]   3 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6]   4 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    for (i=0;i<10;i++)
    {
        /*
        [0031] Del V6 3000::3005
        [0032] Del V6 3000::3004
        [0033] Del V6 3000::3003
        [0034] Del V6 3000::3002
        [0035] Del V6 3000::3001
        */
        if (i<5)
        {
            currentIp6AddrArray[0].arIP[0] = 0x30;
            currentIp6AddrArray[0].arIP[1]=0;
            currentIp6AddrArray[0].arIP[2]=0;
            currentIp6AddrArray[0].arIP[3]=0;
            currentIp6AddrArray[0].arIP[4]=0;
            currentIp6AddrArray[0].arIP[5]=0;
            currentIp6AddrArray[0].arIP[6]=0;
            currentIp6AddrArray[0].arIP[7]=0;
            currentIp6AddrArray[0].arIP[8]=0;
            currentIp6AddrArray[0].arIP[9]=0;
            currentIp6AddrArray[0].arIP[10]=0;
            currentIp6AddrArray[0].arIP[11]=0;
            currentIp6AddrArray[0].arIP[12]=0;
            currentIp6AddrArray[0].arIP[13]=0;
            currentIp6AddrArray[0].arIP[14]=0x30;
            currentIp6AddrArray[0].arIP[15]=5-i;
        }
        else
        {
            if (i==5)
            {
                /*
                [0036] Del V6 3000::300A
                */

                currentIp6AddrArray[0].arIP[0] = 0x30;
                currentIp6AddrArray[0].arIP[1]=0;
                currentIp6AddrArray[0].arIP[2]=0;
                currentIp6AddrArray[0].arIP[3]=0;
                currentIp6AddrArray[0].arIP[4]=0;
                currentIp6AddrArray[0].arIP[5]=0;
                currentIp6AddrArray[0].arIP[6]=0;
                currentIp6AddrArray[0].arIP[7]=0;
                currentIp6AddrArray[0].arIP[8]=0;
                currentIp6AddrArray[0].arIP[9]=0;
                currentIp6AddrArray[0].arIP[10]=0;
                currentIp6AddrArray[0].arIP[11]=0;
                currentIp6AddrArray[0].arIP[12]=0;
                currentIp6AddrArray[0].arIP[13]=0;
                currentIp6AddrArray[0].arIP[14]=0x30;
                currentIp6AddrArray[0].arIP[15]=0x0A;
            }
            else
            {
                /*
                [0037] Del V6 3000::3009
                [0038] Del V6 3000::3008
                [0039] Del V6 3000::3007
                [0040] Del V6 3000::3006
                */
                currentIp6AddrArray[0].arIP[0] = 0x30;
                currentIp6AddrArray[0].arIP[1]=0;
                currentIp6AddrArray[0].arIP[2]=0;
                currentIp6AddrArray[0].arIP[3]=0;
                currentIp6AddrArray[0].arIP[4]=0;
                currentIp6AddrArray[0].arIP[5]=0;
                currentIp6AddrArray[0].arIP[6]=0;
                currentIp6AddrArray[0].arIP[7]=0;
                currentIp6AddrArray[0].arIP[8]=0;
                currentIp6AddrArray[0].arIP[9]=0;
                currentIp6AddrArray[0].arIP[10]=0;
                currentIp6AddrArray[0].arIP[11]=0;
                currentIp6AddrArray[0].arIP[12]=0;
                currentIp6AddrArray[0].arIP[13]=0;
                currentIp6AddrArray[0].arIP[14]=0x30;
                currentIp6AddrArray[0].arIP[15]=15-i;
            }
        }
        prefixLengthIPv6Array[0]=128;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[0], prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[0].u32Ip[3],
                                            currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    for (i=0;i<5;i++)
    {
    /*
        [0041] Del V4 52.1.1.1
        [0042] Del V4 51.1.1.1
        [0043] Del V4 50.1.1.1
        [0044] Del V4 49.1.1.1
        [0045] Del V4 48.1.1.1

    */
        currentIp4AddrArray[0].arIP[0] = 52-i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    for (i=0;i<6;i++)
    {
        /*
        [0046] Add V6 3000::3007
        [0047] Add V6 3000::3008
        [0048] Add V6 3000::3009
        */
        if (i<3)
        {
            currentIp6AddrArray[0].arIP[0] = 0x30;
            currentIp6AddrArray[0].arIP[1]=0;
            currentIp6AddrArray[0].arIP[2]=0;
            currentIp6AddrArray[0].arIP[3]=0;
            currentIp6AddrArray[0].arIP[4]=0;
            currentIp6AddrArray[0].arIP[5]=0;
            currentIp6AddrArray[0].arIP[6]=0;
            currentIp6AddrArray[0].arIP[7]=0;
            currentIp6AddrArray[0].arIP[8]=0;
            currentIp6AddrArray[0].arIP[9]=0;
            currentIp6AddrArray[0].arIP[10]=0;
            currentIp6AddrArray[0].arIP[11]=0;
            currentIp6AddrArray[0].arIP[12]=0;
            currentIp6AddrArray[0].arIP[13]=0;
            currentIp6AddrArray[0].arIP[14]=0x30;
            currentIp6AddrArray[0].arIP[15]=7+i;
        }
        else
        {
            /*
            [0049] Add V6 3000::3000
            [0050] Add V6 3000::3001
            [0051] Add V6 3000::3002
            */
            currentIp6AddrArray[0].arIP[0] = 0x30;
            currentIp6AddrArray[0].arIP[1]=0;
            currentIp6AddrArray[0].arIP[2]=0;
            currentIp6AddrArray[0].arIP[3]=0;
            currentIp6AddrArray[0].arIP[4]=0;
            currentIp6AddrArray[0].arIP[5]=0;
            currentIp6AddrArray[0].arIP[6]=0;
            currentIp6AddrArray[0].arIP[7]=0;
            currentIp6AddrArray[0].arIP[8]=0;
            currentIp6AddrArray[0].arIP[9]=0;
            currentIp6AddrArray[0].arIP[10]=0;
            currentIp6AddrArray[0].arIP[11]=0;
            currentIp6AddrArray[0].arIP[12]=0;
            currentIp6AddrArray[0].arIP[13]=0;
            currentIp6AddrArray[0].arIP[14]=0x30;
            currentIp6AddrArray[0].arIP[15]=i-3;
        }
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0052] Del V4 47.1.1.1
    [0053] Del V4 46.1.1.1
    [0054] Del V4 45.1.1.1
    [0055] Del V4 44.1.1.1
    [0056] Del V4 43.1.1.1
    */
    for (i=5;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 42+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0057] Del V6 3000::3005
    [0058] Del V6 3000::3004
    [0059] Del V6 3000::3003
    */
    for (i=0;i<3;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15]=5-i;

        prefixLengthIPv6Array[0]=128;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[0], prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[0].u32Ip[3],
                                            currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0060] Del V4 42.1.1.1
    [0061] Del V4 41.1.1.1
    [0062] Del V4 40.1.1.1
    [0063] Del V4 39.1.1.1
    [0064] Del V4 38.1.1.1
    */
    for (i=5;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 37+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0065] Add V4 31.1.1.1
    [0066] Add V4 32.1.1.1
    [0067] Add V4 33.1.1.1
    [0068] Add V4 34.1.1.1
    */
    for (i=0;i<4;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 31+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0069] Del V6 3000::3002
    [0070] Del V6 3000::3001
    [0071] Del V6 3000::300A
    [0072] Del V6 3000::3009
    */
    for (i=0;i<4;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        switch (i)
        {
        case 0:
            currentIp6AddrArray[0].arIP[15] = 2;
            break;
        case 1:
            currentIp6AddrArray[0].arIP[15] = 1;
            break;
        case 2:
            currentIp6AddrArray[0].arIP[15] = 0x0A;
            break;
        case 3:
            currentIp6AddrArray[0].arIP[15] = 9;
            break;
        }

        prefixLengthIPv6Array[0]=128;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[0], prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[0].u32Ip[3],
                                            currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0073] Add V6 3000::3003
    [0074] Add V6 3000::3004
    */
    for (i=0;i<2;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15]=3+i;
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0075] Del V4 37.1.1.1
    [0076] Del V4 36.1.1.1
    [0077] Del V4 35.1.1.1
    [0078] Del V4 34.1.1.1
    [0079] Del V4 33.1.1.1
    [0080] Del V4 32.1.1.1
    [0081] Del V4 31.1.1.1
    [0082] Del V4 30.1.1.1
    [0083] Del V4 29.1.1.1
    [0084] Del V4 28.1.1.1
    [0085] Del V4 27.1.1.1
    [0086] Del V4 26.1.1.1
    [0087] Del V4 25.1.1.1
    [0088] Del V4 24.1.1.1
    [0089] Del V4 23.1.1.1
    */
    for (i=15;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 22+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0090] Del V6 3000::3008
    [0091] Del V6 3000::3007
    [0092] Del V6 3000::3006
    */
    for (i=0;i<3;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15] = 8-i;
        prefixLengthIPv6Array[0]=128;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[0], prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
             PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[0].u32Ip[3],
                                            currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0093] Add V4 35.1.1.1
    [0094] Add V4 36.1.1.1
    [0095] Add V4 37.1.1.1
    [0096] Add V4 38.1.1.1
    */
    for (i=0;i<4;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 35+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0097] Del V4 22.1.1.1
    [0098] Del V4 21.1.1.1
    [0099] Del V4 20.1.1.1
    [0100] Del V4 19.1.1.1
    [0101] Del V4 18.1.1.1
    */
    for (i=5;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 17+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0102] Add V6 3000::3005
    [0103] Add V6 3000::3006
    */
    for (i=0;i<2;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15]=5+i;
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }
            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,1);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }
            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }
            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,1);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }
            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
            }

            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }
            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,1);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }
            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }
            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,1);
            if (rc !=GT_OK)
            {
                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            }

            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
            }

            return;
        }
    }
    /*
    [0104] Add V4 39.1.1.1
    [0105] Add V4 40.1.1.1
    [0106] Add V4 41.1.1.1
    [0107] Add V4 42.1.1.1
    */
    for (i=0;i<4;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 39+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0108] Del V6 3000::3005
    [0109] Del V6 3000::3004
    [0110] Del V6 3000::3003
    */
    for (i=0;i<3;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15] = 5-i;
        prefixLengthIPv6Array[0]=128;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[0], prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[0].u32Ip[3],
                                            currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
      [0111] Add V6 3000::3007
      [0112] Add V6 3000::3008
    */
    for (i=0;i<2;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15]=7+i;
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0113] Del V4 17.1.1.1
    [0114] Del V4 56.1.1.1
    [0115] Del V4 55.1.1.1
    [0116] Del V4 54.1.1.1
    [0117] Del V4 53.1.1.1
    */
    for (i=5;i>0;i--)
    {
        if (i==5)
        {
            currentIp4AddrArray[0].arIP[0] = 17;
        }
        else
        {
            currentIp4AddrArray[0].arIP[0] = 52 + i;
        }
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4]   5 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6]    6 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0118] Add V6 3000::3009
    [0119] Add V6 3000::3000
    */
    for (i=0;i<2;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        if (i==0)
        {
            currentIp6AddrArray[0].arIP[15] = 9;
        }
        else
        {
            currentIp6AddrArray[0].arIP[15]=0;
        }
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }
       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4]   7 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6]   8 valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    /*
    [0120] Del V6 3000::3002
    [0121] Del V6 3000::3001
    [0122] Del V6 3000::300A
    [0123] Del V6 3000::3009
    */
    for (i=0;i<4;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        switch (i)
        {
        case 0:
            currentIp6AddrArray[0].arIP[15] = 2;
            break;
        case 1:
            currentIp6AddrArray[0].arIP[15] = 1;
            break;
        case 2:
            currentIp6AddrArray[0].arIP[15] = 0x0A;
            break;
        case 3:
            currentIp6AddrArray[0].arIP[15] = 9;
            break;
        }
        prefixLengthIPv6Array[0]=128;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[0], prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[0].u32Ip[3],
                                            currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0124] Add V4 43.1.1.1
    [0125] Add V4 44.1.1.1
    [0126] Add V4 45.1.1.1
    [0127] Add V4 46.1.1.1
    [0128] Add V4 47.1.1.1
    [0129] Add V4 48.1.1.1
    [0130] Add V4 49.1.1.1
    [0131] Add V4 50.1.1.1
    */
    for (i=0;i<8;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 43+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0132] Del V4 52.1.1.1
    [0133] Del V4 51.1.1.1
    [0134] Del V4 50.1.1.1
    [0135] Del V4 49.1.1.1
    [0136] Del V4 48.1.1.1
    */
    for (i=5;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 47 + i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0137] Add V6 3000::3001
    [0138] Add V6 3000::3002
    [0139] Add V6 3000::3003
    [0140] Add V6 3000::3004
    */
    for (i=0;i<4;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15] = 1+i;
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0141] Add V4 51.1.1.1
    [0142] Add V4 52.1.1.1
    [0143] Add V4 53.1.1.1
    [0144] Add V4 54.1.1.1
    */
    for (i=0;i<4;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 51+i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0145] Del V4 47.1.1.1
    [0146] Del V4 46.1.1.1
    [0147] Del V4 45.1.1.1
    [0148] Del V4 44.1.1.1
    [0149] Del V4 43.1.1.1
    [0150] Del V4 42.1.1.1
    [0151] Del V4 41.1.1.1
    [0152] Del V4 40.1.1.1
    [0153] Del V4 39.1.1.1
    [0154] Del V4 38.1.1.1
    [0155] Del V4 37.1.1.1
    [0156] Del V4 36.1.1.1
    [0157] Del V4 35.1.1.1
    [0158] Del V4 34.1.1.1
    [0159] Del V4 33.1.1.1
    */
    for (i=15;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 32 + i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0160] Add V4 55.1.1.1
    [0161] Add V4 16.1.1.1
    [0162] Add V4 17.1.1.1
    [0163] Add V4 18.1.1.1
    */
    for (i=0;i<4;i++)
    {
        if (i==0)
        {
            currentIp4AddrArray[0].arIP[0] = 55;
        }
        else
        {
            currentIp4AddrArray[0].arIP[0] = 15 + i;
        }
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0164] Del V6 3000::3008
    [0165] Del V6 3000::3007
    [0166] Del V6 3000::3006
    */
    for (i=0;i<3;i++)
    {
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15] = 8-i;
        prefixLengthIPv6Array[0]=128;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, vrId, currentIp6AddrArray[0], prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V6, 0x%8x %8x %8x %8x:\n",currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("failed:0x%x!!! Del V6, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrArray[0].u32Ip[3],
                                            currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0167] Add V4 19.1.1.1
    [0168] Add V4 20.1.1.1
    [0169] Add V4 21.1.1.1
    [0170] Add V4 22.1.1.1
    */
    for (i=0;i<4;i++)
    {
        currentIp4AddrArray[0].arIP[0] = 19 + i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[0],
                          prefixLengthIPv4Array[0],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }
    /*
    [0171] Del V4 32.1.1.1
    [0172] Del V4 31.1.1.1
    [0173] Del V4 30.1.1.1
    [0174] Del V4 29.1.1.1
    [0175] Del V4 28.1.1.1
    */
    for (i=5;i>0;i--)
    {
        currentIp4AddrArray[0].arIP[0] = 27 + i;
        currentIp4AddrArray[0].arIP[1]=1;
        currentIp4AddrArray[0].arIP[2]=1;
        currentIp4AddrArray[0].arIP[3]=1;
        prefixLengthIPv4Array[0]=32;

        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrArray[0],prefixLengthIPv4Array[0]);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Del V4, %d.%d.%d.%d\n",currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
            */
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Del V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[0].arIP[0],currentIp4AddrArray[0].arIP[1],
                         currentIp4AddrArray[0].arIP[2],currentIp4AddrArray[0].arIP[3]);
        }

       rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

     /* retrieves configuration of the Virtual Router */
    rc = prvTgfIpLpmVirtualRouterGet(prvTgfLpmDBId, vrId,
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterGet FAILED, rc = [%d]", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterGet");
        return;
    }

    /* if Ipv6 MC is supported then one more line is occupied by the default ipv6 MC
       and we will have defrag when adding 3000::3005 and not when adding 3000::3006 */

    /* uncomment for debugging
    if (prvTgfDefLpmDBVrCfg.isSupportIpv6Mc==GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("\n [0176] before adding V6 3000::3005 ->  before this entry is added, the ipv4 and ipv6 valid check is ok\n");
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }

        rc1 = dumpRouteTcam(GT_TRUE);
        if ((rc != GT_OK)||(rc1 != GT_OK))
        {
            if (rc1!=GT_OK)
            {
                PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc1, "prvTgfIpValidityCheck");
            }
            return;
        }
    }
    */

    /* [0176] Add V6 3000::3005      ->  after this entry is added, the ipv4 and ipv6 valid
       check is ok when isSupportIpv6Mc=GT_FALSE but not ok when isSupportIpv6Mc=GT_TRUE.*/
    currentIp6AddrArray[0].arIP[0] = 0x30;
    currentIp6AddrArray[0].arIP[1]=0;
    currentIp6AddrArray[0].arIP[2]=0;
    currentIp6AddrArray[0].arIP[3]=0;
    currentIp6AddrArray[0].arIP[4]=0;
    currentIp6AddrArray[0].arIP[5]=0;
    currentIp6AddrArray[0].arIP[6]=0;
    currentIp6AddrArray[0].arIP[7]=0;
    currentIp6AddrArray[0].arIP[8]=0;
    currentIp6AddrArray[0].arIP[9]=0;
    currentIp6AddrArray[0].arIP[10]=0;
    currentIp6AddrArray[0].arIP[11]=0;
    currentIp6AddrArray[0].arIP[12]=0;
    currentIp6AddrArray[0].arIP[13]=0;
    currentIp6AddrArray[0].arIP[14]=0x30;
    currentIp6AddrArray[0].arIP[15] = 5;
    prefixLengthIPv6Array[0]=128;

    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
    if (rc == GT_OK)
    {
        /* uncomment for debugging
        PRV_UTF_LOG5_MAC("Add Ipv6Addr, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        */
    }
    else
    {
        PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        return;
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        return;
    }

    /* uncomment for debugging
    PRV_UTF_LOG0_MAC("\n [0176] Add V6 3000::3005      ->  after this entry is added, the ipv4 and ipv6 valid check is ok\n");
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 2\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }

    rc1 = dumpRouteTcam(GT_TRUE);
    if ((rc != GT_OK)||(rc1 != GT_OK))
    {
        if (rc1!=GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam 2\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc1, "prvTgfIpValidityCheck");
        }
        return;
    }
    */

    /* if Ipv6 MC is NOT supported then we will have defrag when adding 3000::3006 and not when adding 3000::3005 */
    if (prvTgfDefLpmDBVrCfg.isSupportIpv6Mc==GT_FALSE)
    {
        /*[0177] Add V6 3000::3006     ->    after added this entry , ipv4 valid check is failed, ipv6 is ok.   There is move action for ipv4.*/
        currentIp6AddrArray[0].arIP[0] = 0x30;
        currentIp6AddrArray[0].arIP[1]=0;
        currentIp6AddrArray[0].arIP[2]=0;
        currentIp6AddrArray[0].arIP[3]=0;
        currentIp6AddrArray[0].arIP[4]=0;
        currentIp6AddrArray[0].arIP[5]=0;
        currentIp6AddrArray[0].arIP[6]=0;
        currentIp6AddrArray[0].arIP[7]=0;
        currentIp6AddrArray[0].arIP[8]=0;
        currentIp6AddrArray[0].arIP[9]=0;
        currentIp6AddrArray[0].arIP[10]=0;
        currentIp6AddrArray[0].arIP[11]=0;
        currentIp6AddrArray[0].arIP[12]=0;
        currentIp6AddrArray[0].arIP[13]=0;
        currentIp6AddrArray[0].arIP[14]=0x30;
        currentIp6AddrArray[0].arIP[15] = 6;
        prefixLengthIPv6Array[0]=128;

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 22;
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG5_MAC("Add Ipv6Addr, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Add V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

        PRV_UTF_LOG0_MAC("\n [0177] Add V6 3000::3006     ->    after added this entry , ipv4 valid check used to fail, ipv6 is ok.   There is move action for ipv4\n");
        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 3\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }

        rc1 = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
        if ((rc != GT_OK)||(rc1 != GT_OK))
        {
            if (rc1!=GT_OK)
            {
                PRV_UTF_LOG0_MAC("dumpRouteTcam 3\n");
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc1, "dumpRouteTcam");
            }
            return;
        }

        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }
    }

    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A random IP v4/v6 prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=1000
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomCostumerTestScenario
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;

    GT_U32 loop_add_v4 = 0;
    GT_U32 loop_del_v4 = 0;
    GT_U32 loop_add_v6 = 0;
    GT_U32 loop_del_v6 = 0;

    GT_U32 max_ipv4 =2000;
    GT_U32 max_ipv6 =1000;

    GT_U32        loop = 0;
    GT_IPADDR     currentIp4AddrAdd;
    GT_U32        baseIp4Addr=0x30080801;
    GT_IPADDR     currentIp4AddrDel;
    GT_IPV6ADDR   currentIp6AddrAdd;
    GT_U8         baseIp6Addr[16]={0x30,0,0,0,0,0,0,0,0,0,0,0,0,0,0x30,0};
    GT_IPV6ADDR   currentIp6AddrDel;
    GT_U32        prefixLengthIPv6 = 128;
    GT_U32        prefixLengthIPv4 = 32;

    IN  GT_U32    vrId = 0;
    GT_U32        enable_fragmentation = 1;
    GT_IPADDR     currentIp4AddrArray[10];
    GT_U32        prefixLengthIPv4Array[10];
    GT_U32        i=0,loop_times;
    GT_BOOL       override = GT_FALSE;
    GT_BOOL       defragmentationEnable=GT_FALSE;
    GT_U32        routeEntryBaseIndexValue=0;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%12+11;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    /* test senarion for
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=1000
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1*/

    /*
    127.0.0.1/32      0
    192.168.1.25/32   1
    127.0.0.0/8       17
    192.168.1.0/24    18
    */

    /* add the current prefix*/
    currentIp4AddrArray[0].arIP[0]=127;
    currentIp4AddrArray[0].arIP[1]=0;
    currentIp4AddrArray[0].arIP[2]=0;
    currentIp4AddrArray[0].arIP[3]=1;
    prefixLengthIPv4Array[0]=32;

    currentIp4AddrArray[1].arIP[0]=192;
    currentIp4AddrArray[1].arIP[1]=168;
    currentIp4AddrArray[1].arIP[2]=1;
    currentIp4AddrArray[1].arIP[3]=25;
    prefixLengthIPv4Array[1]=32;

    currentIp4AddrArray[2].arIP[0]=127;
    currentIp4AddrArray[2].arIP[1]=0;
    currentIp4AddrArray[2].arIP[2]=0;
    currentIp4AddrArray[2].arIP[3]=0;
    prefixLengthIPv4Array[2]=8;

    currentIp4AddrArray[3].arIP[0]=192;
    currentIp4AddrArray[3].arIP[1]=168;
    currentIp4AddrArray[3].arIP[2]=1;
    currentIp4AddrArray[3].arIP[3]=0;
    prefixLengthIPv4Array[3]=24;

    for (i=0;i<4;i++)
    {
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[i],
                          prefixLengthIPv4Array[i],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /*
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
        }
    }

    for(loop_times=0;loop_times<PRV_TGF_LOOPS_NUM;loop_times++)
    {
        /* uncomment for debugging
        PRV_UTF_LOG1_MAC("loop_times:%d:\n",loop_times);
        */
        /*random*/
        loop = (cpssOsRand() % 4);

        if(loop == 0)
        {
        /*  add IPv4 prefix from (IPV4ADDR+0), (IPV4ADDR+1). to (IPV4ADDR+n1),
            In the next loop, add IPv4 prefix from (IPV4ADDR+n1+1)  to(IPV4ADDR+n1+n1)
            And the next loop...
            The total IPv4 added less than 2k.
            If the number of IPv4 added up to 2k,go start add IPv4 from (IPV4ADDR+0) */
            currentIp4AddrAdd.u32Ip = baseIp4Addr;
            while(1)
            {
                /* add the current prefix*/
                /* if the prefix we try to add is a MC prefix then skip it */
                if ((currentIp4AddrAdd.arIP[0]&0xF0)!=0xE0)
                {
                    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp4AddrAdd,prefixLengthIPv4,&nextHopInfo,override,defragmentationEnable);
                    if (rc == GT_OK)
                    {
                        /* uncomment for debugging
                        PRV_UTF_LOG1_MAC("add Ipv4Addr, 0x%x:\n",currentIp4AddrAdd.u32Ip);
                        */

                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG1_MAC("currentIp4AddrAdd that fail validity,0x%x\n",currentIp4AddrAdd.u32Ip);
                            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
                            return;
                        }
                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG2_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n after ipv4add 0x%x\n", rc,currentIp4AddrAdd.u32Ip);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
                            return;
                        }
                    }
                    else
                    {
                        if ((rc!=GT_ALREADY_EXIST)&& (rc!=GT_FULL)&&(rc!=GT_BAD_PARAM))
                        {
                            PRV_UTF_LOG2_MAC("FAIL:0x%x,add Ipv4Addr, 0x%x:\n", rc, currentIp4AddrAdd.u32Ip);
                        }
                    }
                }

                loop_add_v4 = (loop_add_v4 + 1) % max_ipv4;
                currentIp4AddrAdd.arIP[3]  = currentIp4AddrAdd.arIP[3]  +  (loop_add_v4 % 256);
                currentIp4AddrAdd.arIP[2]  = currentIp4AddrAdd.arIP[2]  +  ((loop_add_v4 / 256) % 256);

                if((loop_add_v4 % 122) == 33)
                {
                    break;
                }
            }
    }
    else if(loop == 1)
    {

        /*delete IPv4 prefix from (IPV4ADDR+2k), (IPV4ADDR+2k-1) to (IPV4ADDR+2k-n2),
        In the next loop, delete IPv4 prefix from (IPV4ADDR+2k-n2-1)  to(IPV4ADDR+2k-n2-n2)
        And the next loop
        If the number of IPv4 deleted up to 2k,go start delete IPv4 from  (IPV4ADDR+2k), */
        cpssOsMemCpy(&currentIp4AddrDel.u32Ip, &baseIp4Addr, sizeof(currentIp4AddrDel));
        currentIp4AddrDel.arIP[3] += (20000 % 256);
        currentIp4AddrDel.arIP[2] += ((20000 / 256) % 256);
        while(1)
        {
            /* delete the current prefix */
            /* if the prefix we try to delete is a MC prefix then skip it */
            if ((currentIp4AddrDel.arIP[0]&0xF0)!=0xE0)
            {
                rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrDel,prefixLengthIPv4);
                if (rc == GT_OK)
                {
                    /* uncomment for debugging
                    PRV_UTF_LOG1_MAC("delete Ipv4Addr, 0x%x:\n",currentIp4AddrDel.u32Ip);
                    */
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG1_MAC("currentIp4AddrDel that fail validity,0x%x\n",currentIp4AddrDel.u32Ip);
                        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
                        return;
                    }
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG2_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n, after ipv4 del 0x%x\n", rc,currentIp4AddrDel.u32Ip);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
                        return;
                    }
                }
                else
                {
                    if (rc!=GT_NOT_FOUND)
                    {
                        PRV_UTF_LOG2_MAC("FAIL:0x%x,delete Ipv4Addr, 0x%x:\n",rc,currentIp4AddrDel.u32Ip);
                    }
                }
            }
            loop_del_v4 = (loop_del_v4 + 1) % 20000;
            currentIp4AddrDel.arIP[3]  = currentIp4AddrDel.arIP[3]  -  (loop_del_v4 % 256);
            currentIp4AddrDel.arIP[2]  = currentIp4AddrDel.arIP[2]  -  ((loop_del_v4 / 256) % 256);

            if((loop_del_v4 % 144) == 77)
            {
                break;
            }
        }
    }
    else if(loop == 2)
    {
        /*  add IPv6 prefix from (IPV6ADDR+0), (IPV6ADDR+1) to (IPV6ADDR+n3),
            In the next loop, add IPv6 prefix from (IPV6ADDR+n3+1)  to(IPV6ADDR+n3+n3)
            And the next loop
            The total IPv6 added less than 1k. If the number of IPv6 added up to 1k,go start add IPv6 from  (IPV6ADDR+0), */
        cpssOsMemCpy(currentIp6AddrAdd.arIP, (void*)baseIp6Addr, sizeof(GT_IPV6ADDR));
        while(1)
        {
            /* add the current prefix*/
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrAdd,prefixLengthIPv6,&nextHopInfo,override,enable_fragmentation);
            if (rc == GT_OK)
            {
                /* uncomment for debugging
                PRV_UTF_LOG5_MAC("add Ipv6Addr, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
                */
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG4_MAC("currentIp6AddrAdd that fail validity,0x%8x %8x %8x %8x:\n",currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
                    PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
                    return;
                }
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG5_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail afteripv6 add,0x%8x %8x %8x %8x:\n", rc,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
                    return;
                }
            }
            else
            {
                if ((rc!=GT_ALREADY_EXIST)&& (rc!=GT_FULL)&&(rc!=GT_BAD_PARAM))
                    PRV_UTF_LOG6_MAC("FAIL:0x%x!!! add Ipv6Addr, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
            }
            loop_add_v6 = (loop_add_v6 + 1) % max_ipv6;
            currentIp6AddrAdd.arIP[15]  = currentIp6AddrAdd.arIP[15]  +  (loop_add_v6 % 256);
            currentIp6AddrAdd.arIP[14]  = currentIp6AddrAdd.arIP[14]  +  ((loop_add_v6 / 256) % 256);

            if((loop_add_v6 % 166) == 111)
            {
                 break;
            }
        }
    }
    else
    {
        /*delete IPv6 prefix from (IPV6ADDR+1k), (IPV6ADDR+1k-1) to (IPV6ADDR+1k-n4),
        In the next loop, delete IPv6 prefix from (IPV6ADDR+1k-n4-1)  to(IPV6ADDR+1k-n4-n4)
        And the next loop
        If the number of IPv6 deleted up to 1k,go start delete IPv6 from  (IPV6ADDR+1k), */
        cpssOsMemCpy(currentIp6AddrDel.arIP, (void*)baseIp6Addr, sizeof(GT_IPV6ADDR));
        currentIp6AddrDel.arIP[15] += (10000 % 256);
        currentIp6AddrDel.arIP[14] += ((10000 / 256) % 256);
        while(1)
        {
            /* delete the current prefix */
            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId,vrId,currentIp6AddrDel, prefixLengthIPv6);
            if (rc == GT_OK)
            {
                /* uncomment for debugging
                PRV_UTF_LOG4_MAC("delete Ipv6Addr, 0x%8x %8x %8x %8x:\n",currentIp6AddrDel.u32Ip[3],
                                currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                */
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG4_MAC("currentIp6AddrDel that fail validity,0x%8x %8x %8x %8x:\n",currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                    PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
                    return;
                }
               rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG5_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail after ipv6 del,0x%8x %8x %8x %8x:\n", rc,currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
                    return;
                }
            }
            else
            {
                if (rc!=GT_NOT_FOUND)
                {
                    PRV_UTF_LOG5_MAC("failed:0x%x!!! delete Ipv6Addr, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrDel.u32Ip[3],
                                                currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                }
            }

            loop_del_v6 = (loop_del_v6 + 1) % max_ipv6;
            currentIp6AddrDel.arIP[15]  = currentIp6AddrDel.arIP[15]  -  (loop_del_v6 % 256);
            currentIp6AddrDel.arIP[14]  = currentIp6AddrDel.arIP[14]  -  ((loop_del_v6 / 256) % 256);

            if((loop_del_v6 % 188) == 144)
            {
                break;
            }
        }
     }
    }

    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomTestScenario function
* @endinternal
*
* @brief   Test scenario
*         A random IP v4/v6 prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=1000
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingRandomTestScenario
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;

    GT_U32 loop_add_v4 = 0;
    GT_U32 loop_del_v4 = 0;
    GT_U32 loop_add_v6 = 0;
    GT_U32 loop_del_v6 = 0;

    GT_U32 numOfIpv4Added = 0;
    GT_U32 numOfIpv4ToDel = 0;
    GT_U32 numOfIpv6Added = 0;
    GT_U32 numOfIpv6ToDel = 0;

    GT_U32 max_ipv4 =2000;
    GT_U32 max_ipv6 =1000;

    GT_U32        loop = 0;
    GT_IPADDR     currentIp4AddrAdd;
    GT_U32        baseIp4Addr=0x30080801;
    GT_IPADDR     currentIp4AddrDel;
    GT_IPV6ADDR   currentIp6AddrAdd;
    GT_U8         baseIp6Addr[16]={0x30,0,0,0,0,0,0,0,0,0,0,0,0,0,0x30,0};
    GT_IPV6ADDR   currentIp6AddrDel;
    GT_U32        prefixLengthIPv6 = 128;
    GT_U32        prefixLengthIPv4 = 32;

    IN  GT_U32    vrId = 0;
    GT_U32        enable_fragmentation = 1;
    GT_IPADDR     currentIp4AddrArray[10];
    GT_U32        prefixLengthIPv4Array[10];
    GT_U32        i=0,loop_times;
    GT_BOOL       override = GT_FALSE;
    GT_BOOL       defragmentationEnable=GT_FALSE;
    GT_U32        routeEntryBaseIndexValue=0;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%12+11;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    /* test senarion for
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=1000
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1*/

    /*
    127.0.0.1/32      0
    192.168.1.25/32   1
    127.0.0.0/8       17
    192.168.1.0/24    18
    */

    /* add the current prefix*/
    currentIp4AddrArray[0].arIP[0]=127;
    currentIp4AddrArray[0].arIP[1]=0;
    currentIp4AddrArray[0].arIP[2]=0;
    currentIp4AddrArray[0].arIP[3]=1;
    prefixLengthIPv4Array[0]=32;

    currentIp4AddrArray[1].arIP[0]=192;
    currentIp4AddrArray[1].arIP[1]=168;
    currentIp4AddrArray[1].arIP[2]=1;
    currentIp4AddrArray[1].arIP[3]=25;
    prefixLengthIPv4Array[1]=32;

    currentIp4AddrArray[2].arIP[0]=127;
    currentIp4AddrArray[2].arIP[1]=0;
    currentIp4AddrArray[2].arIP[2]=0;
    currentIp4AddrArray[2].arIP[3]=0;
    prefixLengthIPv4Array[2]=8;

    currentIp4AddrArray[3].arIP[0]=192;
    currentIp4AddrArray[3].arIP[1]=168;
    currentIp4AddrArray[3].arIP[2]=1;
    currentIp4AddrArray[3].arIP[3]=0;
    prefixLengthIPv4Array[3]=24;

    for (i=0;i<4;i++)
    {
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrArray[i],
                          prefixLengthIPv4Array[i],&nextHopInfo,
                          override,defragmentationEnable);
        if (rc == GT_OK)
        {
            /* uncomment for debugging
            PRV_UTF_LOG4_MAC("Add V4, %d.%d.%d.%d\n",currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
            */
            routeEntryBaseIndexValue++;
            nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%11+11;
        }
        else
        {
            PRV_UTF_LOG5_MAC("FAIL:0x%x,Add V4, %d.%d.%d.%d\n",rc,currentIp4AddrArray[i].arIP[0],currentIp4AddrArray[i].arIP[1],
                         currentIp4AddrArray[i].arIP[2],currentIp4AddrArray[i].arIP[3]);
        }
    }

    currentIp4AddrAdd.u32Ip = baseIp4Addr;
    cpssOsMemCpy(currentIp6AddrAdd.arIP, (void*)baseIp6Addr, sizeof(GT_IPV6ADDR));

    for(loop_times=0;loop_times<PRV_TGF_LOOPS_NUM;loop_times++)
    {
        /* uncomment for debugging
        PRV_UTF_LOG1_MAC("loop_times:%d:\n",loop_times);
        */
        /*random*/
        loop = (cpssOsRand() % 4);

        if(loop == 0)
        {
        /*  add IPv4 prefix from (IPV4ADDR+0), (IPV4ADDR+1). to (IPV4ADDR+n1),
            In the next loop, add IPv4 prefix from (IPV4ADDR+n1+1)  to(IPV4ADDR+n1+n1)
            And the next loop...
            The total IPv4 added less than 2k.
            If the number of IPv4 added up to 2k,go start add IPv4 from (IPV4ADDR+0) */

            /* if no ipv4 was added start from the base address
               if ipv4 was added continue from the last address added */
            if (numOfIpv4Added==0)
            {
                currentIp4AddrAdd.u32Ip = baseIp4Addr;
            }
            else
            {   /* max number of ipv4 was added, need to delete them before we can nontinue adding */
                if (numOfIpv4Added==max_ipv4)
                {
                    continue;
                }
            }
            while(1)
            {
                /* add the current prefix*/
                /* if the prefix we try to add is a MC prefix then skip it */
                if ((currentIp4AddrAdd.arIP[0]&0xF0)!=0xE0)
                {
                    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp4AddrAdd,prefixLengthIPv4,&nextHopInfo,override,defragmentationEnable);
                    if (rc == GT_OK)
                    {
                        numOfIpv4Added++;
                        /* uncomment for debugging
                        PRV_UTF_LOG1_MAC("add Ipv4Addr, 0x%x:\n",currentIp4AddrAdd.u32Ip);
                        */

                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG1_MAC("currentIp4AddrAdd that fail validity,0x%x\n",currentIp4AddrAdd.u32Ip);
                            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
                            return;
                        }
                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG2_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n after ipv4add 0x%x\n", rc,currentIp4AddrAdd.u32Ip);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
                            return;
                        }
                    }
                    else
                    {
                        if ((rc!=GT_ALREADY_EXIST)&& (rc!=GT_FULL)&&(rc!=GT_BAD_PARAM))
                        {
                            PRV_UTF_LOG2_MAC("FAIL:0x%x,add Ipv4Addr, 0x%x:\n", rc, currentIp4AddrAdd.u32Ip);
                        }
                    }
                }

                if (numOfIpv4Added==max_ipv4)
                {
                    /* max number of ipv4 was added, need to delete them before we can nontinue adding */
                    break;
                }
                loop_add_v4 = (loop_add_v4 + 1) % max_ipv4;
                currentIp4AddrAdd.u32Ip=  currentIp4AddrAdd.u32Ip + loop_add_v4;

                if((loop_add_v4 % 122) == 33)
                {
                    break;
                }
            }
        }
        else if(loop == 1)
        {
            /*  delete the ipv4 prefix list addded in loop 0 */
            currentIp4AddrDel.u32Ip = currentIp4AddrAdd.u32Ip;
            if((loop_add_v4!=0)&&(numOfIpv4Added!=0))
            {
               loop_del_v4 = loop_add_v4;
               numOfIpv4ToDel=numOfIpv4Added;
            }
            else
            {
                /* nothing to delete */
                continue;
            }
            while(1)
            {
                /* delete the current prefix */
                /* if the prefix we try to delete is a MC prefix then skip it */
                if ((currentIp4AddrDel.arIP[0]&0xF0)!=0xE0)
                {
                    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,vrId,currentIp4AddrDel,prefixLengthIPv4);
                    if (rc == GT_OK)
                    {
                        numOfIpv4ToDel--;
                        /* uncomment for debugging
                        PRV_UTF_LOG1_MAC("delete Ipv4Addr, 0x%x:\n",currentIp4AddrDel.u32Ip);
                        */
                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG1_MAC("currentIp4AddrDel that fail validity,0x%x\n",currentIp4AddrDel.u32Ip);
                            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
                            return;
                        }
                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG2_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n, after ipv4 del 0x%x\n", rc,currentIp4AddrDel.u32Ip);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
                            return;
                        }
                    }
                    else
                    {
                        if (rc!=GT_NOT_FOUND)
                        {
                            PRV_UTF_LOG2_MAC("FAIL:0x%x,delete Ipv4Addr, 0x%x:\n",rc,currentIp4AddrDel.u32Ip);
                        }
                    }
                }
                if((loop_del_v4==0)||(numOfIpv4ToDel==0))
                {
                    if (numOfIpv4Added==max_ipv4)
                    {
                        loop_add_v4=0;
                    }
                    numOfIpv4Added = 0;
                    break;
                }
                else
                {
                    currentIp4AddrDel.u32Ip= currentIp4AddrDel.u32Ip - loop_del_v4;
                    loop_del_v4--;
                }
            }
        }
        else if(loop == 2)
        {
            /*  add IPv6 prefix from (IPV6ADDR+0), (IPV6ADDR+1) to (IPV6ADDR+n3),
                In the next loop, add IPv6 prefix from (IPV6ADDR+n3+1)  to(IPV6ADDR+n3+n3)
                And the next loop
                The total IPv6 added less than 1k. If the number of IPv6 added up to 1k,go start add IPv6 from  (IPV6ADDR+0), */

            /*  if no ipv6 was added start from the base address
                if ipv6 was added continue from the last address added */
            if (numOfIpv6Added==0)
            {
                cpssOsMemCpy(currentIp6AddrAdd.arIP, (void*)baseIp6Addr, sizeof(GT_IPV6ADDR));
            }
            else
            {   /* max number of ipv6 was added, need to delete them before we can nontinue adding */
                if (numOfIpv6Added==max_ipv6)
                {
                    continue;
                }
            }
            while(1)
            {
                /* add the current prefix*/
                rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrAdd,prefixLengthIPv6,&nextHopInfo,override,enable_fragmentation);
                if (rc == GT_OK)
                {
                    numOfIpv6Added++;
                    /* uncomment for debugging
                    PRV_UTF_LOG5_MAC("add Ipv6Addr, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
                    */
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG4_MAC("currentIp6AddrAdd that fail validity,0x%8x %8x %8x %8x:\n",currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
                        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
                        return;
                    }
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG5_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail afteripv6 add,0x%8x %8x %8x %8x:\n", rc,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
                        return;
                    }
                }
                else
                {
                    if ((rc!=GT_ALREADY_EXIST)&& (rc!=GT_FULL)&&(rc!=GT_BAD_PARAM))
                        PRV_UTF_LOG6_MAC("FAIL:0x%x!!! add Ipv6Addr, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0]);
                }
                if (numOfIpv6Added==max_ipv6)
                {
                    /* max number of ipv6 was added, need to delete them before we can nontinue adding */
                    break;
                }

                loop_add_v6 = (loop_add_v6 + 1) % max_ipv6;
                currentIp6AddrAdd.arIP[15]  = currentIp6AddrAdd.arIP[15]  +  (GT_U8)(loop_add_v6 % 256);
                currentIp6AddrAdd.arIP[14]  = currentIp6AddrAdd.arIP[14]  +  (GT_U8)((loop_add_v6 / 256) % 256);

                if((loop_add_v6 % 166) == 111)
                {
                     break;
                }
            }
        }
        else
        {
            /*  delete the ipv6 prefix list addded in loop 2 */
            cpssOsMemCpy(currentIp6AddrDel.arIP, currentIp6AddrAdd.arIP, sizeof(GT_IPV6ADDR));
            if((loop_add_v6!=0)&&(numOfIpv6Added!=0))
            {
               loop_del_v6 = loop_add_v6;
               numOfIpv6ToDel=numOfIpv6Added;
            }
            else
            {
                /* nothing to delete */
                continue;
            }
            while(1)
            {
                /* delete the current prefix */
                rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId,vrId,currentIp6AddrDel, prefixLengthIPv6);
                if (rc == GT_OK)
                {
                    numOfIpv6ToDel--;
                    /* uncomment for debugging
                    PRV_UTF_LOG4_MAC("delete Ipv6Addr, 0x%8x %8x %8x %8x:\n",currentIp6AddrDel.u32Ip[3],
                                    currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                    */
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG4_MAC("currentIp6AddrDel that fail validity,0x%8x %8x %8x %8x:\n",currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
                        return;
                    }
                   rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG5_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail after ipv6 del,0x%8x %8x %8x %8x:\n", rc,currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
                        return;
                    }
                }
                else
                {
                    if (rc!=GT_NOT_FOUND)
                    {
                        PRV_UTF_LOG5_MAC("failed:0x%x!!! delete Ipv6Addr, 0x%8x %8x %8x %8x:\n", rc, currentIp6AddrDel.u32Ip[3],
                                                    currentIp6AddrDel.u32Ip[2],currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0]);
                    }
                }
                if((loop_del_v6==0)||(numOfIpv6ToDel==0))
                {
                    if (numOfIpv6Added==max_ipv6)
                    {
                        loop_add_v6=0;
                    }

                    /* all ipv6 added in loop 0 were deleted */
                    numOfIpv6Added=0;
                    break;
                }
                else
                {
                    currentIp6AddrDel.arIP[15]  = currentIp6AddrDel.arIP[15]  -  (loop_del_v6 % 256);
                    currentIp6AddrDel.arIP[14]  = currentIp6AddrDel.arIP[14]  -  ((loop_del_v6 / 256) % 256);
                    loop_del_v6--;
                }
            }
        }
    }

    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenarioTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingSpecificCostumerTestScenarioTrafficGenerate
(
    GT_VOID
)
{
    GT_IP_ADDR_TYPE_UNT ipAddr;
    GT_STATUS rc = GT_OK;

    /* AUTODOC: GENERATE TRAFFIC: */
    ipAddr.ipv4Addr.arIP[0]=192;
    ipAddr.ipv4Addr.arIP[1]=168;
    ipAddr.ipv4Addr.arIP[2]=1;
    ipAddr.ipv4Addr.arIP[3]=25;
    prvTgfIpUcRoutingTrafficGenerate(GT_TRUE,&ipAddr);/* ipv4 traffic */

    ipAddr.ipv4Addr.arIP[0]=21;
    ipAddr.ipv4Addr.arIP[1]=1;
    ipAddr.ipv4Addr.arIP[2]=1;
    ipAddr.ipv4Addr.arIP[3]=1;
    prvTgfIpUcRoutingTrafficGenerate(GT_TRUE,&ipAddr);/* ipv4 traffic */

    ipAddr.ipv4Addr.arIP[0]=17;
    prvTgfIpUcRoutingTrafficGenerate(GT_TRUE,&ipAddr);/* ipv4 traffic */

    ipAddr.ipv4Addr.arIP[0]=18;
    prvTgfIpUcRoutingTrafficGenerate(GT_TRUE,&ipAddr);/* ipv4 traffic */

    ipAddr.ipv6Addr.arIP[0]=0x30;
    ipAddr.ipv6Addr.arIP[1]=0;
    ipAddr.ipv6Addr.arIP[2]=0;
    ipAddr.ipv6Addr.arIP[3]=0;
    ipAddr.ipv6Addr.arIP[4]=0;
    ipAddr.ipv6Addr.arIP[5]=0;
    ipAddr.ipv6Addr.arIP[6]=0;
    ipAddr.ipv6Addr.arIP[7]=0;
    ipAddr.ipv6Addr.arIP[8]=0;
    ipAddr.ipv6Addr.arIP[9]=0;
    ipAddr.ipv6Addr.arIP[10]=0;
    ipAddr.ipv6Addr.arIP[11]=0;
    ipAddr.ipv6Addr.arIP[12]=0;
    ipAddr.ipv6Addr.arIP[13]=0;
    ipAddr.ipv6Addr.arIP[14]=0x30;
    ipAddr.ipv6Addr.arIP[15]=5;
    prvTgfIpUcRoutingTrafficGenerate(GT_FALSE,&ipAddr);/* ipv6 traffic */

     /* retrieves configuration of the Virtual Router */
    rc = prvTgfIpLpmVirtualRouterGet(prvTgfLpmDBId, vrId,
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterGet FAILED, rc = [%d]", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterGet");
        return;
    }

    if (prvTgfDefLpmDBVrCfg.isSupportIpv6Mc==GT_FALSE)
    {
        ipAddr.ipv6Addr.arIP[15] = 6;
        prvTgfIpUcRoutingTrafficGenerate(GT_FALSE,&ipAddr);/* ipv6 traffic */
    }
}
/**
* @internal prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioRestore function
* @endinternal
*
* @brief   Clean test scenario from costumer
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragUcRoutingCostumerTestScenarioRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray);

    /* -------------------------------------------------------------------------
     * 1. Restore IP Configuration

     */
    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring IP Configuration =======\n");

    /* retrieves configuration of the Virtual Router */
    rc = prvTgfIpLpmVirtualRouterGet(prvTgfLpmDBId, vrId,
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterGet FAILED, rc = [%d]", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterGet");
        return;
    }

    /* flush all UC Ipv4 prefix */
    if (prvTgfDefLpmDBVrCfg.isSupportIpv4Uc==GT_TRUE)
    {
        rc = prvTgfIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixesFlush");
    }

    /* flush all UC Ipv6 prefix */
    if (prvTgfDefLpmDBVrCfg.isSupportIpv6Uc==GT_TRUE)
    {
        rc = prvTgfIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId, vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixesFlush");
    }

    /* AUTODOC: flush all MC Ipv4 prefixes */
    if (prvTgfDefLpmDBVrCfg.isSupportIpv4Mc==GT_TRUE)
    {
        rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush");
    }

    /* AUTODOC: flush all MC Ipv6 prefixes */
    if (prvTgfDefLpmDBVrCfg.isSupportIpv6Mc==GT_TRUE)
    {
        rc = prvTgfIpLpmIpv6McEntriesFlush(prvTgfLpmDBId, vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntriesFlush");
    }

     /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Restore Base Configuration
     */

    /* fill default destination IP address for packet */
     cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, defIpv4DstAddr, sizeof(prvTgfPacketIpv4Part.dstAddr));
     cpssOsMemCpy(prvTgfPacketIpv6Part.dstAddr, defIpv6DstAddr, sizeof(prvTgfPacketIpv6Part.dstAddr));

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

    /* AUTODOC: invalidate vlan entries [5,6] */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }
    /* clear capturing table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d",
                                 prvTgfDevNum);
}

