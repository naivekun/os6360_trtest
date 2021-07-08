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
* @file prvTgfDynamicPartitionIpLpmMcRouting.c
*
* @brief Checking Dynamic Partition of IPv4/IPv6 MC
*
* @version   1
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

#define UNUSED(x)       ((void)(x))         /* to avoid warnings */

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

/* the Route entry index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 10;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* MLL pair pointer indexes */
static GT_U32        prvTgfMllPointerIndex     = 5;
static GT_U32        prvTgfMllPointerIndex_2   = 7;

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS
};

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 19;

/* virtual router id */
static GT_U32        vrId                      = 0;

/* the MAC address to write to 40 MSBs of MAC SA base addr */
static GT_ETHERADDR  prvTgfMacAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/******************************* Test packet **********************************/
static TGF_IPV4_ADDR  defIpv4DstAddr = {224,1,1,1};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_ipv4Mc =
{
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_ipv6Uc = {
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
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1, 1, 1},                    /* srcAddr = ipSrc */
    { 224,1, 1, 1}                     /* dstAddr = ipGrp */
};

/* DATA of packet len = 22 (0x16) */
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
static TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part_ipv4Mc},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketIpv4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4PartArray                                        /* partsArray */
};

static TGF_IPV6_ADDR  defIpv6DstAddr = {0x2001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1234};

static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypeIpv6Part =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

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


/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv6PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part_ipv6Uc},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypeIpv6Part},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)


/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo_ipv6 =
{
    PRV_TGF_IPV6_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketIpv6PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv6PartArray                                        /* partsArray */
};

/******************************* IPV6 MC Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_ipv6Mc = {
    {0x33, 0x33, 0x00, 0x00, 0x44, 0xff},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};


/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part_ipv6Mc = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x1111, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x44FF}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_ipv6Mc[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part_ipv6Mc},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypeIpv6Part},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part_ipv6Mc},
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
* @internal prvTgfDynamicPartitionIpLpmDbMcConfigurationSet function
* @endinternal
*
* @brief   Set IP LPM DB Base Configuration
*
* @param[in] lastLineIndex            - index of the last line in the TCAM,
*                                      used to create the new LPP DB
*                                       None
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbMcConfigurationSet
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

    /* cpssDxChIpLpmDBCreate with lpmDbId[19], partitionEnable= FALSE,(lastLineIndex+1)
       TCAM lines ((lastLineIndex+1)*4 tcam cells) and other valid parameters */
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
* @internal prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet
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
* @internal prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Base Configuration
*
* @param[in] protocol                 -  to use in prvTgfIpMcRouteEntriesWrite, affects mll definition
*                                       None
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet
(
     IN CPSS_IP_PROTOCOL_STACK_ENT protocol
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray;

    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC             *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Route Configuration =======\n");
    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast/Multicast IPv4\IPv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4\IPv6 Unicast/Multicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: sets 40 MSBs of MAC SA base addr to 00:00:00:00:00 */
    rc = prvTgfIpRouterMacSaBaseSet(prvTgfDevNum, &prvTgfMacAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseSet");

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: create MC Link List with index 5, port 2, VLAN 80 */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_NEXTHOPE_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;
    mllPairEntry.nextPointer                     = 0;

    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex;

    /* AUTODOC: add default MC route entry with index 10 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

    /* AUTODOC: create MC Link List with index 7, port 3, VLAN 80 */
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex_2,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    mcRouteEntryArray[0].ipv4MllPtr = prvTgfMllPointerIndex_2;
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex_2;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex_2;

    /* AUTODOC: add MC route entry with index 12 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex+2, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");


    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table for
     *    UC ipv6 entries usage
     */

    /* write a ARP MAC address to the Router ARP Table */
    arpMacAddr.arEther[0] = 0;
    arpMacAddr.arEther[1] = 0;
    arpMacAddr.arEther[2] = 0;
    arpMacAddr.arEther[3] = 0;
    arpMacAddr.arEther[4] = 0x22;
    arpMacAddr.arEther[5] = (GT_U8)(prvTgfRouteEntryBaseIndex+1);/* value 11 */

    /* AUTODOC: write ARP MACs 00:00:00:00:22:0b to ARP Table index 11 */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouteEntryBaseIndex+1, &arpMacAddr);
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
    routeEntriesArray.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
    routeEntriesArray.nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    routeEntriesArray.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    routeEntriesArray.nextHopARPPointer          = prvTgfRouteEntryBaseIndex+1;
    routeEntriesArray.nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entries with: */
    /* AUTODOC: cmd=CMD_ROUTE, nextHopVlan=6 */
    /* AUTODOC: nextHopPorts=23 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex+1, &routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     *  Set the FDB entry With DA_ROUTE, for UC ipv6 entries usage
     */

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part_ipv6Uc.daMac, sizeof(TGF_MAC_ADDR));

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
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4McRouting function
* @endinternal
*
* @brief   A specific test that fit a 6 lines TCAM, the test add and delete prefixes
*         in such a way that we will have move of prefixes location when adding new
*         entries.
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4McRouting
(
    GT_VOID
)
{
        GT_STATUS       rc = GT_OK;

        IN  GT_U32      vrId = 0;
        GT_U32          enable_fragmentation = 1;
        GT_IPADDR       currentIp4GrpAddrArray[10];
        GT_U32          prefixLengthIPv4GrpArray[10];
        GT_IPADDR       currentIp4SrcAddrArray[10];
        GT_U32          prefixLengthIPv4SrcArray[10];

        GT_U8           i=0;

        GT_IPV6ADDR     currentIp6AddrArray[10];
        GT_U32          prefixLengthIPv6Array[10];
        GT_BOOL         override = GT_FALSE;

        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
        PRV_TGF_IP_LTT_ENTRY_STC              mcRouteLttEntry;

        /* AUTODOC: SETUP CONFIGURATION: */
        PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

        cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+1;
        nextHopInfo.ipLttEntry.numOfPaths          = 0;
        nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
        mcRouteLttEntry.numOfPaths          = 0;
        mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        /* test senarion for
        lpmDbPartitionEnable=0x0
        lpmDbFirstTcamLine=0
        lpmDbLastTcamLine=5
        maxNumOfIpv4Prefixes=1
        maxNumOfIpv4McEntries=1
        maxNumOfIpv6Prefixes=1*/

       /*
        add prefixes:
        group 225.1.1.1/32
        Src   1.1.1.1/32
        group 225.1.1.1/30
        Src   1.1.1.1/30
        group 225.1.1.1/28
        Src   1.1.1.1/28
        group 225.1.1.1/26
        Src   1.1.1.1/26
        group 225.1.1.1/24
        Src   1.1.1.1/24
        group 225.1.1.1/22
        Src   1.1.1.1/22
        group 225.1.1.1/20
        Src   1.1.1.1/20

        ---------------------------------------------------------------------------------------------
                      column 0              column1          column2              column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32   |    1.1.1.1/32      |   1.1.1.0/30       |   1.1.1.0/28
        ---------------------------------------------------------------------------------------------
        line 1      1.1.1.0/22     |    225.1.1.0/30    |   1.1.1.0/24       |   1.1.1.0/26
        ---------------------------------------------------------------------------------------------
        line 2      1.1.0.0/20     |    225.1.1.0/28    |   225.1.1.0/26     |   225.1.1.0/24
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 5      225.1.1.0/22   |    225.1.0.0/20    |   224.0.0.0/4(def) |   0/0(def)
        ---------------------------------------------------------------------------------------------

       */

    /* add the current prefix*/
    currentIp4GrpAddrArray[0].arIP[0]=225;
    currentIp4GrpAddrArray[0].arIP[1]=1;
    currentIp4GrpAddrArray[0].arIP[2]=1;
    currentIp4GrpAddrArray[0].arIP[3]=1;
    prefixLengthIPv4GrpArray[0]=32;

    currentIp4SrcAddrArray[0].arIP[0]=1;
    currentIp4SrcAddrArray[0].arIP[1]=1;
    currentIp4SrcAddrArray[0].arIP[2]=1;
    currentIp4SrcAddrArray[0].arIP[3]=1;
    prefixLengthIPv4SrcArray[0]=32;

    for (i=0;i<7;i++)
    {
        /* for group 225.1.1.1/24 Src 1.1.1.1/24 we want the egress port to be 23 and not 18
           routeEntryBaseIndex=12 --> mll 7 --> egress port 23 */
        if ((prefixLengthIPv4GrpArray[0]-(i*2))==24)
        {
            mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex + 2;
        }

        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, vrId,
                                        currentIp4GrpAddrArray[0],(prefixLengthIPv4GrpArray[0]-(i*2)),
                                        currentIp4SrcAddrArray[0],(prefixLengthIPv4SrcArray[0]-(i*2)),
                                        &mcRouteLttEntry,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* prints used for debug - in case of fail add this code
            PRV_UTF_LOG5_MAC("Add V4, group %d.%d.%d.%d/%d,",
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             (prefixLengthIPv4GrpArray[0]-(i*2)));
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             (prefixLengthIPv4SrcArray[0]-(i*2)));
           */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x,Add V4, group %d.%d.%d.%d/%d,",rc,
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             (prefixLengthIPv4GrpArray[0]-(i*2)));
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             (prefixLengthIPv4SrcArray[0]-(i*2)));
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
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }
    }
    /* the following code is for debug purpose so if this test fails we can remove
       the code from remark and use the print to help us debug the problem */
#if 0
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = dumpRouteTcam(GT_TRUE);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
#endif

    /* delete entry grp=225.1.1.0/26 , src=1.1.1.1/26

        ---------------------------------------------------------------------------------------------
                      column 0              column1          column2              column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32   |    1.1.1.1/32      |   1.1.1.0/30       |   1.1.1.0/28
        ---------------------------------------------------------------------------------------------
        line 1      1.1.1.0/22     |    225.1.1.0/30    |   1.1.1.0/24       |---- empty cell----
        ---------------------------------------------------------------------------------------------
        line 2      1.1.0.0/20     |    225.1.1.0/28    |---- empty cell---- |   225.1.1.0/24
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 5      225.1.1.0/22   |    225.1.0.0/20    |   224.0.0.0/4(def) |   0/0(def)
        ---------------------------------------------------------------------------------------------
    */

    currentIp4GrpAddrArray[0].arIP[0]=225;
    currentIp4GrpAddrArray[0].arIP[1]=1;
    currentIp4GrpAddrArray[0].arIP[2]=1;
    currentIp4GrpAddrArray[0].arIP[3]=1;
    prefixLengthIPv4GrpArray[0]=26;

    currentIp4SrcAddrArray[0].arIP[0]=1;
    currentIp4SrcAddrArray[0].arIP[1]=1;
    currentIp4SrcAddrArray[0].arIP[2]=1;
    currentIp4SrcAddrArray[0].arIP[3]=1;
    prefixLengthIPv4SrcArray[0]=26;

    rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, vrId,
                                    currentIp4GrpAddrArray[0],prefixLengthIPv4GrpArray[0],
                                    currentIp4SrcAddrArray[0],prefixLengthIPv4SrcArray[0]);
    if (rc == GT_OK)
    {
        /*
        PRV_UTF_LOG5_MAC("Del V4, group %d.%d.%d.%d/%d,",
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         prefixLengthIPv4GrpArray[0]);
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         prefixLengthIPv4SrcArray[0]);
       */
    }
    else
    {
        PRV_UTF_LOG6_MAC("FAIL:0x%x,Del V4, group %d.%d.%d.%d/%d,",rc,
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         prefixLengthIPv4GrpArray[0]);
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         prefixLengthIPv4SrcArray[0]);
    }

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 1");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 2");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 3");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 4");
    }

    /* the following code is for debug purpose so if this test fails we can remove
       the code from remark and use the print to help us debug the problem */
