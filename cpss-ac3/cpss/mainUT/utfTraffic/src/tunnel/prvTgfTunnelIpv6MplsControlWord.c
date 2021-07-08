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
* @file prvTgfTunnelIpv6MplsControlWord.c
*
* @brief Tunnel Start: Ipv6 MPLS control word test
*
* @version   4
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
#include <common/tgfPacketGen.h>
#include <tunnel/tgfTunnelStartGenericIpv6Template.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_10_CNS           10

/* tunnel vid */
#define PRV_TGF_VLANID_200_CNS           200

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS 3

/* mac SA index in global mac sa table */
#define PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS          7

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* flag to indecate mac entry state */
/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32        prvTgfRouterArpTunnelStartLineIndex = 8;
static GT_U16       udpDstPort = 5247;
static GT_U16       udpSrcPort = 64212;

/*  IPv6 0ver IPv6 UDP Template packet to compare */
static TGF_PACKET_STC prvTgfTunnelMplsControlWordIpv6PacketInfo;

/* type of the tunnel */
static CPSS_TUNNEL_TYPE_ENT prvTgfTunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
    GT_U8                         udbOfsetArray[PRV_TGF_TTI_MAX_UDB_CNS];
    PRV_TGF_TTI_OFFSET_TYPE_ENT   udbOffsetTypeArray[PRV_TGF_TTI_MAX_UDB_CNS];
} prvTgfRestoreCfg;

/* the TTI Rule index */
static GT_U32 prvTgfTtiRuleIndex = 10;