#if 0
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 5");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 6");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 7");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 8");
    }
    rc = dumpRouteTcam(GT_TRUE);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 9");
    }
#endif

    /* add entry grp=225.1.1.0/23 , src=1.1.1.1/23

        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                       column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32 |  1.1.1.1/32   |   1.1.1.0/30                |   1.1.1.0/28
        ---------------------------------------------------------------------------------------------
        line 1      1.1.1.0/22   |  225.1.1.0/30 |   1.1.1.0/23                |   1.1.1.0/24
                                 |               |  (new entry added)          |(moved from line_1/column_2)
        ---------------------------------------------------------------------------------------------
        line 2      1.1.0.0/20   |  225.1.1.0/28 | 225.1.1.0/24                | 225.1.1.0/23
                                 |               |(moved from line_2/column_3) | (new entry added)
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 5      225.1.1.0/22 |  225.1.0.0/20 |   224.0.0.0/4(def)          |   0/0(def)
        ---------------------------------------------------------------------------------------------

    */
    currentIp4GrpAddrArray[0].arIP[0]=225;
    currentIp4GrpAddrArray[0].arIP[1]=1;
    currentIp4GrpAddrArray[0].arIP[2]=1;
    currentIp4GrpAddrArray[0].arIP[3]=1;
    prefixLengthIPv4GrpArray[0]=23;

    currentIp4SrcAddrArray[0].arIP[0]=1;
    currentIp4SrcAddrArray[0].arIP[1]=1;
    currentIp4SrcAddrArray[0].arIP[2]=1;
    currentIp4SrcAddrArray[0].arIP[3]=1;
    prefixLengthIPv4SrcArray[0]=23;

    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, vrId,
                                    currentIp4GrpAddrArray[0],prefixLengthIPv4GrpArray[0],
                                    currentIp4SrcAddrArray[0],prefixLengthIPv4SrcArray[0],
                                    &mcRouteLttEntry,override,enable_fragmentation);
    if (rc == GT_OK)
    {
        /* prints used for debug - in case of fail add this code
        PRV_UTF_LOG5_MAC("Add V4, group %d.%d.%d.%d/%d,",
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         (prefixLengthIPv4GrpArray[0]-(i*2)));
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         (prefixLengthIPv4SrcArray[0]-(i*2)));
        */
    }
    else
    {
        PRV_UTF_LOG6_MAC("FAIL:0x%x,Add V4, group %d.%d.%d.%d/%d,",rc,
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         (prefixLengthIPv4GrpArray[0]-(i*2)));
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         (prefixLengthIPv4SrcArray[0]-(i*2)));
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }

    /* the following code is for debug purpose so if this test fails we can remove
       the code from remark and use the print to help us debug the problem */
#if 0
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = dumpRouteTcam(GT_TRUE);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
#endif

   /* delete entry grp=225.1.1.0/30 , src=1.1.1.1/30
      delete entry grp=225.1.1.0/28 , src=1.1.1.1/28

        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                       column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32 |  1.1.1.1/32   |   empty cell                |   empty cell
        ---------------------------------------------------------------------------------------------
        line 1      1.1.1.0/22   |  empty cell   |   1.1.1.0/23                |   1.1.1.0/24
        ---------------------------------------------------------------------------------------------
        line 2      1.1.0.0/20   |  empty cell   |   225.1.1.0/24              | 225.1.1.0/23
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 5      225.1.1.0/22 |  225.1.0.0/20 |   224.0.0.0/4(def)          |   0/0(def)
        ---------------------------------------------------------------------------------------------

   */
    for (i=0;i<2;i++)
    {
        currentIp4GrpAddrArray[0].arIP[0]=225;
        currentIp4GrpAddrArray[0].arIP[1]=1;
        currentIp4GrpAddrArray[0].arIP[2]=1;
        currentIp4GrpAddrArray[0].arIP[3]=1;
        prefixLengthIPv4GrpArray[0]=30;

        currentIp4SrcAddrArray[0].arIP[0]=1;
        currentIp4SrcAddrArray[0].arIP[1]=1;
        currentIp4SrcAddrArray[0].arIP[2]=1;
        currentIp4SrcAddrArray[0].arIP[3]=1;
        prefixLengthIPv4SrcArray[0]=30;


        rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, vrId,
                                        currentIp4GrpAddrArray[0],(prefixLengthIPv4GrpArray[0]-(i*2)),
                                        currentIp4SrcAddrArray[0],(prefixLengthIPv4SrcArray[0]-(i*2)));
        if (rc == GT_OK)
        {
            /* prints used for debug - in case of fail add this code
            PRV_UTF_LOG5_MAC("Del V4, group %d.%d.%d.%d/%d,",
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             prefixLengthIPv4GrpArray[0]);
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             prefixLengthIPv4SrcArray[0]);
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x,Del V4, group %d.%d.%d.%d/%d,",rc,
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             prefixLengthIPv4GrpArray[0]);
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             prefixLengthIPv4SrcArray[0]);
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 1");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 2");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 3");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 4");
        }

        /* the following code is for debug purpose so if this test fails we can remove
           the code from remark and use the print to help us debug the problem */
#if 0
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 5");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 6");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 7");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 8");
        }
        rc = dumpRouteTcam(GT_TRUE);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 9");
        }
#endif
    }
    /*
       Add V6 3000::3000 will be added in line


        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                   column3
        ---------------------------------------------------------------------------------------------
        line 0      new ipv6 UC entry added 3000: :3000
        ---------------------------------------------------------------------------------------------
        line 1      1.1.1.0/22   |  225.1.1.1/32    |   1.1.1.0/23        |   1.1.1.0/24
                                 |  (moved from     |                     |
                                 |  line_0/column_0)|                     |
        ---------------------------------------------------------------------------------------------
        line 2      1.1.0.0/20   |  1.1.1.1/32      | 225.1.1.0/24        |   225.1.1.0/23
                                 | (moved from      |                     |
                                 | line_0/column_1) |                     |
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 5      225.1.1.0/22 |  225.1.0.0/20 |   224.0.0.0/4(def)     |   0/0(def)
        ---------------------------------------------------------------------------------------------

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

    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+1;
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
    if (rc == GT_OK)
    {
        /* prints used for debug - in case of fail add this code
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        return;
    }

    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv6McRouting function
* @endinternal
*
* @brief   A specific test that fit a 7 lines TCAM, the test add and delete prefixes
*         in such a way that we will have move of prefixes location when adding new
*         entries.
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv6McRouting
(
    GT_VOID
)
{
        GT_STATUS       rc = GT_OK;

        IN  GT_U32      vrId = 0;
        GT_U32          enable_fragmentation = 1;
        GT_IPADDR       currentIp4GrpAddrArray[10];
        GT_U32          prefixLengthIPv4GrpArray[10];
        GT_IPADDR       currentIp4SrcAddrArray[10];
        GT_U32          prefixLengthIPv4SrcArray[10];

        GT_U8           i=0;

        GT_IPV6ADDR     currentIp6AddrArray[10];
        GT_U32          prefixLengthIPv6Array[10];
        GT_IPV6ADDR     currentIp6SrcAddrArray[10];
        GT_U32          prefixLengthIPv6SrcArray[10];
        GT_BOOL         override = GT_FALSE;

        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
        PRV_TGF_IP_LTT_ENTRY_STC              mcRouteLttEntry;

        /* AUTODOC: SETUP CONFIGURATION: */

        /* need the same configuration done for ipv4Mc test, after the call to the
           prvTgfDynamicPartitionIpLpmDefragIpv4McRouting function we will have the
           following TCAM arrangement


        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                   column3
        ---------------------------------------------------------------------------------------------
        line 0      255.1.1.1/32 |  1.1.1.1/32      |                     |
        ---------------------------------------------------------------------------------------------
        line 1      1.1.0.0/22   |                  |                     |   1.1.1.0/24
        ---------------------------------------------------------------------------------------------
        line 2      new ipv6 UC entry added 3000: :3000/128
        ---------------------------------------------------------------------------------------------
        line 3      1.1.0.0/20   |  1.1.0.0/23      | 225.1.1.0/24        |   225.1.1.0/23
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 5      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 6      225.1.1.0/22 |  225.1.0.0/20     |   224.0.0.0/4(def)     |   0/0(def)
        ---------------------------------------------------------------------------------------------


           */
        prvTgfDynamicPartitionIpLpmDefragIpv4McRouting();


        PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

        cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+1;
        nextHopInfo.ipLttEntry.numOfPaths          = 0;

        nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
        mcRouteLttEntry.numOfPaths          = 0;
        mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        /* test senarion for
        lpmDbPartitionEnable=0x0
        lpmDbFirstTcamLine=0
        lpmDbLastTcamLine=6
        maxNumOfIpv4Prefixes=1
        maxNumOfIpv4McEntries=1
        maxNumOfIpv6Prefixes=1*/

        /* delete entry grp=225.1.1.0/32 , src=1.1.1.1/32
                        grp=225.1.1.0/22 , src=1.1.1.1/22

        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                   column3
        ---------------------------------------------------------------------------------------------
        line 0                   |                  |                     |
        ---------------------------------------------------------------------------------------------
        line 1                   |                  |                     |   1.1.1.0/24
        ---------------------------------------------------------------------------------------------
        line 2      new ipv6 UC entry added 3000: :3000/128
        ---------------------------------------------------------------------------------------------
        line 3      1.1.0.0/20   |  1.1.0.0/23      | 225.1.1.0/24        |   225.1.1.0/23
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 5      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 6                   |  225.1.0.0/20     |   224.0.0.0/4(def)     |   0/0(def)
        ---------------------------------------------------------------------------------------------
        */

        currentIp4GrpAddrArray[0].arIP[0]=225;
        currentIp4GrpAddrArray[0].arIP[1]=1;
        currentIp4GrpAddrArray[0].arIP[2]=1;
        currentIp4GrpAddrArray[0].arIP[3]=1;
        prefixLengthIPv4GrpArray[0]=32;

        currentIp4SrcAddrArray[0].arIP[0]=1;
        currentIp4SrcAddrArray[0].arIP[1]=1;
        currentIp4SrcAddrArray[0].arIP[2]=1;
        currentIp4SrcAddrArray[0].arIP[3]=1;
        prefixLengthIPv4SrcArray[0]=32;

        for (i=0;i<2;i++)
        {
            rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, vrId,
                                            currentIp4GrpAddrArray[0],prefixLengthIPv4GrpArray[0]-(i*10),
                                            currentIp4SrcAddrArray[0],prefixLengthIPv4SrcArray[0]-(i*10));
            if (rc == GT_OK)
            {
                /* prints used for debug - in case of fail add this code
                PRV_UTF_LOG5_MAC("Del V4, group %d.%d.%d.%d/%d,",
                                 currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                                 currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                                 prefixLengthIPv4GrpArray[0]-(i*10));
                PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                                 currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                                 currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                                 prefixLengthIPv4SrcArray[0]-(i*10));
                */
            }
            else
            {
                PRV_UTF_LOG6_MAC("FAIL:0x%x,Del V4, group %d.%d.%d.%d/%d,",rc,
                                 currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                                 currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                                 prefixLengthIPv4GrpArray[0]-(i*10));
                PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                                 currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                                 currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                                 prefixLengthIPv4SrcArray[0]-(i*10));
            }

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 1");
            }
            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 2");
            }
            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 3");
            }
            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 4");
            }
        }

        /* the following code is for debug purpose so if this test fails we can remove
           the code from remark and use the print to help us debug the problem */
    #if 0
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 5");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 6");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 7");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 8");
        }
        rc = dumpRouteTcam(GT_TRUE);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 9");
        }
    #endif

    /* add new ipv6 MC entry group FF00::44FF/127 src 1111::2222/127

        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                   column3
        ---------------------------------------------------------------------------------------------
        line 0       IPv6 MC group    FF00::44FF/127
        ---------------------------------------------------------------------------------------------
        line 1       IPv6 MC Src      1111::2222/127
        ---------------------------------------------------------------------------------------------
        line 2      new ipv6 UC entry added 3000: :3000/128
        ---------------------------------------------------------------------------------------------
        line 3      1.1.0.0/20   |  1.1.0.0/23      | 225.1.1.0/24        |   225.1.1.0/23
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 5      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 6      1.1.1.0/24   |  225.1.0.0/20     |   224.0.0.0/4(def) |   0/0(def)
                    moved from   |                   |                    |
                    line0/column3|                   |                    |
        ---------------------------------------------------------------------------------------------


    */
    currentIp6AddrArray[0].arIP[0]=0xFF;
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
    currentIp6AddrArray[0].arIP[14]=0x44;
    currentIp6AddrArray[0].arIP[15]=0xff;
    prefixLengthIPv6Array[0]=127;

    currentIp6SrcAddrArray[0].arIP[0]=0x11;
    currentIp6SrcAddrArray[0].arIP[1]=0x11;
    currentIp6SrcAddrArray[0].arIP[2]=0;
    currentIp6SrcAddrArray[0].arIP[3]=0;
    currentIp6SrcAddrArray[0].arIP[4]=0;
    currentIp6SrcAddrArray[0].arIP[5]=0;
    currentIp6SrcAddrArray[0].arIP[6]=0;
    currentIp6SrcAddrArray[0].arIP[7]=0;
    currentIp6SrcAddrArray[0].arIP[8]=0;
    currentIp6SrcAddrArray[0].arIP[9]=0;
    currentIp6SrcAddrArray[0].arIP[10]=0;
    currentIp6SrcAddrArray[0].arIP[11]=0;
    currentIp6SrcAddrArray[0].arIP[12]=0;
    currentIp6SrcAddrArray[0].arIP[13]=0;
    currentIp6SrcAddrArray[0].arIP[14]=0x22;
    currentIp6SrcAddrArray[0].arIP[15]=0x22;
    prefixLengthIPv6SrcArray[0]=127;

    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;

    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],
                                   currentIp6SrcAddrArray[0],prefixLengthIPv6SrcArray[0],
                                   &mcRouteLttEntry,override,enable_fragmentation);

    if (rc == GT_OK)
    {
        /* prints used for debug - in case of fail add this code
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        return;
    }

    /* delete entry grp=225.1.1.0/20 , src=1.1.1.1/20
                    grp=225.1.1.0/23 , src=1.1.1.1/23
                    grp=225.1.1.0/24 , src=1.1.1.1/24
       delete ipv6 uc 3000: :3000/128

        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                   column3
        ---------------------------------------------------------------------------------------------
        line 0       IPv6 MC group    FF00::44FF/127
        ---------------------------------------------------------------------------------------------
        line 1       IPv6 MC Src      1111::2222/127
        ---------------------------------------------------------------------------------------------
        line 2
        ---------------------------------------------------------------------------------------------
        line 3                   |                   |                     |
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 5      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 6                   |                    |   224.0.0.0/4(def) |   0/0(def)
        ---------------------------------------------------------------------------------------------

        */

        currentIp4GrpAddrArray[0].arIP[0]=225;
        currentIp4GrpAddrArray[0].arIP[1]=1;
        currentIp4GrpAddrArray[0].arIP[2]=1;
        currentIp4GrpAddrArray[0].arIP[3]=1;
        prefixLengthIPv4GrpArray[0]=24;

        currentIp4SrcAddrArray[0].arIP[0]=1;
        currentIp4SrcAddrArray[0].arIP[1]=1;
        currentIp4SrcAddrArray[0].arIP[2]=1;
        currentIp4SrcAddrArray[0].arIP[3]=1;
        prefixLengthIPv4SrcArray[0]=24;

        for (i=0;i<5;i++)
        {
            /* do not delete prefixes with prefixLength 22 and 21*/
            if (i==2||i==3)
            {
                continue;
            }
            rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, vrId,
                                            currentIp4GrpAddrArray[0],prefixLengthIPv4GrpArray[0]-i,
                                            currentIp4SrcAddrArray[0],prefixLengthIPv4SrcArray[0]-i);
            if (rc == GT_OK)
            {
                /* prints used for debug - in case of fail add this code
                PRV_UTF_LOG5_MAC("Del V4, group %d.%d.%d.%d/%d,",
                                 currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                                 currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                                 prefixLengthIPv4GrpArray[0]-i);
                PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                                 currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                                 currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                                 prefixLengthIPv4SrcArray[0]-i);
                */
            }
            else
            {
                PRV_UTF_LOG6_MAC("FAIL:0x%x,Del V4, group %d.%d.%d.%d/%d,",rc,
                                 currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                                 currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                                 prefixLengthIPv4GrpArray[0]-i);
                PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                                 currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                                 currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                                 prefixLengthIPv4SrcArray[0]-i);
            }

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 1");
            }
            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 2");
            }
            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 3");
            }
            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 4");
            }
        }

        /* delete ipv6 uc entry*/
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

        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+1;
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0]);
        if (rc == GT_OK)
        {
             /* prints used for debug - in case of fail add this code
            PRV_UTF_LOG5_MAC("Del V6, enable_fragmentation:%d, 0x%8x %8x %8x %8x:\n",enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
             */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x!!! Del V6, enable_fragmentation:%d,0x%8x %8x %8x %8x:\n",rc,enable_fragmentation,currentIp6AddrArray[0].u32Ip[3],currentIp6AddrArray[0].u32Ip[2],currentIp6AddrArray[0].u32Ip[1],currentIp6AddrArray[0].u32Ip[0]);
        }

        /* the following code is for debug purpose so if this test fails we can remove
           the code from remark and use the print to help us debug the problem */
    #if 0
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 5");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 6");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 7");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 8");
        }
        rc = dumpRouteTcam(GT_TRUE);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 9");
        }
    #endif

        /*  add new ipv6 MC entry group FF00::44FF/128 src 1111::2222/128

        ---------------------------------------------------------------------------------------------
                     column 0       column1          column2                   column3
        ---------------------------------------------------------------------------------------------
        line 0       IPv6 MC group    FF00::44FF/128    new group entry
        ---------------------------------------------------------------------------------------------
        line 1       IPv6 MC Src      1111::2222/128    new src entry
        ---------------------------------------------------------------------------------------------
        line 2       IPv6 MC group    FF00::44FF/127    moved from line 0
        ---------------------------------------------------------------------------------------------
        line 3       IPv6 MC Src      1111::2222/127    moved from line 1
        ---------------------------------------------------------------------------------------------
        line 4      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 5      ipv6 default UC entry
        ---------------------------------------------------------------------------------------------
        line 6                      |               |   224.0.0.0/4(def)    |   0/0(def)
        ---------------------------------------------------------------------------------------------

        */

        currentIp6AddrArray[0].arIP[0]=0xFF;
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
        currentIp6AddrArray[0].arIP[14]=0x44;
        currentIp6AddrArray[0].arIP[15]=0xff;
        prefixLengthIPv6Array[0]=128;

        currentIp6SrcAddrArray[0].arIP[0]=0x11;
        currentIp6SrcAddrArray[0].arIP[1]=0x11;
        currentIp6SrcAddrArray[0].arIP[2]=0;
        currentIp6SrcAddrArray[0].arIP[3]=0;
        currentIp6SrcAddrArray[0].arIP[4]=0;
        currentIp6SrcAddrArray[0].arIP[5]=0;
        currentIp6SrcAddrArray[0].arIP[6]=0;
        currentIp6SrcAddrArray[0].arIP[7]=0;
        currentIp6SrcAddrArray[0].arIP[8]=0;
        currentIp6SrcAddrArray[0].arIP[9]=0;
        currentIp6SrcAddrArray[0].arIP[10]=0;
        currentIp6SrcAddrArray[0].arIP[11]=0;
        currentIp6SrcAddrArray[0].arIP[12]=0;
        currentIp6SrcAddrArray[0].arIP[13]=0;
        currentIp6SrcAddrArray[0].arIP[14]=0x22;
        currentIp6SrcAddrArray[0].arIP[15]=0x22;
        prefixLengthIPv6SrcArray[0]=128;

        mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+2;

        rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],
                                       currentIp6SrcAddrArray[0],prefixLengthIPv6SrcArray[0],
                                       &mcRouteLttEntry,override,enable_fragmentation);

        if (rc == GT_OK)
        {
             /* prints used for debug - in case of fail add this code
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
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
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
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
            return;
        }

        /* the following code is for debug purpose so if this test fails we can remove
           the code from remark and use the print to help us debug the problem */
#if 0
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 5");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 6");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 7");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 8");
        }
        rc = dumpRouteTcam(GT_TRUE);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 9");
        }
#endif
    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingMovingDefault function