static PRV_TGF_TTI_KEY_SIZE_ENT         keySizeOrig;
static GT_U32                           pclIdOrig;
static PRV_TGF_BRG_EGRESS_PORT_INFO_STC egressInfoOrig;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelMplsControlWordBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID prvTgfTunnelMplsControlWordBridgeConfigSet
(
     GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0, 0};
    TGF_PACKET_L2_STC *l2PartPtr;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* add tags */
    tagArray[0] = (GT_U8)GT_TRUE;
    tagArray[1] = (GT_U8)GT_TRUE;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 10 with tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_10_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

   /* check is mac entry created */

    /* use L2 part */
    l2PartPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[6].partPtr;
    /* AUTODOC: add FDB entry with MAC 01:02:03:34:02 , VLAN 10, port 58 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(l2PartPtr->daMac,
                                          PRV_TGF_VLANID_10_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelMplsControlWordTTIConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E lookup
*         - Set TTI rule action
*         - Set TTI rule
*         - Set Udb
*/
static GT_VOID prvTgfTunnelMplsControlWordTTIConfigSet
(
    GT_VOID
)
{

    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ruleAction = {0};
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_U8                    pclId = 0x1;
    GT_U32                   ii;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);


    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Mpls TCAM location */

    /* init TTI Pattern */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    /* init TTI Mask */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_TRUE;
    ruleAction.passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;

    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.bridgeBypass    = GT_FALSE;

    ruleAction.redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
    ruleAction.keepPreviousQoS = GT_TRUE;
    ruleAction.tunnelStart    = GT_FALSE;
    ruleAction.tunnelStartPtr = 0;

    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;

     /* set TTI Mask */
     /* PCL ID */
    ttiPattern.udbArray[0] = pclId;
    ttiMask.udbArray[0] = 0x1f;

    /* Local Device Source is Trunk */
    ttiPattern.udbArray[1] = 0;
   ttiMask.udbArray[1] = 0x20;

    /* Local Device Source ePort/TrunkID */
    ttiPattern.udbArray[2] = (GT_U8)prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiMask.udbArray[2] = 0xff;

    /* eVlan */
    ttiPattern.udbArray[3] = PRV_TGF_VLANID_200_CNS;
    ttiMask.udbArray[3] = 0xff;

    /* udp destination port */
    ttiPattern.udbArray[4] = (GT_U8)((udpDstPort >> 8) & 0xff);
    ttiMask.udbArray[4] = 0xff;
    ttiPattern.udbArray[5] = (GT_U8)(udpDstPort & 0xff);
    ttiMask.udbArray[5] = 0xff;

    /* udp src port */
    ttiPattern.udbArray[6] = (GT_U8)((udpSrcPort >> 8) & 0xff);;
    ttiMask.udbArray[6] = 0xff;
    ttiPattern.udbArray[7] = (GT_U8)(udpSrcPort & 0xff);
    ttiMask.udbArray[7] = 0xff;

    /* capwap preamble type: non-DTLS capwap packet */
    ttiPattern.udbArray[8] = 0;
    ttiMask.udbArray[8] = 0xff;

    /* capwap type: payload is 802.3 frame */
    ttiPattern.udbArray[9] = 0;
    ttiMask.udbArray[9] = 0x80;

    /* capwap fragment: payload is not capwap fragmented F-bit offset 0*/
    ttiPattern.udbArray[10] = 0;
    /* capwap keep-alive bit: it is not keep-alive packet*/
    ttiPattern.udbArray[10] = ttiPattern.udbArray[10] & 0xef; /* set bit4 (K-bit) to 0*/
    /* capwap radio mac bit: it is not radio mac address extension */
    ttiPattern.udbArray[10] = ttiPattern.udbArray[10] & 0xf7; /* set bit3 (M-bit) to 0*/
    ttiMask.udbArray[10] = 0x19;

    /*Reserved*/
    for (ii=11; ii<19; ii++)
    {
        ttiPattern.udbArray[ii] = 0;
        ttiMask.udbArray[ii] = 0;
    }


    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,&keySizeOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E , PRV_TGF_TTI_KEY_SIZE_20_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex,PRV_TGF_TTI_RULE_UDB_20_E, &ttiPattern, &ttiMask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /*save udb for restore*/
    for(ii=0; ii < 20; ii++)
    {
        rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,ii,&prvTgfRestoreCfg.udbOffsetTypeArray[ii],&prvTgfRestoreCfg.udbOfsetArray[ii]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);
    }

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,0,PRV_TGF_TTI_OFFSET_METADATA_E,22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,1,PRV_TGF_TTI_OFFSET_METADATA_E,22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,2,PRV_TGF_TTI_OFFSET_METADATA_E,26);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,3,PRV_TGF_TTI_OFFSET_METADATA_E,24);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,4,PRV_TGF_TTI_OFFSET_L4_E,2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,5,PRV_TGF_TTI_OFFSET_L4_E,3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,6,PRV_TGF_TTI_OFFSET_L4_E,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,7,PRV_TGF_TTI_OFFSET_L4_E,1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,8,PRV_TGF_TTI_OFFSET_L4_E,8);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,9,PRV_TGF_TTI_OFFSET_L4_E,10);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,10,PRV_TGF_TTI_OFFSET_L4_E,11);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,&pclIdOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelIpv6MplsControlWordTunnelStartConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
static GT_VOID prvTgfTunnelIpv6MplsControlWordTunnelStartConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileData;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileDataGet;
    TGF_PACKET_L2_STC     *l2PartPtr;
    TGF_PACKET_IPV6_STC     *prvTgfPacketIpv6PartPtr;
    GT_ETHERADDR          macSa;
    GT_ETHERADDR          macSaGet;
    GT_U32                globalMacSaIndexGet;
    GT_U32 ii;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;


    GT_U32 i,j;
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    cpssOsMemSet(&tunnelEntry.ipv6Cfg,0,sizeof(tunnelEntry.ipv6Cfg));
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    cpssOsMemSet(&profileDataGet,0,sizeof(profileDataGet));
    cpssOsMemSet(macSa.arEther,0,sizeof(GT_ETHERADDR));
    cpssOsMemSet(macSaGet.arEther,0,sizeof(GT_ETHERADDR));
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_L2_E, &prvTgfTunnelMplsControlWordIpv6PacketInfo, (GT_VOID *)&l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV6_E, &prvTgfTunnelMplsControlWordIpv6PacketInfo, (GT_VOID *)&prvTgfPacketIpv6PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E; /* eliminate Coverity warning */
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_VLANID_200_CNS;
    tunnelEntry.ipv6Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv6Cfg.up               = 0;
    tunnelEntry.ipv6Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv6Cfg.dscp             = 0;
    tunnelEntry.ipv6Cfg.ttl              = 33;
    tunnelEntry.ipv6Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv6Cfg.udpDstPort       = udpDstPort;
    tunnelEntry.ipv6Cfg.udpSrcPort       = udpSrcPort;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, l2PartPtr->saMac, sizeof(TGF_MAC_ADDR));

    for (ii = 0; ii < 8; ii++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6PartPtr->srcAddr[ii] >> 8);
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6PartPtr->srcAddr[ii];
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6PartPtr->dstAddr[ii] >> 8);
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6PartPtr->dstAddr[ii];
    }

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV6 */
    /* AUTODOC:   vlanId=6, ttl=33 */
    /* AUTODOC:   DA=88:77:11:11:55:66, srcIp=2.2.2.2, dstIp=1.1.1.3 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex, prvTgfTunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /*** Get a tunnel start entry ***/
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex, &tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /*  AUTODOC: Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VLANID_200_CNS, tunnelEntry.ipv6Cfg.vlanId,
                                 "tunnelEntry.ipv6Cfg.vlanId: %d", tunnelEntry.ipv6Cfg.vlanId);

    /*  AUTODOC: Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv6Cfg.macDa.arEther, l2PartPtr->saMac, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv6Cfg.macDa.arEther[5]: 0x%2X",
                                 tunnelEntry.ipv6Cfg.macDa.arEther[5]);

    /*  AUTODOC: configure tunnel start profile */
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E; /* 8 byte */
    profileData.udpSrcPortMode = PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E;
    profileData.hashShiftLeftBitsNumber = 0;
    profileData.controlWordIndex = 1;
    /* At the begining all bits are zero. In this cycle only "ones" are filled  */
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if ( i == 1)
            {
                if (j == 3)
                {
                    profileData.templateDataBitsCfg[i*8+j] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
                }
            }
            if (i == 2)
            {
                if (j == 6)
                {
                    profileData.templateDataBitsCfg[i*8+j] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
                }
            }
        }
    }
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum, PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);
    rc = prvTgfTunnelStartGenProfileTableEntryGet(prvTgfDevNum, PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileDataGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);
    rc = cpssOsMemCmp(&profileData, &profileDataGet, sizeof(PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong profile data get");


    /*  AUTODOC: configure tunnel start mac SA*/

    cpssOsMemCpy((GT_VOID*)macSa.arEther,
                 (GT_VOID*)l2PartPtr->daMac,
                 sizeof(GT_ETHERADDR));

    /* AUTODOC: configure MAC SA to global MAC SA table into index 7*/
    rc =  prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS,&macSa);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");
    rc = prvTgfIpRouterGlobalMacSaGet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaGet: %d", prvTgfDevNum);
    rc = cpssOsMemCmp(&macSa, &macSaGet, sizeof(GT_ETHERADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong mac sa data get");

    /* AUTODOC: configure index to global MAC SA table in eport attribute table */
    rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");
    rc = prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &globalMacSaIndexGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet: %d", prvTgfDevNum);
    rc = (globalMacSaIndexGet == PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS) ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong mac sa index get");

    /* set the egress eport to use TS for the ethernet passenger */
    egressInfo.tunnelStart = GT_TRUE;
    egressInfo.tunnelStartPtr = prvTgfRouterArpTunnelStartLineIndex;
    egressInfo.tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* configure destination e-port properties */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],&egressInfoOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],&egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfTunnelMplsControlWordBuildPacket function
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
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV6_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
static GT_VOID prvTgfTunnelMplsControlWordBuildPacket
(
    GT_VOID
)
{
    GT_U32                  ip_header_protocol = 2;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_UDP_STC *prvTgfPacketUdpPartPtr;
    TGF_PACKET_IPV6_STC     *prvTgfPacketIpv6PartPtr;
    TGF_PACKET_VLAN_TAG_STC *prvTgfPacketVlanTagPtr;

    /* AUTODOC: get default Generic IPv6 UDP Template packet */
    rc = prvTgfPacketGenericIpv6Template8DefaultPacketGet(ip_header_protocol,&prvTgfTunnelMplsControlWordIpv6PacketInfo.numOfParts, &prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketGenericIpv6UdpTemplateDefaultPacketGet");
    prvTgfPacketUdpPartPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[4].partPtr;
    prvTgfPacketUdpPartPtr->dstPort = 5247;
    prvTgfPacketUdpPartPtr->length = 0x60;
    prvTgfPacketIpv6PartPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[3].partPtr;
    prvTgfPacketIpv6PartPtr->payloadLen = 0x60;

    prvTgfPacketVlanTagPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[1].partPtr;
    prvTgfPacketVlanTagPtr->vid = PRV_TGF_VLANID_200_CNS;
    prvTgfPacketVlanTagPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[7].partPtr;
    prvTgfPacketVlanTagPtr->vid = PRV_TGF_VLANID_10_CNS;
    prvTgfTunnelMplsControlWordIpv6PacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
}

/**
* @internal prvTgfTunnelMplsControlWordCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*/
static GT_VOID prvTgfTunnelMplsControlWordCheckCaptureEgressTrafficOnPort
(
 GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC expectedPacketInfo;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;

    TGF_PACKET_L2_STC *l2PartPtr;
    TGF_PACKET_L2_STC l2Part;
    TGF_PACKET_IPV6_STC     *prvTgfPacketIpv6PartPtr;
    TGF_PACKET_IPV6_STC     prvTgfPacketIpv6Part;
    TGF_PACKET_TEMPLATE8_STC *prvTgfPacketTemplate8PartPtr;
    TGF_PACKET_TEMPLATE8_STC  prvTgfPacketControlWordPart;
    TGF_PACKET_UDP_STC      *prvTgfPacketUdpPartPtr;

    GT_U8 prvTgfTemplate8DataArr[8] = {0x00, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
    GT_U8 prvControlWordDataArr[4] = {0x00, 0x00, 0x00, 0x00};

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);
    /* build expected packet */
    l2PartPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[0].partPtr;
    cpssOsMemCpy(&l2Part, l2PartPtr, sizeof(TGF_PACKET_L2_STC));
    cpssOsMemCpy(l2PartPtr->daMac,l2Part.saMac,sizeof(l2Part.saMac));
    cpssOsMemCpy(l2PartPtr->saMac,l2Part.daMac,sizeof(l2Part.daMac));

    prvTgfPacketIpv6PartPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[3].partPtr;
    cpssOsMemCpy(&prvTgfPacketIpv6Part, prvTgfPacketIpv6PartPtr, sizeof(TGF_PACKET_IPV6_STC));
    cpssOsMemCpy(prvTgfPacketIpv6PartPtr->dstAddr,prvTgfPacketIpv6Part.srcAddr,sizeof(prvTgfPacketIpv6Part.srcAddr));
    cpssOsMemCpy(prvTgfPacketIpv6PartPtr->srcAddr,prvTgfPacketIpv6Part.dstAddr,sizeof(prvTgfPacketIpv6Part.dstAddr));
    prvTgfPacketIpv6PartPtr->payloadLen += 4; /* add MPLS control word length */
    prvTgfPacketTemplate8PartPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[5].partPtr;
    cpssOsMemCpy(prvTgfPacketTemplate8PartPtr->dataPtr, prvTgfTemplate8DataArr, sizeof(prvTgfTemplate8DataArr));

    /* Set UDP length */
    prvTgfPacketUdpPartPtr = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[4].partPtr;
    prvTgfPacketUdpPartPtr->length += 4;

    /* copy the expected packet from the ingress packet */
    expectedPacketInfo.numOfParts = prvTgfTunnelMplsControlWordIpv6PacketInfo.numOfParts + 1;
    expectedPacketInfo.totalLen = prvTgfTunnelMplsControlWordIpv6PacketInfo.totalLen;

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
    for(ii = 0 ; ii < 6 ; ii++)
    {
        expectedPacketInfo.partsArray[ii] = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[ii];
    }

     /* set control word */
    prvTgfPacketControlWordPart.dataLength = 4;
    prvTgfPacketControlWordPart.dataPtr = cpssOsMalloc(sizeof(prvControlWordDataArr));
    cpssOsMemCpy(prvTgfPacketControlWordPart.dataPtr, prvControlWordDataArr, sizeof(prvControlWordDataArr));
    expectedPacketInfo.partsArray[6].type = TGF_PACKET_PART_TEMPLATE_E;
    expectedPacketInfo.partsArray[6].partPtr = &prvTgfPacketControlWordPart;

    /* copy expected parts from the original sent parts */
    for(ii = 7 ; ii < expectedPacketInfo.numOfParts ; ii++)
    {
        expectedPacketInfo.partsArray[ii] = prvTgfTunnelMplsControlWordIpv6PacketInfo.partsArray[ii - 1];
    }

    /* print and check captured packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &expectedPacketInfo,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
             0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* free the dynamic allocated memory */
    cpssOsFree(prvTgfPacketControlWordPart.dataPtr);
    cpssOsFree(expectedPacketInfo.partsArray);
}

/**
* @internal prvTgfTunnelMplsControlWordTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
*/
static GT_VOID prvTgfTunnelMplsControlWordTrafficGenerate
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

    /* AUTODOC: send 3  IPv6  packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */

    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfTunnelMplsControlWordIpv6PacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    /* check captured egress traffic on port */
    prvTgfTunnelMplsControlWordCheckCaptureEgressTrafficOnPort();

    return;
}

/**
* @internal prvTgfTunnelMplsControlWordConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfTunnelMplsControlWordConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore TTI Configuration
     */

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for IPv6 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore key size for PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E , keySizeOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore pclId */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,pclIdOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* AUTODOC: restore egress ePort info */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],&egressInfoOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


    /* AUTODOC: invalidate VLAN entry 10 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_10_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* restore 'hard drop' */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E, CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    prvTgfPacketRestoreDefaultParameters();

    PRV_UTF_LOG0_MAC("======= reset mac counters on all tested ports. =======\n");
    prvTgfEthCountersReset(prvTgfDevNum);

}

/**
* @internal prvTgfTunnelIpv6MplsControlWordTest function
* @endinternal
*
* @brief   Capwap test main function
*/
GT_VOID prvTgfTunnelIpv6MplsControlWordTest
(
    GT_VOID
)
{
    prvTgfTunnelMplsControlWordBuildPacket();

    prvTgfTunnelMplsControlWordBridgeConfigSet();

    prvTgfTunnelMplsControlWordTTIConfigSet();

    prvTgfTunnelIpv6MplsControlWordTunnelStartConfigurationSet();

    prvTgfTunnelMplsControlWordTrafficGenerate();

    prvTgfTunnelMplsControlWordConfigurationRestore();
}