* @endinternal
*
* @brief   A specific test that fit a 6 lines TCAM, the test add and delete prefixes
*         in such a way that we will have move of prefixes location when adding new
*         entries.
*         The scenario in built in a way that the default entry must be moved.
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingMovingDefault
(
    GT_VOID
)
{
        GT_STATUS       rc = GT_OK;

        IN  GT_U32      vrId = 0;
        GT_U32          enable_fragmentation = 1;
        GT_IPADDR       currentIp4GrpAddrArray[10];
        GT_U32          prefixLengthIPv4GrpArray[10];
        GT_IPADDR       currentIp4SrcAddrArray[10];
        GT_U32          prefixLengthIPv4SrcArray[10];

        GT_U8           i=0;

        GT_IPV6ADDR     currentIp6AddrArray[10];
        GT_U32          prefixLengthIPv6Array[10];
        GT_BOOL         override = GT_FALSE;

        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
        PRV_TGF_IP_LTT_ENTRY_STC              mcRouteLttEntry;

        /* AUTODOC: SETUP CONFIGURATION: */
        PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

        cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+1;
        nextHopInfo.ipLttEntry.numOfPaths          = 0;

        nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
        mcRouteLttEntry.numOfPaths          = 0;
        mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        /* test senarion for
        lpmDbPartitionEnable=0x0
        lpmDbFirstTcamLine=0
        lpmDbLastTcamLine=4
        maxNumOfIpv4Prefixes=1
        maxNumOfIpv4McEntries=1
        maxNumOfIpv6Prefixes=1*/

       /*
        add prefixes:
        group 225.1.1.1/32
        Src   1.1.1.1/32
        group 225.1.1.1/30
        Src   1.1.1.1/30
        group 225.1.1.1/28
        Src   1.1.1.1/28
        group 225.1.1.1/26
        Src   1.1.1.1/26
        group 225.1.1.1/24
        Src   1.1.1.1/24

        ---------------------------------------------------------------------------------------------
                      column 0              column1          column2              column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32   |    1.1.1.1/32      |   225.1.1.0/30     |   1.1.1.0/30
        ---------------------------------------------------------------------------------------------
        line 1      1.1.1.0/26     |    225.1.1.0/28    |   1.1.1.0/28       |   1.1.1.0/24
        ---------------------------------------------------------------------------------------------
        line 2      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default UC entry
        --------------------------------------------------------------------------------------------
        line 4      225.1.1.0/26   |    225.1.0.0/24    |   224.0.0.0/4(def) |   0/0(def)
        ---------------------------------------------------------------------------------------------

       */

    /* add the current prefix*/
    currentIp4GrpAddrArray[0].arIP[0]=225;
    currentIp4GrpAddrArray[0].arIP[1]=1;
    currentIp4GrpAddrArray[0].arIP[2]=1;
    currentIp4GrpAddrArray[0].arIP[3]=1;
    prefixLengthIPv4GrpArray[0]=32;

    currentIp4SrcAddrArray[0].arIP[0]=1;
    currentIp4SrcAddrArray[0].arIP[1]=1;
    currentIp4SrcAddrArray[0].arIP[2]=1;
    currentIp4SrcAddrArray[0].arIP[3]=1;
    prefixLengthIPv4SrcArray[0]=32;

    for (i=0;i<5;i++)
    {
        /* for group 225.1.1.1/24 Src 1.1.1.1/24 we want the egress port to be 23 and not 18
           routeEntryBaseIndex=12 --> mll 7 --> egress port 23 */
        if ((prefixLengthIPv4GrpArray[0]-(i*2))==24)
        {
            mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex + 2;
        }

        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, vrId,
                                        currentIp4GrpAddrArray[0],(prefixLengthIPv4GrpArray[0]-(i*2)),
                                        currentIp4SrcAddrArray[0],(prefixLengthIPv4SrcArray[0]-(i*2)),
                                        &mcRouteLttEntry,override,enable_fragmentation);
        if (rc == GT_OK)
        {
            /* prints used for debug - in case of fail add this code
            PRV_UTF_LOG5_MAC("Add V4, group %d.%d.%d.%d/%d,",
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             (prefixLengthIPv4GrpArray[0]-(i*2)));
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             (prefixLengthIPv4SrcArray[0]-(i*2)));
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x,Add V4, group %d.%d.%d.%d/%d,",rc,
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             (prefixLengthIPv4GrpArray[0]-(i*2)));
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             (prefixLengthIPv4SrcArray[0]-(i*2)));
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
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        }
    }
    /* the following code is for debug purpose so if this test fails we can remove
       the code from remark and use the print to help us debug the problem */
#if 0
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = dumpRouteTcam(GT_TRUE);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
#endif

    /* delete entry grp=225.1.1.0/26 , src=1.1.1.1/26

        ---------------------------------------------------------------------------------------------
                      column 0              column1          column2              column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32   |    1.1.1.1/32      |   225.1.1.0/30     |   1.1.1.0/30
        ---------------------------------------------------------------------------------------------
        line 1      --empty cell-- |    225.1.1.0/28    |   1.1.1.0/28       |   1.1.1.0/24
        ---------------------------------------------------------------------------------------------
        line 2      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default UC entry
        --------------------------------------------------------------------------------------------
        line 4      --empty cell-- |    225.1.0.0/24    |   224.0.0.0/4(def) |   0/0(def)
        ---------------------------------------------------------------------------------------------

    */

    currentIp4GrpAddrArray[0].arIP[0]=225;
    currentIp4GrpAddrArray[0].arIP[1]=1;
    currentIp4GrpAddrArray[0].arIP[2]=1;
    currentIp4GrpAddrArray[0].arIP[3]=1;
    prefixLengthIPv4GrpArray[0]=26;

    currentIp4SrcAddrArray[0].arIP[0]=1;
    currentIp4SrcAddrArray[0].arIP[1]=1;
    currentIp4SrcAddrArray[0].arIP[2]=1;
    currentIp4SrcAddrArray[0].arIP[3]=1;
    prefixLengthIPv4SrcArray[0]=26;

    rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, vrId,
                                    currentIp4GrpAddrArray[0],prefixLengthIPv4GrpArray[0],
                                    currentIp4SrcAddrArray[0],prefixLengthIPv4SrcArray[0]);
    if (rc == GT_OK)
    {
       /* prints used for debug - in case of fail add this code
        PRV_UTF_LOG5_MAC("Del V4, group %d.%d.%d.%d/%d,",
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         prefixLengthIPv4GrpArray[0]);
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         prefixLengthIPv4SrcArray[0]);
        */
    }
    else
    {
        PRV_UTF_LOG6_MAC("FAIL:0x%x,Del V4, group %d.%d.%d.%d/%d,",rc,
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         prefixLengthIPv4GrpArray[0]);
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         prefixLengthIPv4SrcArray[0]);
    }

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 1");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 2");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 3");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 4");
    }

    /* the following code is for debug purpose so if this test fails we can remove
       the code from remark and use the print to help us debug the problem */
#if 0
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 5");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 6");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 7");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 8");
    }
    rc = dumpRouteTcam(GT_TRUE);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 9");
    }
#endif

    /* add entry grp=225.1.1.0/23 , src=1.1.1.1/23

        ---------------------------------------------------------------------------------------------
                      column 0              column1          column2              column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32   |    1.1.1.1/32      |   225.1.1.0/30     |   1.1.1.0/30
        ---------------------------------------------------------------------------------------------
        line 1      1.1.0.0/23     |    225.1.1.0/28    |   1.1.1.0/28       |   1.1.1.0/24
                  new entry added  |                    |                    |
        ---------------------------------------------------------------------------------------------
        line 2      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default UC entry
        --------------------------------------------------------------------------------------------
        line 4    224.0.0.0/4(def) |    225.1.0.0/24    |  225.1.0.0/23      |   0/0(def)
                   moved from      |                    |  new entry added   |
                  line_4/column_2  |                    |                    |
        ---------------------------------------------------------------------------------------------

    */
    currentIp4GrpAddrArray[0].arIP[0]=225;
    currentIp4GrpAddrArray[0].arIP[1]=1;
    currentIp4GrpAddrArray[0].arIP[2]=1;
    currentIp4GrpAddrArray[0].arIP[3]=1;
    prefixLengthIPv4GrpArray[0]=23;

    currentIp4SrcAddrArray[0].arIP[0]=1;
    currentIp4SrcAddrArray[0].arIP[1]=1;
    currentIp4SrcAddrArray[0].arIP[2]=1;
    currentIp4SrcAddrArray[0].arIP[3]=1;
    prefixLengthIPv4SrcArray[0]=23;

    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, vrId,
                                    currentIp4GrpAddrArray[0],prefixLengthIPv4GrpArray[0],
                                    currentIp4SrcAddrArray[0],prefixLengthIPv4SrcArray[0],
                                    &mcRouteLttEntry,override,enable_fragmentation);
    if (rc == GT_OK)
    {
        /* prints used for debug - in case of fail add this code
        PRV_UTF_LOG5_MAC("Add V4, group %d.%d.%d.%d/%d,",
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         (prefixLengthIPv4GrpArray[0]-(i*2)));
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         (prefixLengthIPv4SrcArray[0]-(i*2)));
       */
    }
    else
    {
        PRV_UTF_LOG6_MAC("FAIL:0x%x,Add V4, group %d.%d.%d.%d/%d,",rc,
                         currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                         currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                         (prefixLengthIPv4GrpArray[0]-(i*2)));
        PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                         currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                         currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                         (prefixLengthIPv4SrcArray[0]-(i*2)));
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }

    /* the following code is for debug purpose so if this test fails we can remove
       the code from remark and use the print to help us debug the problem */
#if 0
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
    if (rc !=GT_OK)
    {
        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
    rc = dumpRouteTcam(GT_TRUE);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
    }
#endif

   /* delete entry grp=225.1.1.0/30 , src=1.1.1.1/30
      delete entry grp=225.1.1.0/28 , src=1.1.1.1/28

        ---------------------------------------------------------------------------------------------
                      column 0              column1          column2              column3
        ---------------------------------------------------------------------------------------------
        line 0      225.1.1.1/32   |    1.1.1.1/32      |  empty cell        |   empty cell
        ---------------------------------------------------------------------------------------------
        line 1      1.1.0.0/23     |    empty cell      |   empty cell       |   1.1.1.0/24
                  new entry added  |                    |                    |
        ---------------------------------------------------------------------------------------------
        line 2      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default UC entry
        --------------------------------------------------------------------------------------------
        line 4    224.0.0.0/4(def) |    225.1.0.0/24    |  225.1.0.0/23      |   0/0(def)
                   moved from      |                    |  new entry added   |
                  line_4/column_2  |                    |                    |
        ---------------------------------------------------------------------------------------------


   */
    for (i=0;i<2;i++)
    {
        currentIp4GrpAddrArray[0].arIP[0]=225;
        currentIp4GrpAddrArray[0].arIP[1]=1;
        currentIp4GrpAddrArray[0].arIP[2]=1;
        currentIp4GrpAddrArray[0].arIP[3]=1;
        prefixLengthIPv4GrpArray[0]=30;

        currentIp4SrcAddrArray[0].arIP[0]=1;
        currentIp4SrcAddrArray[0].arIP[1]=1;
        currentIp4SrcAddrArray[0].arIP[2]=1;
        currentIp4SrcAddrArray[0].arIP[3]=1;
        prefixLengthIPv4SrcArray[0]=30;


        rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, vrId,
                                        currentIp4GrpAddrArray[0],(prefixLengthIPv4GrpArray[0]-(i*2)),
                                        currentIp4SrcAddrArray[0],(prefixLengthIPv4SrcArray[0]-(i*2)));
        if (rc == GT_OK)
        {
            /* prints used for debug - in case of fail add this code
            PRV_UTF_LOG5_MAC("Del V4, group %d.%d.%d.%d/%d,",
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             prefixLengthIPv4GrpArray[0]-(i*2));
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             prefixLengthIPv4SrcArray[0]-(i*2));
            */
        }
        else
        {
            PRV_UTF_LOG6_MAC("FAIL:0x%x,Del V4, group %d.%d.%d.%d/%d,",rc,
                             currentIp4GrpAddrArray[0].arIP[0],currentIp4GrpAddrArray[0].arIP[1],
                             currentIp4GrpAddrArray[0].arIP[2],currentIp4GrpAddrArray[0].arIP[3],
                             prefixLengthIPv4GrpArray[0]-(i*2));
            PRV_UTF_LOG5_MAC("src %d.%d.%d.%d/%d,\n",
                             currentIp4SrcAddrArray[0].arIP[0],currentIp4SrcAddrArray[0].arIP[1],
                             currentIp4SrcAddrArray[0].arIP[2],currentIp4SrcAddrArray[0].arIP[3],
                             prefixLengthIPv4SrcArray[0]-(i*2));
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 1");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 2");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 3");
        }
        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 4");
        }

        /* the following code is for debug purpose so if this test fails we can remove
           the code from remark and use the print to help us debug the problem */
#if 0
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 5");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 6");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 7");
        }
        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
        if (rc !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 8");
        }
        rc = dumpRouteTcam(GT_TRUE);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam 1\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck 9");
        }
#endif
    }
    /*
       Add V6 3000::3000 will be added in line

        ---------------------------------------------------------------------------------------------
                      column 0              column1          column2              column3
        ---------------------------------------------------------------------------------------------
        line 0      new ipv6 UC entry added 3000: :3000
        ---------------------------------------------------------------------------------------------
        line 1      1.1.0.0/23     |    225.1.1.1/32     |   1.1.1.1/32      |   1.1.1.0/24
                                   |    moved from       |   moved from      |
                                   |    line_0/column_0  |  line_0/column_0  |
        ---------------------------------------------------------------------------------------------
        line 2      ipv6 default MC entry
        ---------------------------------------------------------------------------------------------
        line 3      ipv6 default UC entry
        --------------------------------------------------------------------------------------------
        line 4    224.0.0.0/4(def) |    225.1.0.0/24    |  225.1.0.0/23      |   0/0(def)
        ---------------------------------------------------------------------------------------------

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

    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+1;
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId,vrId,currentIp6AddrArray[0],prefixLengthIPv6Array[0],&nextHopInfo,override,enable_fragmentation);
    if (rc == GT_OK)
    {
         /* prints used for debug - in case of fail add this code
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
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
    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
        return;
    }

    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}

/**
* @internal prvTgfIpMcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isIpv4Traffic            -     GT_TRUE:  generate ipv4 traffic,
*                                      GT_FALSE: generate ipv6 traffic.
* @param[in] ipAddrPtr                -     (pointer to) the IP address in case we want to send
*                                      traffic different from the default
* @param[in] ipGroupPrefixLen         - prefix len of ipv4 MC group
* @param[in] ipSrcAddrPtr             -  (pointer to) the IP src address in case we want to send
*                                      traffic different from the default
* @param[in] ipSrcPrefixLen           - prefix len of ipv4 MC source
*                                       None
*/
GT_VOID prvTgfIpMcRoutingTrafficGenerate
(
   IN GT_BOOL               isIpv4Traffic,
   IN GT_IP_ADDR_TYPE_UNT   *ipAddrPtr,
   IN GT_U32                ipGroupPrefixLen,
   IN GT_IP_ADDR_TYPE_UNT   *ipSrcAddrPtr,
   IN GT_U32                ipSrcPrefixLen
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
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    TGF_PACKET_PART_STC             *prvTgfPacketPartArrayPtr;
    GT_U32                           outputCapturePort;
    GT_U8                           i=0;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                               tcamRowIndex;
    GT_U32                               tcamColumnIndex;

    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;


    ipSrcPrefixLen=ipSrcPrefixLen;

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
            cpssOsMemCpy(prvTgfPacketIpv4Part.srcAddr, ipSrcAddrPtr->ipv4Addr.arIP, sizeof(prvTgfPacketIpv4Part.srcAddr));

            /* copy bits 0..24  of dst Ip to mac DA - prevent ipv4 mac DA mismatch exception */
            prvTgfPacketL2Part_ipv4Mc.daMac[5] = prvTgfPacketIpv4Part.dstAddr[3];
            prvTgfPacketL2Part_ipv4Mc.daMac[4] = prvTgfPacketIpv4Part.dstAddr[2];
            prvTgfPacketL2Part_ipv4Mc.daMac[3] = prvTgfPacketIpv4Part.dstAddr[1];

            prvTgfPacketL2Part_ipv4Mc.daMac[3]&=0x7F;/* MAC_DA[23]=0 */
        }
        else
        {
            PRV_UTF_LOG0_MAC("Error - no IPv4 to send traffic to\n");
            return;
        }

        /* for group 225.1.1.1/24 Src 1.1.1.1/24 we want the egress port to be 23 and not 18
            routeEntryBaseIndex=12 --> mll 7 --> egress port 23 */
        if (ipGroupPrefixLen==24)
        {
            outputCapturePort = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
        }
        else
        {
            outputCapturePort = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS];
        }
    }
    else
    {
        if ((ipAddrPtr!=NULL)&&(ipSrcAddrPtr==NULL))/* ipv6 Uc entry */
        {
            /* fill destination IP address for packet */
            for (i = 0; i < 8; i++)
            {
                prvTgfPacketIpv6Part.dstAddr[i] = (ipAddrPtr->ipv6Addr.arIP[i * 2 +1]);
                prvTgfPacketIpv6Part.dstAddr[i] |= ((GT_U16)(ipAddrPtr->ipv6Addr.arIP[i * 2]))<<8;

                outputCapturePort = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
            }
        }
        else if ((ipAddrPtr!=NULL)&&(ipSrcAddrPtr!=NULL))/* ipv6 MC entry */
        {
            for (i = 0; i < 8; i++)
            {
                prvTgfPacketIpv6Part_ipv6Mc.dstAddr[i] = (ipAddrPtr->ipv6Addr.arIP[i * 2 +1]);
                prvTgfPacketIpv6Part_ipv6Mc.dstAddr[i] |= ((GT_U16)(ipAddrPtr->ipv6Addr.arIP[i * 2]))<<8;

                prvTgfPacketIpv6Part_ipv6Mc.srcAddr[i] = (ipSrcAddrPtr->ipv6Addr.arIP[i * 2 +1]);
                prvTgfPacketIpv6Part_ipv6Mc.srcAddr[i] |= ((GT_U16)(ipSrcAddrPtr->ipv6Addr.arIP[i * 2]))<<8;
            }

            /* update daMac of the packet sent */
            /* copy bits 0..31  of dst Ip to mac DA - prevent ipv6 MC mac DA mismatch exception */
            prvTgfPacketL2Part_ipv6Mc.daMac[5] = ipAddrPtr->ipv6Addr.arIP[15];
            prvTgfPacketL2Part_ipv6Mc.daMac[4] = ipAddrPtr->ipv6Addr.arIP[14];
            prvTgfPacketL2Part_ipv6Mc.daMac[3] = ipAddrPtr->ipv6Addr.arIP[13];
            prvTgfPacketL2Part_ipv6Mc.daMac[2] = ipAddrPtr->ipv6Addr.arIP[12];

            /* for group FF00::44FF/128 Src 1111::2222/128 we want the egress port to be 23.
               routeEntryBaseIndex=11 --> mll 7 --> egress port 23
               for group FF00::44FF/127 Src 1111::2222/127 we want the egress port to be 18.
               routeEntryBaseIndex=10 --> mll 5 --> egress port 18
            */
            if (ipGroupPrefixLen==128)
            {
                outputCapturePort = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS];
            }
            else
            {
                outputCapturePort = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS];
            }
        }
        else
        {
            PRV_UTF_LOG0_MAC("Error - no IPv6 to send traffic to\n");
            return;
        }
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

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = outputCapturePort;

    /* in case of prefix starting with 224.x.x.x, we should hit
       the default entry and the packet should be trapped to CPU */
    if (isIpv4Traffic == GT_TRUE && ipAddrPtr!=NULL && ipAddrPtr->ipv4Addr.arIP[0]==224)
    {
        /* start Rx capture */
        rc = tgfTrafficTableRxStartCapture(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");
    }
    else
    {
        /* enable capture on nexthope port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);
    }

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
        if (ipSrcAddrPtr!=NULL)/* MC entry */
        {
            partsCount = sizeof(prvTgfPacketPartArray_ipv6Mc) / sizeof(prvTgfPacketPartArray_ipv6Mc[0]);
            prvTgfPacketPartArrayPtr = prvTgfPacketPartArray_ipv6Mc;
        }
        else
        {
            partsCount = sizeof(prvTgfPacketIpv6PartArray) / sizeof(prvTgfPacketIpv6PartArray[0]);
            prvTgfPacketPartArrayPtr = prvTgfPacketIpv6PartArray;
        }

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

    /* AUTODOC: send IPv4 MC packet from port 0 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=5, (srcIP=0.0.0.0/0, dstIP=224.0.0.0/4) or
                         (srcIP=1.1.1.1/32, dstIP=225.1.1.1/32) or
                         (srcIP=1.1.1.0/24, dstIP=225.1.1.0/24) or
                         IPv6 UC packet from port 0 with (dstIP=3000::3000/128)
                         or for IPv6 Mc test:
                         (srcIP=0.0.0.0/0, dstIP=224.0.0.0/4)
                         IPv6 MC packet from port 0 with (dstIP=ff00::44ff/128)
                         IPv6 MC packet from port 0 with (dstIP=ff00::44ff/127)*/

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: verify routed packet */
    if (isIpv4Traffic == GT_TRUE && ipAddrPtr!=NULL && ipAddrPtr->ipv4Addr.arIP[0]==224)
    {
        /* stop Rx capture */
        rc = tgfTrafficTableRxStartCapture(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");

        /* get first entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorRxInCpuGet %d");

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_FALSE, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc,
                                      "tgfTrafficGeneratorRxInCpuGet %d");
    }
    else
    {
        if (isIpv4Traffic == GT_TRUE)
        {
            /* disable capture on a MC subscriber port */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    1, 1,
                    prvTgfPacketInfo.totalLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
                    0, 0,
                    prvTgfPacketInfo.totalLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

            /* for group 225.1.1.1/24 Src 1.1.1.1/24 we want the egress port to be 23 and not 18
                routeEntryBaseIndex=11 --> mll 7 --> egress port 23 */
            if (ipGroupPrefixLen==24)
            {
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS],
                        0, 0,
                        prvTgfPacketInfo.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS],
                        1, 1,
                        prvTgfPacketInfo.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
            else
            {
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS],
                        1, 1,
                        prvTgfPacketInfo.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS],
                        0, 0,
                        prvTgfPacketInfo.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
        }
        else
        {
            /* disable capture on nexthope port */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

             /* for group FF00::44FF/128 Src 1111::2222/128 we want the egress port to be 23.
                   routeEntryBaseIndex=11 --> mll 7 --> egress port 23
                   for group FF00::44FF/127 Src 1111::2222/127 we want the egress port to be 18.
                   routeEntryBaseIndex=10 --> mll 5 --> egress port 18
                */
            if (ipGroupPrefixLen==128)
            {
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                        1, 1,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
                        0, 0,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS],
                        0, 0,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS],
                        1, 1,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
            else
            {
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                        1, 1,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
                        0, 0,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_18_CNS],
                        1, 1,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_23_CNS],
                        0, 0,
                        prvTgfPacketInfo_ipv6.totalLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
        }

        /* for all packets that are non ipv4 default we expect the packet to be routed and
           not trapped to CPU, so trigger should be checked */

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

        if (isIpv4Traffic == GT_TRUE)
        {
            vfdArray[0].offset = 6;
            vfdArray[0].patternPtr[0] = 0;
            vfdArray[0].patternPtr[1] = 0;
            vfdArray[0].patternPtr[2] = 0;
            vfdArray[0].patternPtr[3] = 0;
            vfdArray[0].patternPtr[4] = 0;
            vfdArray[0].patternPtr[5] = PRV_TGF_NEXTHOPE_VLANID_CNS;
        }
        else
        {
            vfdArray[0].offset = 0;
            vfdArray[0].patternPtr[0] = 0;
            vfdArray[0].patternPtr[1] = 0;
            vfdArray[0].patternPtr[2] = 0;
            vfdArray[0].patternPtr[3] = 0;
            vfdArray[0].patternPtr[4] = 0x22;
            if (ipAddrPtr!=NULL)
            {
                if (ipSrcAddrPtr!=NULL)/* MC entry*/
                {
                     cpssOsMemCpy(vfdArray[0].patternPtr,prvTgfPacketL2Part_ipv6Mc.daMac, sizeof(TGF_MAC_ADDR));
                }
                else/* UC entry*/
                {
                    rc = prvTgfIpLpmIpv6UcPrefixSearch(prvTgfLpmDBId, vrId, ipAddrPtr->ipv6Addr, 128, &nextHopInfo, &tcamRowIndex, &tcamColumnIndex);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfIpLpmIpv6UcPrefixGet\n");
                    vfdArray[0].patternPtr[5] = (GT_U8)nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                }
            }
            else
            {
                vfdArray[0].patternPtr[5] = 0xb;
            }
        }

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray,
                                                            &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
            portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            vfdArray[0].patternPtr[0], vfdArray[0].patternPtr[1], vfdArray[0].patternPtr[2],
            vfdArray[0].patternPtr[3],vfdArray[0].patternPtr[4],vfdArray[0].patternPtr[5]);
    }

    /* get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n",
                         prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingTrafficGenerate
(
    GT_VOID
)
{
    GT_IP_ADDR_TYPE_UNT ipAddr;
    GT_IP_ADDR_TYPE_UNT ipSrcAddr;

    /* AUTODOC: GENERATE TRAFFIC: */
    ipAddr.ipv4Addr.arIP[0]=225;
    ipAddr.ipv4Addr.arIP[1]=1;
    ipAddr.ipv4Addr.arIP[2]=1;
    ipAddr.ipv4Addr.arIP[3]=1;

    ipSrcAddr.ipv4Addr.arIP[0]=1;
    ipSrcAddr.ipv4Addr.arIP[1]=1;
    ipSrcAddr.ipv4Addr.arIP[2]=1;
    ipSrcAddr.ipv4Addr.arIP[3]=1;

    prvTgfIpMcRoutingTrafficGenerate(GT_TRUE,&ipAddr,32,&ipSrcAddr,32);/* ipv4 traffic */

    ipAddr.ipv4Addr.arIP[0]=224;
    ipAddr.ipv4Addr.arIP[1]=1;
    ipAddr.ipv4Addr.arIP[2]=1;
    ipAddr.ipv4Addr.arIP[3]=1;

    ipSrcAddr.ipv4Addr.arIP[0]=2;
    ipSrcAddr.ipv4Addr.arIP[1]=2;
    ipSrcAddr.ipv4Addr.arIP[2]=2;
    ipSrcAddr.ipv4Addr.arIP[3]=2;

    prvTgfIpMcRoutingTrafficGenerate(GT_TRUE,&ipAddr,32,&ipSrcAddr,32);/* ipv4 traffic */

    ipAddr.ipv4Addr.arIP[0]=225;
    ipAddr.ipv4Addr.arIP[1]=1;
    ipAddr.ipv4Addr.arIP[2]=1;
    ipAddr.ipv4Addr.arIP[3]=0;

    ipSrcAddr.ipv4Addr.arIP[0]=1;
    ipSrcAddr.ipv4Addr.arIP[1]=1;
    ipSrcAddr.ipv4Addr.arIP[2]=1;
    ipSrcAddr.ipv4Addr.arIP[3]=0;

    prvTgfIpMcRoutingTrafficGenerate(GT_TRUE,&ipAddr,24,&ipSrcAddr,24);/* ipv4 traffic */

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
    ipAddr.ipv6Addr.arIP[15]=0;

    prvTgfIpMcRoutingTrafficGenerate(GT_FALSE,&ipAddr,128,NULL,0);/* ipv6 traffic */
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv6McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv6McRoutingTrafficGenerate
(
    GT_VOID
)
{
    GT_IP_ADDR_TYPE_UNT ipAddr;
    GT_IP_ADDR_TYPE_UNT ipSrcAddr;

    /* AUTODOC: GENERATE TRAFFIC: */
    ipAddr.ipv4Addr.arIP[0]=224;
    ipAddr.ipv4Addr.arIP[1]=1;
    ipAddr.ipv4Addr.arIP[2]=1;
    ipAddr.ipv4Addr.arIP[3]=1;

    ipSrcAddr.ipv4Addr.arIP[0]=2;
    ipSrcAddr.ipv4Addr.arIP[1]=2;
    ipSrcAddr.ipv4Addr.arIP[2]=2;
    ipSrcAddr.ipv4Addr.arIP[3]=2;

    prvTgfIpMcRoutingTrafficGenerate(GT_TRUE,&ipAddr,32,&ipSrcAddr,32);/* ipv4 traffic */

    ipAddr.ipv6Addr.arIP[0]=0xFF;
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
    ipAddr.ipv6Addr.arIP[14]=0x44;
    ipAddr.ipv6Addr.arIP[15]=0xff;

    ipSrcAddr.ipv6Addr.arIP[0]=0x11;
    ipSrcAddr.ipv6Addr.arIP[1]=0x11;
    ipSrcAddr.ipv6Addr.arIP[2]=0;
    ipSrcAddr.ipv6Addr.arIP[3]=0;
    ipSrcAddr.ipv6Addr.arIP[4]=0;
    ipSrcAddr.ipv6Addr.arIP[5]=0;
    ipSrcAddr.ipv6Addr.arIP[6]=0;
    ipSrcAddr.ipv6Addr.arIP[7]=0;
    ipSrcAddr.ipv6Addr.arIP[8]=0;
    ipSrcAddr.ipv6Addr.arIP[9]=0;
    ipSrcAddr.ipv6Addr.arIP[10]=0;
    ipSrcAddr.ipv6Addr.arIP[11]=0;
    ipSrcAddr.ipv6Addr.arIP[12]=0;
    ipSrcAddr.ipv6Addr.arIP[13]=0;
    ipSrcAddr.ipv6Addr.arIP[14]=0x22;
    ipSrcAddr.ipv6Addr.arIP[15]=0x22;

    prvTgfIpMcRoutingTrafficGenerate(GT_FALSE,&ipAddr,128,&ipSrcAddr,128);/* ipv6 traffic */

    ipAddr.ipv6Addr.arIP[0]=0xFF;
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
    ipAddr.ipv6Addr.arIP[14]=0x44;
    ipAddr.ipv6Addr.arIP[15]=0xfe;

    ipSrcAddr.ipv6Addr.arIP[0]=0x11;
    ipSrcAddr.ipv6Addr.arIP[1]=0x11;
    ipSrcAddr.ipv6Addr.arIP[2]=0;
    ipSrcAddr.ipv6Addr.arIP[3]=0;
    ipSrcAddr.ipv6Addr.arIP[4]=0;
    ipSrcAddr.ipv6Addr.arIP[5]=0;
    ipSrcAddr.ipv6Addr.arIP[6]=0;
    ipSrcAddr.ipv6Addr.arIP[7]=0;
    ipSrcAddr.ipv6Addr.arIP[8]=0;
    ipSrcAddr.ipv6Addr.arIP[9]=0;
    ipSrcAddr.ipv6Addr.arIP[10]=0;
    ipSrcAddr.ipv6Addr.arIP[11]=0;
    ipSrcAddr.ipv6Addr.arIP[12]=0;
    ipSrcAddr.ipv6Addr.arIP[13]=0;
    ipSrcAddr.ipv6Addr.arIP[14]=0x22;
    ipSrcAddr.ipv6Addr.arIP[15]=0x22;

    prvTgfIpMcRoutingTrafficGenerate(GT_FALSE,&ipAddr,127,&ipSrcAddr,127);/* ipv6 traffic */
}

/**
* @internal prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore function
* @endinternal
*
* @brief   Restore IP LPM DB Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore
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

/**
* @internal prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore function
* @endinternal
*
* @brief   Clean test scenario from costumer
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore
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

/**
* @internal prvTgfDynamicPartitionIpLpmDefragMcRoutingRandomCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A random IP v4/v6 MC prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=2047
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcRoutingRandomCostumerTestScenario
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
    GT_IPADDR     currentIp4SrcAddrAdd;
    GT_U32        baseIp4Addr=0x300808e0;
    GT_U32        baseIp4SrcAddr=0x30080801;
    GT_IPADDR     currentIp4AddrDel;
    GT_IPADDR     currentIp4SrcAddrDel;

    GT_IPV6ADDR   currentIp6AddrAdd;
    GT_IPV6ADDR   currentIp6SrcAddrAdd;
    GT_U8         baseIp6Addr[16]={0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0xFF,0xFF};
    GT_U8         baseIp6SrcAddr[16]={0x30,0,0,0,0,0,0,0,0,0,0,0,0,0,0x30,0};
    GT_IPV6ADDR   currentIp6AddrDel;
    GT_IPV6ADDR   currentIp6SrcAddrDel;

    GT_U32        prefixLengthIPv6 = 128;
    GT_U32        prefixLengthIPv6Src = 128;
    GT_U32        prefixLengthIPv4 = 32;
    GT_U32        prefixLengthIPv4Src = 32;

    IN  GT_U32    vrId = 0;
    GT_U32        enable_fragmentation = 1;
    GT_U32        loop_times;
    GT_BOOL       override = GT_FALSE;
    GT_U32        routeEntryBaseIndexValue=0;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC              mcRouteLttEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%12+11;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    mcRouteLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%12+11;;
    mcRouteLttEntry.numOfPaths          = 0;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    /* test senarion for
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=2047
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1*/

    for(loop_times=0;loop_times<PRV_TGF_LOOPS_NUM;loop_times++)
    {
         /* prints used for debug - in case of fail add this code
        cpssOsPrintf("loop_times:%d:\n",loop_times);
         */
        PRV_UTF_LOG0_MAC(".");
        /*random*/
        loop = (cpssOsRand() % 4);

        if(loop == 0)
        {
        /*add IPv4 prefix from (IPV4ADDR+0), (IPV4ADDR+1). to (IPV4ADDR+n1),
        In the next loop, add IPv4 prefix from (IPV4ADDR+n1+1)  to(IPV4ADDR+n1+n1)
        And the next loop
        The total IPv4 added less than 2k. If the number of IPv4 added up to 2k,go start add IPv4 from  (IPV4ADDR+0), */
        currentIp4AddrAdd.u32Ip = baseIp4Addr;
        currentIp4SrcAddrAdd.u32Ip = baseIp4SrcAddr;
        while(1)
        {
            /* add the current prefix*/
            rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, vrId,
                                currentIp4AddrAdd,prefixLengthIPv4,
                                currentIp4SrcAddrAdd,prefixLengthIPv4Src,
                                &mcRouteLttEntry,override,enable_fragmentation);
            if (rc == GT_OK)
            {
                /* uncomment for debugging
                PRV_UTF_LOG4_MAC("add Ipv4GrpAddr, 0x%x/%d: Ipv4SrcAddr, 0x%x/%d:\n",
                             currentIp4AddrAdd.u32Ip,prefixLengthIPv4,currentIp4SrcAddrAdd.u32Ip,prefixLengthIPv4Src);
                */

                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG4_MAC("currentIp4AddrAdd that fail validity,0x%x/%d: currentIp4SrcAddrAdd, 0x%x/%d:\n",
                             currentIp4AddrAdd.u32Ip,prefixLengthIPv4,currentIp4SrcAddrAdd.u32Ip,prefixLengthIPv4Src);
                    PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0
                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG5_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n after Ipv4GrpAddr, 0x%x/%d: Ipv4SrcAddr, 0x%x/%d:\n",
                             rc, currentIp4AddrAdd.u32Ip,prefixLengthIPv4,currentIp4SrcAddrAdd.u32Ip,prefixLengthIPv4Src);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0
                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
            }
            else
            {
                if ((rc!=GT_ALREADY_EXIST)&& (rc!=GT_FULL)&&(rc!=GT_BAD_PARAM))
                {
                    PRV_UTF_LOG5_MAC("FAIL:0x%x,add Ipv4GrpAddr, 0x%x/%d: Ipv4SrcAddr, 0x%x/%d:\n",
                             rc,currentIp4AddrAdd.u32Ip,prefixLengthIPv4,currentIp4SrcAddrAdd.u32Ip,prefixLengthIPv4Src);
                }
            }

            loop_add_v4 = (loop_add_v4 + 1) % max_ipv4;
            currentIp4AddrAdd.arIP[3]  = currentIp4AddrAdd.arIP[3]  +  (loop_add_v4 % 256);
            currentIp4AddrAdd.arIP[2]  = currentIp4AddrAdd.arIP[2]  +  ((loop_add_v4 / 256) % 256);

            currentIp4SrcAddrAdd.arIP[3]  = currentIp4SrcAddrAdd.arIP[3]  +  (loop_add_v4 % 256);
            currentIp4SrcAddrAdd.arIP[2]  = currentIp4SrcAddrAdd.arIP[2]  +  ((loop_add_v4 / 256) % 256);

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
        cpssOsMemCpy(&currentIp4SrcAddrDel.u32Ip, &baseIp4SrcAddr, sizeof(currentIp4SrcAddrDel));
        currentIp4AddrDel.arIP[3] += (20000 % 256);
        currentIp4AddrDel.arIP[2] += ((20000 / 256) % 256);
        currentIp4SrcAddrDel.arIP[3] += (20000 % 256);
        currentIp4SrcAddrDel.arIP[2] += ((20000 / 256) % 256);
        while(1)
        {
            /* delete the current prefix */
            rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, vrId,
                                        currentIp4AddrDel,prefixLengthIPv4,
                                        currentIp4SrcAddrDel,prefixLengthIPv4Src);
            if (rc == GT_OK)
            {
                /* prints used for debug - in case of fail add this code
                cpssOsPrintf("delete Ipv4Addr, 0x%x/%d: Ipv4SrcAddr, 0x%x:\n",
                             currentIp4AddrDel.u32Ip,prefixLengthIPv4,currentIp4SrcAddrDel.u32Ip,prefixLengthIPv4Src);
                */
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG4_MAC("currentIp4AddrDel that fail validity,0x%x/%d currentIp4SrcAddrDel=0x%x/%d \n",
                                 currentIp4AddrDel.u32Ip,prefixLengthIPv4,currentIp4SrcAddrDel.u32Ip,prefixLengthIPv4Src);
                    PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0
                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG5_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n, after ipv4 del grp:0x%x/%d, src:0x%x/%d\n",
                                 rc,currentIp4AddrDel.u32Ip,prefixLengthIPv4,currentIp4SrcAddrDel.u32Ip,prefixLengthIPv4Src);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0
                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
            }
            else
            {
                if (rc!=GT_NOT_FOUND)
                {
                    PRV_UTF_LOG5_MAC("FAIL:0x%x,delete Ipv4Addr:0x%x/%d src:0x%x/%d \n",
                                 rc,currentIp4AddrDel.u32Ip,prefixLengthIPv4,currentIp4SrcAddrDel.u32Ip,prefixLengthIPv4Src);
                }
            }

            loop_del_v4 = (loop_del_v4 + 1) % 20000;
            currentIp4AddrDel.arIP[3]  = currentIp4AddrDel.arIP[3]  -  (loop_del_v4 % 256);
            currentIp4AddrDel.arIP[2]  = currentIp4AddrDel.arIP[2]  -  ((loop_del_v4 / 256) % 256);

            currentIp4SrcAddrDel.arIP[3]  = currentIp4SrcAddrDel.arIP[3]  -  (loop_del_v4 % 256);
            currentIp4SrcAddrDel.arIP[2]  = currentIp4SrcAddrDel.arIP[2]  -  ((loop_del_v4 / 256) % 256);

            if((loop_del_v4 % 144) == 77)
            {
                break;
            }
        }
    }
    else if(loop == 2)
    {
        /*add IPv6 prefix from (IPV6ADDR+0), (IPV6ADDR+1) to (IPV6ADDR+n3),
        In the next loop, add IPv6 prefix from (IPV6ADDR+n3+1)  to(IPV6ADDR+n3+n3)
        And the next loop
        The total IPv6 added less than 1k. If the number of IPv6 added up to 1k,go start add IPv6 from  (IPV6ADDR+0), */

        cpssOsMemCpy(currentIp6AddrAdd.arIP, (void*)baseIp6Addr, sizeof(GT_IPV6ADDR));
        cpssOsMemCpy(currentIp6SrcAddrAdd.arIP, (void*)baseIp6SrcAddr, sizeof(GT_IPV6ADDR));
        while(1)
        {
            /* add the current prefix*/
            rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId,vrId,currentIp6AddrAdd,prefixLengthIPv6,
                                           currentIp6SrcAddrAdd,prefixLengthIPv6Src,
                                           &mcRouteLttEntry,override,enable_fragmentation);

            if (rc == GT_OK)
            {
                /* prints used for debug - in case of fail add this code
                  cpssOsPrintf("add Ipv6Addr, enable_fragmentation:%d, 0x%8x %8x %8x %8x/%d ,    Src=0x%8x %8x %8x %8x/%d :\n",
                             enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                             currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                             currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                             currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);
                */
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG10_MAC("currentIp6AddrAdd that fail validity,0x%8x %8x %8x %8x/%d ,    Src=0x%8x %8x %8x %8x/%d :\n",
                             currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                             currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                             currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                             currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);
                    PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0
                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG11_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail after ipv6 add,0x%8x %8x %8x %8x/%d ,    Src= 0x%8x %8x %8x %8x/%d :\n",
                             rc,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                             currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                             currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                             currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);

                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0
                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
            }
            else
            {
                if ((rc!=GT_ALREADY_EXIST)&& (rc!=GT_FULL)&&(rc!=GT_BAD_PARAM))
                    PRV_UTF_LOG12_MAC("FAIL:0x%x!!! add Ipv6Addr, enable_fragmentation:%d,0x%8x %8x %8x %8x/%d ,    Src= 0x%8x %8x %8x %8x/%d :\n",
                             rc, enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                             currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                             currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                             currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);
            }

            loop_add_v6 = (loop_add_v6 + 1) % max_ipv6;
            currentIp6AddrAdd.arIP[15]  = currentIp6AddrAdd.arIP[15]  +  (loop_add_v6 % 256);
            currentIp6AddrAdd.arIP[14]  = currentIp6AddrAdd.arIP[14]       +  ((loop_add_v6 / 256) % 256);

            currentIp6SrcAddrAdd.arIP[15]  = currentIp6SrcAddrAdd.arIP[15]  +  (loop_add_v6 % 256);
            currentIp6SrcAddrAdd.arIP[14]  = currentIp6SrcAddrAdd.arIP[14]       +  ((loop_add_v6 / 256) % 256);

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
        cpssOsMemCpy(currentIp6SrcAddrDel.arIP, (void*)baseIp6SrcAddr, sizeof(GT_IPV6ADDR));
        currentIp6AddrDel.arIP[15] += (10000 % 256);
        currentIp6AddrDel.arIP[14] += ((10000 / 256) % 256);
        currentIp6SrcAddrDel.arIP[15] += (10000 % 256);
        currentIp6SrcAddrDel.arIP[14] += ((10000 / 256) % 256);
        while(1)
        {
            /* delete the current prefix */
            rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId,vrId,currentIp6AddrDel, prefixLengthIPv6,currentIp6SrcAddrDel, prefixLengthIPv6Src);
            if (rc == GT_OK)
            {
                 /* prints used for debug - in case of fail add this code
                cpssOsPrintf("delete Ipv6Addr, 0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                */
                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG10_MAC("currentIp6AddrDel that fail validity,0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                    PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
               rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG11_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail after ipv6 del,0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                rc,currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                    /* the following code is for debug purpose so if this test fails we can remove
                       the code from remark and use the print to help us debug the problem */
#if 0
                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                    if (rc !=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                    }

                    rc = dumpRouteTcam(GT_TRUE);
                    if (rc!=GT_OK)
                    {
                        PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                    }
#endif
                    return;
                }
            }
            else
            {
                if (rc!=GT_NOT_FOUND)
                {
                    PRV_UTF_LOG11_MAC("failed:0x%x!!! delete Ipv6Addr, 0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                 rc,currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                 currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                 currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                 currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                }
            }

            loop_del_v6 = (loop_del_v6 + 1) % max_ipv6;
            currentIp6AddrDel.arIP[15]  = currentIp6AddrDel.arIP[15]  -  (loop_del_v6 % 256);
            currentIp6AddrDel.arIP[14]  = currentIp6AddrDel.arIP[14]  -  ((loop_del_v6 / 256) % 256);

            currentIp6SrcAddrDel.arIP[15]  = currentIp6SrcAddrDel.arIP[15]  -  (loop_del_v6 % 256);
            currentIp6SrcAddrDel.arIP[14]  = currentIp6SrcAddrDel.arIP[14]  -  ((loop_del_v6 / 256) % 256);

            if((loop_del_v6 % 188) == 144)
            {
                break;
            }
        }
     }
    }
    /* uncomment for debugging
    PRV_UTF_LOG1_MAC("\n loop_times:%d:\n",loop_times);
    */

    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4UcIpv6McRoutingRandomCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A random IP v4/v6 MC prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=1000
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4UcIpv6McRoutingRandomCostumerTestScenario
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
    GT_IPV6ADDR   currentIp6SrcAddrAdd;
    GT_U8         baseIp6Addr[16]={0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0xFF,0xFF};
    GT_U8         baseIp6SrcAddr[16]={0x30,0,0,0,0,0,0,0,0,0,0,0,0,0,0x30,0};
    GT_IPV6ADDR   currentIp6AddrDel;
    GT_IPV6ADDR   currentIp6SrcAddrDel;

    GT_U32        prefixLengthIPv6 = 128;
    GT_U32        prefixLengthIPv6Src = 128;
    GT_U32        prefixLengthIPv4 = 32;

    IN  GT_U32    vrId = 0;
    GT_U32        enable_fragmentation = 1;
    GT_U32        loop_times;
    GT_BOOL       override = GT_FALSE;
    GT_BOOL       defragmentationEnable=GT_FALSE;
    GT_U32        routeEntryBaseIndexValue=0;

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC              mcRouteLttEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%12+11;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    mcRouteLttEntry.routeEntryBaseIndex = routeEntryBaseIndexValue%12+11;;
    mcRouteLttEntry.numOfPaths          = 0;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    /* test senarion for
    lpmDbPartitionEnable=0x0
    lpmDbFirstTcamLine=0
    lpmDbLastTcamLine=1000
    maxNumOfIpv4Prefixes=1
    maxNumOfIpv4McEntries=1
    maxNumOfIpv6Prefixes=1*/

    currentIp4AddrAdd.u32Ip = baseIp4Addr;
    cpssOsMemCpy(currentIp6AddrAdd.arIP, (void*)baseIp6Addr, sizeof(GT_IPV6ADDR));
    cpssOsMemCpy(currentIp6SrcAddrAdd.arIP, (void*)baseIp6SrcAddr, sizeof(GT_IPV6ADDR));

    for(loop_times=0;loop_times<PRV_TGF_LOOPS_NUM;loop_times++)
    {
        /* prints used for debug - in case of fail add this code
        cpssOsPrintf("loop_times:%d:\n",loop_times);
        */
        PRV_UTF_LOG0_MAC(".");
        /*random*/
        loop = (cpssOsRand() % 4);

        if(loop == 0)
        {
            /*add IPv4 prefix from (IPV4ADDR+0), (IPV4ADDR+1). to (IPV4ADDR+n1),
            In the next loop, add IPv4 prefix from (IPV4ADDR+n1+1)  to(IPV4ADDR+n1+n1)
            And the next loop
            The total IPv4 added less than 2k. If the number of IPv4 added up to 2k,go start add IPv4 from  (IPV4ADDR+0), */

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
                    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId, currentIp4AddrAdd, prefixLengthIPv4, &nextHopInfo, override,defragmentationEnable);
                    if (rc == GT_OK)
                    {
                        numOfIpv4Added++;
                        /* prints used for debug - in case of fail add this code
                        cpssOsPrintf("add Ipv4Addr, 0x%x:\n",currentIp4AddrAdd.u32Ip);
                        */

                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG1_MAC("currentIp4AddrAdd that fail validity,0x%x\n",currentIp4AddrAdd.u32Ip);
                            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            /* the following code is for debug purpose so if this test fails we can remove
                               the code from remark and use the print to help us debug the problem */
#if 0
                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = dumpRouteTcam(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
#endif
                            return;
                        }
                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG2_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n after ipv4add 0x%x\n", rc,currentIp4AddrAdd.u32Ip);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            /* the following code is for debug purpose so if this test fails we can remove
                               the code from remark and use the print to help us debug the problem */
#if 0
                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = dumpRouteTcam(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
#endif
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

                loop_add_v4 = (loop_add_v4 + 1);
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
                        /* prints used for debug - in case of fail add this code
                        cpssOsPrintf("delete Ipv4Addr, 0x%x:\n",currentIp4AddrDel.u32Ip);
                        */
                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG1_MAC("currentIp4AddrDel that fail validity,0x%x\n",currentIp4AddrDel.u32Ip);
                            PRV_UTF_LOG1_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n", rc);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            /* the following code is for debug purpose so if this test fails we can remove
                               the code from remark and use the print to help us debug the problem */
#if 0
                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = dumpRouteTcam(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
#endif
                            return;
                        }
                        rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E);
                        if(rc != GT_OK)
                        {
                            PRV_UTF_LOG2_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n, after ipv4 del 0x%x\n", rc,currentIp4AddrDel.u32Ip);
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                            /* the following code is for debug purpose so if this test fails we can remove
                               the code from remark and use the print to help us debug the problem */
#if 0
                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,0);
                            if (rc !=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                            }

                            rc = dumpRouteTcam(GT_TRUE);
                            if (rc!=GT_OK)
                            {
                                PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                            }
#endif
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
                    numOfIpv4Added=0;
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
                cpssOsMemCpy(currentIp6SrcAddrAdd.arIP, (void*)baseIp6SrcAddr, sizeof(GT_IPV6ADDR));
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
                rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId,vrId,currentIp6AddrAdd,prefixLengthIPv6,
                                               currentIp6SrcAddrAdd,prefixLengthIPv6Src,
                                               &mcRouteLttEntry,override,enable_fragmentation);

                if (rc == GT_OK)
                {
                    numOfIpv6Added++;
                     /* prints used for debug - in case of fail add this code
                    cpssOsPrintf("add Ipv6Addr, enable_fragmentation:%d, 0x%8x %8x %8x %8x/%d ,    Src=0x%8x %8x %8x %8x/%d :\n",
                                 enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                                 currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                                 currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                                 currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);
                    */
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG10_MAC("currentIp6AddrAdd that fail validity,0x%8x %8x %8x %8x/%d ,    Src=0x%8x %8x %8x %8x/%d :\n",
                                 currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                                 currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                                 currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                                 currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);
                        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        /* the following code is for debug purpose so if this test fails we can remove
                           the code from remark and use the print to help us debug the problem */
#if 0
                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = dumpRouteTcam(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
#endif
                        return;
                    }
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG11_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail after ipv6 add,0x%8x %8x %8x %8x/%d ,    Src= 0x%8x %8x %8x %8x/%d :\n",
                                 rc,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                                 currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                                 currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                                 currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);

                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        /* the following code is for debug purpose so if this test fails we can remove
                           the code from remark and use the print to help us debug the problem */
#if 0
                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = dumpRouteTcam(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
#endif
                        return;
                    }
                }
                else
                {
                    if ((rc!=GT_ALREADY_EXIST)&& (rc!=GT_FULL)&&(rc!=GT_BAD_PARAM))
                        PRV_UTF_LOG12_MAC("FAIL:0x%x!!! add Ipv6Addr, enable_fragmentation:%d,0x%8x %8x %8x %8x/%d ,    Src= 0x%8x %8x %8x %8x/%d :\n",
                                 rc, enable_fragmentation,currentIp6AddrAdd.u32Ip[3],currentIp6AddrAdd.u32Ip[2],
                                 currentIp6AddrAdd.u32Ip[1],currentIp6AddrAdd.u32Ip[0],prefixLengthIPv6,
                                 currentIp6SrcAddrAdd.u32Ip[3],currentIp6SrcAddrAdd.u32Ip[2],
                                 currentIp6SrcAddrAdd.u32Ip[1],currentIp6SrcAddrAdd.u32Ip[0],prefixLengthIPv6Src);
                }

                if (numOfIpv6Added==max_ipv6)
                {
                    /* max number of ipv6 was added, need to delete them before we can nontinue adding */
                    break;
                }

                loop_add_v6 = (loop_add_v6 + 1);
                currentIp6AddrAdd.arIP[15]  = currentIp6AddrAdd.arIP[15]  +  (loop_add_v6 % 256);
                currentIp6AddrAdd.arIP[14]  = currentIp6AddrAdd.arIP[14]  +  ((loop_add_v6 / 256) % 256);

                currentIp6SrcAddrAdd.arIP[15]  = currentIp6SrcAddrAdd.arIP[15]  +  (loop_add_v6 % 256);
                currentIp6SrcAddrAdd.arIP[14]  = currentIp6SrcAddrAdd.arIP[14]  +  ((loop_add_v6 / 256) % 256);

                if((loop_add_v6 % 166) == 111)
                {
                     break;
                }
            }
        }
        else
        {
            /*  delete the ipv4 prefix list addded in loop 2 */
            cpssOsMemCpy(currentIp6AddrDel.arIP, currentIp6AddrAdd.arIP, sizeof(GT_IPV6ADDR));
            cpssOsMemCpy(currentIp6SrcAddrDel.arIP, currentIp6SrcAddrAdd.arIP, sizeof(GT_IPV6ADDR));
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
                rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId,vrId,currentIp6AddrDel, prefixLengthIPv6,currentIp6SrcAddrDel, prefixLengthIPv6Src);
                if (rc == GT_OK)
                {
                    numOfIpv6ToDel--;
                     /* prints used for debug - in case of fail add this code
                    cpssOsPrintf("delete Ipv6Addr, 0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                    currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                    currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                    currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                    currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                    */
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG10_MAC("currentIp6AddrDel that fail validity,0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                    currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                    currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                    currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                    currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                        PRV_UTF_LOG1_MAC("\r\n[V6] valid check fail!  rc = 0x%x.\r\n", rc);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        /* the following code is for debug purpose so if this test fails we can remove
                           the code from remark and use the print to help us debug the problem */
#if 0
                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = dumpRouteTcam(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
#endif
                        return;
                    }
                    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                    if(rc != GT_OK)
                    {
                        PRV_UTF_LOG11_MAC("\r\n[V4] valid check fail!  rc = 0x%x.\r\n,fail after ipv6 del,0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                    rc,currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                    currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                    currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                    currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");

                        /* the following code is for debug purpose so if this test fails we can remove
                           the code from remark and use the print to help us debug the problem */
#if 0
                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,0,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = cpssDxChIpPatTriePrint(prvTgfLpmDBId,vrId,1,1);
                        if (rc !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck");
                        }

                        rc = dumpRouteTcam(GT_TRUE);
                        if (rc!=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "dumpRouteTcam");
                        }
#endif
                        return;
                    }
                }
                else
                {
                    if (rc!=GT_NOT_FOUND)
                    {
                        PRV_UTF_LOG11_MAC("failed:0x%x!!! delete Ipv6Addr, 0x%8x %8x %8x %8x/%d Src:0x%8x %8x %8x %8x/%d\n",
                                     rc,currentIp6AddrDel.u32Ip[3],currentIp6AddrDel.u32Ip[2],
                                     currentIp6AddrDel.u32Ip[1],currentIp6AddrDel.u32Ip[0],prefixLengthIPv6,
                                     currentIp6SrcAddrDel.u32Ip[3],currentIp6SrcAddrDel.u32Ip[2],currentIp6SrcAddrDel.u32Ip[1],
                                     currentIp6SrcAddrDel.u32Ip[0],prefixLengthIPv6Src);
                    }
                }

                if((loop_del_v6==0)||(numOfIpv6ToDel==0))
                {
                    /* all ipv6 added in loop 0 were deleted */
                    numOfIpv6Added=0;
                    break;
                }
                else
                {
                    currentIp6AddrDel.arIP[15]  = currentIp6AddrDel.arIP[15]  -  (loop_del_v6 % 256);
                    currentIp6AddrDel.arIP[14]  = currentIp6AddrDel.arIP[14]  -  ((loop_del_v6 / 256) % 256);

                    currentIp6SrcAddrDel.arIP[15]  = currentIp6SrcAddrDel.arIP[15]  -  (loop_del_v6 % 256);
                    currentIp6SrcAddrDel.arIP[14]  = currentIp6SrcAddrDel.arIP[14]  -  ((loop_del_v6 / 256) % 256);

                    loop_del_v6--;
                }
            }
        }
    }
    /* uncomment for debugging
    PRV_UTF_LOG1_MAC("\n loop_times:%d:\n",loop_times);
    */
    PRV_UTF_LOG0_MAC("======= TEST PASSES OK =======\n");
}


