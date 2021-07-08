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
* @file prvTgfTunnelTermTrillUnknownUcTransitAndTermination.c
*
* @brief Tunnel: Unknown Unicast: Transit TRILL packet & Send Native to Local Host
*
* @version   17
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
#include <common/tgfPortGen.h>
#include <common/tgfTunnelGen.h>
#include <l2mll/prvTgfL2MllUtils.h>
#include <tunnel/prvTgfTunnelTermTrillUnknownUcTransitAndTermination.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* TRILL Hop Count (in bytes)*/
#define PRV_TGF_TRILL_HOP_COUNT_SIZE_CNS    2

/* Ethernet VLAN Id */
#define PRV_TGF_ETHERNET_VLANID_CNS         5

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            6

/* eVidx to a pointer of linked-list of L2MLL TRILL To-ME (instance #1) for Flooding in ETHERNET VLAN */
#define PRV_TGF_FLOOD_EVIDX_CNS          4096

/* eVidx to a pointer of linked-list of L2MLL TRILL TRANSIT (instance #0) for transit on tree branches in TRILL VLAN*/
#define PRV_TGF_EVIDX_CNS                4097

/* Destination ePort TRILL To-ME (instance #1) - known UC, not used in this test */
#define PRV_TGF_DESTINATION_EPORT_CNS    1000

/* Destination ePort TRILL TRANSIT (instance #0) */
#define PRV_TGF_DESTINATION_EPORT1_CNS   1001

/* Source ePort for TRILL To-ME (instance #1) */
#define PRV_TGF_SOURCE_EPORT_CNS          700

/* Source ePort TRILL TRANSIT (instance #0) */
#define PRV_TGF_SOURCE_EPORT1_CNS         701

/* Tti entry index */
#define PRV_TGF_TTI_INDEX_CNS              80

/* Tti entry index */
#define PRV_TGF_TTI_INDEX1_CNS              1

/* Tti entry index */
#define PRV_TGF_TTI_INDEX2_CNS             20

/* Tti last entry index */
#define PRV_TGF_TTI_LAST_INDEX_CNS        100

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS         1

/* Adjacency entry index*/
#define PRV_TGF_ADJACENCY_INDEX_CNS         1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           0

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS        3

/* max number of VFD to use */
#define PRV_TGF_VFD_NUM_CNS                10

/* number of ports in PRV_TGF_ETHERNET_VLANID_CNS */
static GT_U8  prvTgfPortsInVlanNum = 2;

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* offset of the passenger in the packet*/
#define PASSENGER_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
                                 TGF_ETHERTYPE_SIZE_CNS + TGF_TRILL_HEADER_SIZE_CNS)

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

static TGF_MAC_ADDR prvTgfPacketAllEsadiMacDa = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x42};     /* daMac - "ALL-ESADI-RBridges Multicast address" */
/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x40},                /* daMac - "All-RBridges MC address" */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

/* VLAN_TAG part of packet */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_TRILL_VLANID_CNS                      /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_22F3_TRILL_TAG_CNS};

/* packet's TRILL */
static TGF_PACKET_TRILL_STC prvTgfPacketTrillPart =
{
    0,                  /* version*/
    GT_TRUE,            /* mBit */
    0,                  /* opLength */
    0x20,               /* hopCount */
    0x6543,             /* eRbid */
    0x4321              /* iRbid*/
};

/***************** Incapsulation ethernet frame **********************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartInt = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPartInt =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_ETHERNET_VLANID_CNS       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartInt = {0x600};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_TRILL_E,      &prvTgfPacketTrillPart},
    /* incapsulation ethernet frame part */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartInt},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPartInt},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartInt},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of transmitted packet */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2   + \
    TGF_VLAN_TAG_SIZE_CNS  * 2   + \
    TGF_ETHERTYPE_SIZE_CNS * 2   + \
    TGF_TRILL_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_TRANSMITTED_PACKET_LEN_CNS,      /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/* LENGTH of expected packet (original packet) */
#define PRV_TGF_NATIVE_EXPECTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS  + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* MAC for Tunnel Start Entry - "All-RBridges MC address" */
static TGF_MAC_ADDR prvTgfTunnelStartEntryMacDa = {
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x40
};
/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    GT_U32                                      trillEtherType;
    GT_BOOL                                     trillEnableTransmitPort;
    GT_BOOL                                     trillEnableDestinationPort;
    GT_U16                                      trillOuterVid0;
    GT_BOOL                                     SrcPortPe;
    GT_BOOL                                     SrcPortPe1;
    GT_BOOL                                     DstPortPe;
    GT_BOOL                                     DstPortPe1;
    GT_BOOL                                     SrcPortIngFlt;
    GT_BOOL                                     vlanEgressFilteringState;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   vlanEgressPortTagStateMode;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT           vlanEgressPortTagState;

} prvTgfRestoreCfg;

/******************************************************************************/
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfL2MllVlanInit function
* @endinternal
*
* @brief   Initialize local vlan default settings
*
* @param[in] vlanId                   - vlan id.
* @param[in] floodVidx                - eVidx index
*                                       None
*/
static GT_VOID prvTgfL2MllVlanInit
(
    IN GT_U16           vlanId,
    IN GT_U16           floodVidx
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PORTS_BMP_STC          portsMembers;
    GT_U32                      portIter;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* ports 0, 8, are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsInVlanNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* set vlan entry */
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
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = floodVidx;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT                        pattern;
    PRV_TGF_TTI_RULE_UNT                        mask;
    PRV_TGF_TTI_ACTION_2_STC                    ruleAction = {0};
    PRV_TGF_TTI_TRILL_ADJACENCY_STC             adjacencyPattern;
#if 0
    PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC      iRbidEntry;
    PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC      eRbidEntry;
#endif
    /*PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  iRbidLttEntry;
    PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  eRbidLttEntry;*/
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;
    PRV_TGF_TUNNEL_START_ENTRY_UNT              tunnelEntry;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                lttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT            mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC                     mllPairEntry;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   stateMode;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: define "Flood" eVIDX for native VLAN with value > 4K (indicating L2MLL duplication list) */
    prvTgfL2MllVlanInit(PRV_TGF_ETHERNET_VLANID_CNS, PRV_TGF_FLOOD_EVIDX_CNS);

     /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: enable eVidx for all lookup */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d", GT_TRUE);

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    /* MLL entry :Trg Vidx = 0xFFF (All native Ethernet VLAN port members */
    mllPairEntry.nextPointer = 1;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC:   vlanId=5 */
    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VID_E; /* vidx = 0xFFF: All native Ethernet VLAN port members */
    mllPairEntry.firstMllNode.egressInterface.vlanId = PRV_TGF_ETHERNET_VLANID_CNS;
    mllPairEntry.firstMllNode.meshId = 0; /* traffic is not replicated by this MLL entry */

    /* MLL entry : Trg ePort */
    mllPairEntry.secondMllNode.last = GT_FALSE;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT1_CNS;
    mllPairEntry.secondMllNode.meshId = 0; /* traffic is replicated by this MLL entry */

    /* AUTODOC: set L2 MLL entry 0: */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* MLL entry : Src ePort */

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = PRV_TGF_SOURCE_EPORT1_CNS;
    mllPairEntry.firstMllNode.meshId = 0; /* traffic is replicated by this MLL entry */

    /* AUTODOC: set L2 MLL entry 1: */
    /* AUTODOC:  ePort=701 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 1");

    /* AUTODOC: create TRILL VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* set trill ethertype */
    rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E, &(prvTgfRestoreCfg.trillEtherType));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeGet);

    /* AUTODOC: set TRILL ethertype 0x22F3 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E, TGF_ETHERTYPE_22F3_TRILL_TAG_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeSet);

    /* set trill engine enable on transmit port */
    rc = prvTgfTtiPortTrillEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.trillEnableTransmitPort));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableGet);

    /* AUTODOC: enable TRILL engine on port 0 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* set trill engine enable on destination ePort port */
    rc = prvTgfTtiPortTrillEnableGet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT1_CNS, &(prvTgfRestoreCfg.trillEnableDestinationPort));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableGet);

    /* AUTODOC: enable TRILL engine on ePort 1001 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT1_CNS, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_TRILL */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_TRILL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_TRILL_VLANID_CNS;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table 1: */
    /* AUTODOC:   DA=01:80:C2:00:00:40, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.saMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));
    adjacencyPattern.trillMBit      = 1;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    adjacencyPattern.trillEgressRbid = 0x6543;

    /* AUTODOC: config entry in TRILL Adjacency Check dedicated TCAM: */
    /* AUTODOC:   SA=00:00:00:00:00:55, port=0, trillEgressRbid=0x6543 */
    rc = prvTgfTtiTrillAdjacencyCheckEntrySet(prvTgfDevNum, PRV_TGF_ADJACENCY_INDEX_CNS, &adjacencyPattern);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillAdjacencyCheckEntrySet);

    /************* Configuration for transit packet: ************/
    PRV_UTF_LOG0_MAC("\nConfiguration for TRILL TRANSIT packet:\n");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern,    0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,       0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* configure TTI rule - Packet Instance #0; Transit Processing */
    pattern.trill.trillMBit = GT_TRUE;
    pattern.trill.trillMcDescriptorInstance = 0; /* this is the multi-destination transit packet instance */
    pattern.trill.trillEgressRbid  = 0x6543; /* branch on distribution tree */
    pattern.trill.trillIngressRbid = 0x4321;

    /* configure TTI rule mask - Packet Instance #0; Transit Processing */
    mask.trill.trillMBit = GT_TRUE;
    mask.trill.trillMcDescriptorInstance = 1;
    mask.trill.trillEgressRbid  = 0xFFFF;
    mask.trill.trillIngressRbid = 0xFFFF;

    /* configure TTI rule action */
    ruleAction.command                 = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand         = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.egressInterface.type    = CPSS_INTERFACE_VIDX_E;
    ruleAction.egressInterface.vidx    = PRV_TGF_EVIDX_CNS;
    ruleAction.ingressPipeBypass       = GT_TRUE;
    ruleAction.enableDecrementTtl      = GT_TRUE;
    ruleAction.tunnelTerminate         = GT_FALSE;
    /*changed from PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E; because when 'no modify' the re-parse not take vid0 from the passenger ! */
    ruleAction.tag0VlanCmd             = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ruleAction.ttPassengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.sourceEPortAssignmentEnable = GT_TRUE;
    ruleAction.sourceEPort             = PRV_TGF_SOURCE_EPORT1_CNS; /* This is used by the L2MLL engine for source interface filtering, to prevent
                                                                      the packet from being sent back to the upstream RBridge on this tree*/
    /* A TRILL TTI rule is created per {E-RBID (i.e. distribution tree), I-RBID registered to use this tree*/
    /* AUTODOC: add TTI rule 80 with: */
    /* AUTODOC:   key TTI_KEY_TRILL, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern trillEgressRbid=0x6543, trillIngressRbid=0x4321 */
    /* AUTODOC:   action egressInterface eVidx=4097 */
    /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=0, tag1VlanCmd=VLAN_MODIFY_UNTAGGED */
    /* AUTODOC:   ttPassenger=ETHERNET_NO_CRC, srcEPort=701 */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_TRILL_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set Packet Instance 0 Transit Index = %d",PRV_TGF_TTI_INDEX_CNS);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern,    0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,       0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* configure TTI rule - default TTI rule, last among Multicast TRILL related rules in the TTI TCAM */
    pattern.trill.trillMBit = GT_TRUE;
    pattern.trill.trillMcDescriptorInstance = 0; /* this is the multi-destination transit packet instance */

    /* configure TTI rule mask - default TTI rule, last among Multicast TRILL related rules in the TTI TCAM */
    mask.trill.trillMBit = GT_TRUE;
    mask.trill.trillMcDescriptorInstance = 1;

    /* configure TTI rule action */
    ruleAction.command                 = CPSS_PACKET_CMD_DROP_HARD_E;
    /*changed from PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E; because when 'no modify' the re-parse not take vid0 from the passenger ! */
    ruleAction.tag0VlanCmd             = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;

    /* default TTI rule for Multicast*/
    /* AUTODOC: add TTI rule 100 with: */
    /* AUTODOC:   key TTI_KEY_TRILL, cmd DROP_HARD */
    /* AUTODOC:   pattern trillMcDescriptorInstance=0 */
    /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=0, tag1VlanCmd=VLAN_MODIFY_UNTAGGED */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_LAST_INDEX_CNS, PRV_TGF_TTI_KEY_TRILL_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set Packet Instance 0 Last MC Transit Index = %d",PRV_TGF_TTI_LAST_INDEX_CNS);

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    /* Map the eVIDX to a pointer of linked-list of L2MLL */
    lttEntry.mllPointer = 1;
    lttEntry.entrySelector = 0;

    /* AUTODOC: map eVIDX 4097 to 1 pointer of linked-list of L2MLL */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_EVIDX_CNS, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet");

     /* save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT1_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort 1001 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT1_CNS,
                                                       &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT1_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* set ePort mapping configuration */
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: map port 3 to ePort 1001 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT1_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* set TRILL tunnel start entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.trillCfg.vlanId                 = PRV_TGF_TRILL_VLANID_CNS;
    tunnelEntry.trillCfg.tagEnable              = GT_TRUE;
    tunnelEntry.trillCfg.trillHeader.version    = 0;
    tunnelEntry.trillCfg.trillHeader.mBit       = 0;
    tunnelEntry.trillCfg.trillHeader.opLength   = 0;
    cpssOsMemCpy(tunnelEntry.trillCfg.nextHopMacDa.arEther, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, version=mBit=opLength=0 */
    /* AUTODOC:   DA=01:80:C2:00:00:40 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /************* Configuration for TRILL To-ME packet: ************/
    PRV_UTF_LOG0_MAC("\nConfiguration for TRILL To-ME packet:\n");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern,    0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,       0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* configure TTI rule - The first rule is used to match TRILL End Station Address Distribution Information (ESADI) multi-destination packets */
    pattern.trill.trillMBit = GT_TRUE;
    pattern.trill.trillMcDescriptorInstance = 1; /* this is the multi-destination TRILL To-ME packet instance */
    cpssOsMemCpy((GT_VOID*)pattern.trill.innerPacketMacDa.arEther, /* ALL-ESADI-RBridges Multicast address */
                 (GT_VOID*)prvTgfPacketAllEsadiMacDa,
                 sizeof(pattern.trill.innerPacketMacDa.arEther));

    /* configure TTI rule mask - The first rule is used to match TRILL End Station Address Distribution Information (ESADI) multi-destination packets */
    mask.trill.trillMBit = GT_TRUE;
    mask.trill.trillMcDescriptorInstance = 1;
    cpssOsMemSet((GT_VOID*)mask.trill.innerPacketMacDa.arEther, 0xFF,
                 sizeof(mask.trill.innerPacketMacDa.arEther));

    /* configure TTI rule action */
    ruleAction.command                 = CPSS_PACKET_CMD_DROP_HARD_E; /* prevents ESADI packets from being bridged in the local VLAN */
    /*changed from PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E; because when 'no modify' the re-parse not take vid0 from the passenger ! */
    ruleAction.tag0VlanCmd             = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;

    /* default TTI rule for ESADI packets */
    /* AUTODOC: add TTI rule 1 with: */
    /* AUTODOC:   key TTI_KEY_TRILL, cmd DROP_HARD */
    /* AUTODOC:   pattern trillMBit=TRUE, trillMcDescriptorInstance=1 */
    /* AUTODOC:   pattern innerPacketMacDa=01:80:C2:00:00:42 */
    /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=0, tag1VlanCmd=VLAN_MODIFY_UNTAGGED */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX1_CNS, PRV_TGF_TTI_KEY_TRILL_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set Packet Instance 1 ESADI packet To-ME Index = %d",PRV_TGF_TTI_INDEX1_CNS);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern,    0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,       0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* configure TTI rule - second TRILL TTI rule is used for matching all non-ESADI multi-destination TRILL To-ME packet instances */
    pattern.trill.trillMBit = GT_TRUE;
    pattern.trill.trillMcDescriptorInstance = 1; /* this is the multi-destination TRILL To-ME packet instance */

    /* configure TTI rule mask - second TRILL TTI rule is used for matching all non-ESADI multi-destination TRILL To-ME packet instances */
    mask.trill.trillMBit = GT_TRUE;
    mask.trill.trillMcDescriptorInstance = 1;

    /* configure TTI rule action */
    ruleAction.command                 = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.tunnelTerminate         = GT_TRUE;
    /*changed from PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E; because when 'no modify' the re-parse not take vid0 from the passenger ! */
    ruleAction.tag0VlanCmd             = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ruleAction.ttPassengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;

    /* default TTI rule for Non-ESADI packets */
    /* AUTODOC: add TTI rule 20 with: */
    /* AUTODOC:   key TTI_KEY_TRILL, cmd FORWARD */
    /* AUTODOC:   pattern trillMBit=TRUE, trillMcDescriptorInstance=1 */
    /* AUTODOC:   ttPassenger=ETHERNET_NO_CRC */
    /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=0, tag1VlanCmd=VLAN_MODIFY_UNTAGGED */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX2_CNS, PRV_TGF_TTI_KEY_TRILL_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set Packet Instance 1 NON-ESADI packet To-ME Index = %d",PRV_TGF_TTI_INDEX2_CNS);

    /* RBID LTT table configuration */
    /*iRbidLttEntry.rbidEntryIndex = 11;
    eRbidLttEntry.rbidEntryIndex = 22;*/

    /* AUTODOC: config RBID LTT table 0x4321: */
    /* AUTODOC:   iRbidLtt=11, eRbidLtt=22 */
    /*rc = prvTgfTtiTrillRbidLttEntrySet(prvTgfDevNum, 0x4321, &iRbidLttEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidLttEntrySet);*/

    /* AUTODOC: config RBID LTT table 0x6543: */
    /* AUTODOC:   iRbidLtt=11, eRbidLtt=22 */
    /*rc = prvTgfTtiTrillRbidLttEntrySet(prvTgfDevNum, 0x6543, &eRbidLttEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidLttEntrySet);*/

#if 0
    /* RBID lookup table configuration */
    iRbidEntry.srcTrgPort = PRV_TGF_SOURCE_EPORT_CNS;
    iRbidEntry.trgHwDevice = prvTgfDevNum; /* own device */

    /* AUTODOC: set srcTrgPort=700 for RBID lookup table 11 */
    /*rc = prvTgfTtiTrillRbidEntrySet(prvTgfDevNum, iRbidLttEntry.rbidEntryIndex, &iRbidEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidEntrySet);*/

    eRbidEntry.srcTrgPort = PRV_TGF_DESTINATION_EPORT1_CNS;
    eRbidEntry.trgHwDevice = prvTgfDevNum; /* own device */
#endif

    /* AUTODOC: set srcTrgPort=1001 for RBID lookup table 22 */
    /*rc = prvTgfTtiTrillRbidEntrySet(prvTgfDevNum, eRbidLttEntry.rbidEntryIndex, &eRbidEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidEntrySet);*/

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    /* AUTODOC: map eVIDX 4096 to pointer 0 of linked-list of L2MLL */
    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_FLOOD_EVIDX_CNS, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

     /* This flag indicates that the packet arrived TRILL-encapsulated,
       and therefore should not be flooded on outgoing TRILL interfaces */

    /* save src port Ingress Vlan Filtering configuration */
    rc = prvTgfBrgVlanPortIngFltEnableGet(prvTgfDevNum, PRV_TGF_SOURCE_EPORT_CNS, &(prvTgfRestoreCfg.SrcPortIngFlt));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanPortIngFltEnableGet);

    /* When packet arrives on trill tunnel, bridge must bypass ingress VLAN filtering */
    /* AUTODOC: disable ingress filtering for ePort 700 */
    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, PRV_TGF_SOURCE_EPORT_CNS, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanPortIngFltEnable);

    /* disable VLAN egress filtering, TRILL packets are not subject to bridge VLAN ingress filtering */

    /* save VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnableGet(prvTgfDevNum, &(prvTgfRestoreCfg.vlanEgressFilteringState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnableGet);

    /* AUTODOC: disable VLAN egress filtering */
    rc = prvTgfBrgVlanEgressFilteringEnable(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* set the Outer Tag0 VID that must be for all TRILL packets from the port */
    rc = prvTgfTtiPortTrillOuterVid0Get(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.trillOuterVid0));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Get);

    /* AUTODOC: set Outer Tag0 VID to 6 for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* save VLAN egress port tag state mode */
    rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT1_CNS,
                                                &stateMode);
    prvTgfRestoreCfg.vlanEgressPortTagStateMode = stateMode;
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeGet);

    /* AUTODOC: set EPORT tag state mode for egress ePort 1001 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT1_CNS,
                                                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* save VLAN egress port tag state */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT1_CNS,
                                            &(prvTgfRestoreCfg.vlanEgressPortTagState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateGet);

    /* AUTODOC: set TAG0_CMD port tag state for egress ePort 1001 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT1_CNS,
                                            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

}

/**
* @internal prvTgfTunnelTermTrillUnknownUcTransitAndTerminationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelTermTrillUnknownUcTransitAndTerminationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter    = 0;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8                   packetsCountRx = 0;
    GT_U8                   packetsCountTx = 0;
    GT_U32                  packetSize = 0;
    GT_U32                  packetIter  = 0;
    GT_U32                  triggerBitmap = 0;
    TGF_VFD_INFO_STC        vfdArray[PRV_TGF_VFD_NUM_CNS];
    GT_U32                  vfdIndex = 0;/* index in vfdArray */
    PRV_TGF_BRG_MAC_ENTRY_STC macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC macEntryKey;

    /* AUTODOC: GENERATE TRAFFIC: */
     /******** Check packet on port prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] ********/
    PRV_UTF_LOG1_MAC("\n\n======= Check packet on port %d =======\n\n",prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Reset Counters
     */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:40, SA=00:00:00:00:00:50, VID=6 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    /* AUTODOC:   passenger EtherType=0x0600 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get tunneled packet on port 3 with: */
    /* AUTODOC:   DA=66:55:44:33:22:11, SA=00:00:00:00:00:06, VID=6 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x1F, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        switch (portIter)
        {
        case 0: /* PRV_TGF_SEND_PORT_IDX_CNS */
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS;
            break;
        case 1:
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = 0;
            packetSize     = PRV_TGF_NATIVE_EXPECTED_PACKET_LEN_CNS;
            break;
        case 2:
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
            break;
        case 3: /* PRV_TGF_RECEIVE_PORT_IDX_CNS */
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS;
            break;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    packetsCountRx,
                                    packetsCountTx,
                                    packetSize,
                                    prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
  }

 /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check TRILL da mac */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 0;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check TRILL Ethertype */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)((TGF_ETHERTYPE_22F3_TRILL_TAG_CNS >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[1] = (GT_U8)(TGF_ETHERTYPE_22F3_TRILL_TAG_CNS & 0xFF);
    vfdIndex++;

    /* check TRILL Hop Count decriment */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = PRV_TGF_TRILL_HOP_COUNT_SIZE_CNS;
    vfdArray[vfdIndex].patternPtr[0] = 0;
    vfdArray[vfdIndex].patternPtr[1] = (GT_U8)((prvTgfPacketTrillPart.hopCount - 1) & 0xFF);
    vfdIndex++;

    /* check passenger mac da */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = PASSENGER_OFFSET_CNS;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2PartInt.daMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check passenger native VLAN */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = PASSENGER_OFFSET_CNS + TGF_L2_HEADER_SIZE_CNS + 3;
    vfdArray[vfdIndex].cycleCount = 1; /* 8 bits of vlanId of passenger*/
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)(PRV_TGF_ETHERNET_VLANID_CNS);
    vfdIndex++;

    /* check VLAN-ID of the TRILL TS */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + 3;
    vfdArray[vfdIndex].cycleCount = 1; /* 8 bits of vlanId of TS */
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)(PRV_TGF_TRILL_VLANID_CNS);
    vfdIndex++;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                vfdIndex, vfdArray, &triggerBitmap);
    PRV_UTF_LOG2_MAC("triggerBitmap = %d, rc = 0x%02X\n\n", triggerBitmap, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
    {

        /* check if captured packet has all correct vfd's */
        UTF_VERIFY_EQUAL1_STRING_MAC((1 << vfdIndex) - 1,
                                     triggerBitmap & ((1 << vfdIndex) - 1),
                                     "   Errors while triggers summary result checking: packetIter =  %d\n", packetIter);

        /* check if captured packet has expected TRILL da mac */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, triggerBitmap & BIT_0,
                                     "\n   TRILL MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfTunnelStartEntryMacDa[0],
                                     prvTgfTunnelStartEntryMacDa[1],
                                     prvTgfTunnelStartEntryMacDa[2],
                                     prvTgfTunnelStartEntryMacDa[3],
                                     prvTgfTunnelStartEntryMacDa[4],
                                     prvTgfTunnelStartEntryMacDa[5]);

        /* check if captured packet has expected TRILL Ethertype */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_1) >> 1,
                                     "\n   Ethertype of captured packet must be: %04X",
                                     TGF_ETHERTYPE_22F3_TRILL_TAG_CNS);

        /* Verify the TRILL header <Hop Count> is decremented */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_2) >> 2,
                                     "\n   TRILL Hop Count must be: %02X",
                                     (prvTgfPacketTrillPart.hopCount - 1));

        /* check if captured packet has expected passenger mac da */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_3) >> 3,
                                     "\n   Passenger MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2PartInt.daMac[0],
                                     prvTgfPacketL2PartInt.daMac[1],
                                     prvTgfPacketL2PartInt.daMac[2],
                                     prvTgfPacketL2PartInt.daMac[3],
                                     prvTgfPacketL2PartInt.daMac[4],
                                     prvTgfPacketL2PartInt.daMac[5]);

        /* check if captured packet has expected passenger native VLAN */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_4) >> 4,
                                     "\n   Passenger native VLAN of captured packet must be: %02X",
                                     PRV_TGF_ETHERNET_VLANID_CNS);

        /* check if captured packet has expected passenger native VLAN */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_5) >> 5,
                                     "\n   TS VLAN (not of passenger) of captured packet must be: %02X",
                                     PRV_TGF_TRILL_VLANID_CNS);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }

    /* AUTODOC: verify No MAC learning for forwarding is performed */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_TRILL_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2PartInt.saMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

    UTF_VERIFY_EQUAL6_STRING_MAC(GT_NOT_FOUND, rc, "Device has learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                                 prvTgfPacketL2PartInt.saMac[0],prvTgfPacketL2PartInt.saMac[1],prvTgfPacketL2PartInt.saMac[2],
                                 prvTgfPacketL2PartInt.saMac[3],prvTgfPacketL2PartInt.saMac[4],prvTgfPacketL2PartInt.saMac[5]);

    /******** Check packet on port prvTgfPortsArray[1] ********/
    PRV_UTF_LOG1_MAC("\n\n======= Check packet on port %d =======\n\n",prvTgfPortsArray[1]);
    /* -------------------------------------------------------------------------
     * 1. Reset Counters
     */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
    vfdIndex = 0;

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* send packet with mirroring on port */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[1],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
    /* AUTODOC:   EtherType=0x0600 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        switch (portIter)
        {
        case 0: /* PRV_TGF_SEND_PORT_IDX_CNS */
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS;
            break;
        case 1:
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_NATIVE_EXPECTED_PACKET_LEN_CNS;
            break;
        case 2:
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
            break;
        case 3: /* PRV_TGF_RECEIVE_PORT_IDX_CNS */
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = 0;
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS;
            break;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    packetsCountRx,
                                    packetsCountTx,
                                    packetSize,
                                    prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[1];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check Ethernet mac da */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 0;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2PartInt.daMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check Ethernet mac sa */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 6;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2PartInt.saMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check Ethernet payload */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 6;
    vfdArray[vfdIndex].patternPtr[0] = prvTgfPayloadDataArr[0];
    vfdArray[vfdIndex].patternPtr[1] = prvTgfPayloadDataArr[1];
    vfdArray[vfdIndex].patternPtr[2] = prvTgfPayloadDataArr[2];
    vfdArray[vfdIndex].patternPtr[3] = prvTgfPayloadDataArr[3];
    vfdArray[vfdIndex].patternPtr[4] = prvTgfPayloadDataArr[4];
    vfdArray[vfdIndex].patternPtr[5] = prvTgfPayloadDataArr[5];
    vfdIndex++;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                vfdIndex, vfdArray, &triggerBitmap);
    PRV_UTF_LOG2_MAC("triggerBitmap = %d, rc = 0x%02X\n\n", triggerBitmap, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
    {
        /* check if captured packet has all correct vfd's */
        UTF_VERIFY_EQUAL1_STRING_MAC((1 << vfdIndex) - 1,
                                     triggerBitmap & ((1 << vfdIndex) - 1),
                                     "   Errors while triggers summary result checking: packetIter =  %d\n", packetIter);

        /* check if captured packet has expected Ethernet mac da*/
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, triggerBitmap & BIT_0,
                                     "\n   TRILL MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2PartInt.daMac[0],
                                     prvTgfPacketL2PartInt.daMac[1],
                                     prvTgfPacketL2PartInt.daMac[2],
                                     prvTgfPacketL2PartInt.daMac[3],
                                     prvTgfPacketL2PartInt.daMac[4],
                                     prvTgfPacketL2PartInt.daMac[5]);

        /* check if captured packet has expected Ethernet mac da*/
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_1) >> 1,
                                     "\n   TRILL MAC SA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2PartInt.saMac[0],
                                     prvTgfPacketL2PartInt.saMac[1],
                                     prvTgfPacketL2PartInt.saMac[2],
                                     prvTgfPacketL2PartInt.saMac[3],
                                     prvTgfPacketL2PartInt.saMac[4],
                                     prvTgfPacketL2PartInt.saMac[5]);

        /* check if captured packet has expected Ethernet payload */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_2) >> 2,
                                     "\n   Payload of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPayloadDataArr[0],
                                     prvTgfPayloadDataArr[1],
                                     prvTgfPayloadDataArr[2],
                                     prvTgfPayloadDataArr[3],
                                     prvTgfPayloadDataArr[4],
                                     prvTgfPayloadDataArr[5]);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }
}

/**
* @internal prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;
    PRV_TGF_TTI_TRILL_ADJACENCY_STC             adjacencyPattern;
    PRV_TGF_TUNNEL_START_ENTRY_UNT              tunnelEntry;
    GT_U32                                      lttIndexArray[1];
    GT_U32                                      mllIndexArray[3];

    /* AUTODOC: RESTORE CONFIGURATION: */
    cpssOsMemSet((GT_VOID*) &adjacencyPattern, 0, sizeof(adjacencyPattern));
    cpssOsMemSet((GT_VOID*) &tunnelEntry,      0, sizeof(tunnelEntry));

    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0xFFF;

    /* use 0 MAC DA and VID = 4095 */
    macToMePattern.mac.arEther[0] =
    macToMePattern.mac.arEther[1] =
    macToMePattern.mac.arEther[2] =
    macToMePattern.mac.arEther[3] =
    macToMePattern.mac.arEther[4] =
    macToMePattern.mac.arEther[5] = 0;
    macToMePattern.vlanId = 0xFFF;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, tgfTrafficTableRxPcktTblClear);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ETHERNET_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* AUTODOC: restore TRILL ethertype 0x22F3 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E, prvTgfRestoreCfg.trillEtherType);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeSet);

    /* AUTODOC: restore TRILL engine state on port 0 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillEnableTransmitPort);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

     /* restore trill engine enable on destination ePort */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillEnableDestinationPort);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_TRILL */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_TRILL_E, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortLookupEnableSet);

    /* AUTODOC: invalidate TTI rules [80,1,20,100] */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiRuleValidStatusSet);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX1_CNS, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiRuleValidStatusSet);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX2_CNS, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiRuleValidStatusSet);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_LAST_INDEX_CNS, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiRuleValidStatusSet);

    lttIndexArray[0] = PRV_TGF_FLOOD_EVIDX_CNS;
    mllIndexArray[0] = 0;
    mllIndexArray[1] = 1;
    mllIndexArray[2] = 2;

    /* AUTODOC: clear MLL entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 3);

    lttIndexArray[0] = PRV_TGF_EVIDX_CNS;
    mllIndexArray[1] = 1;
    mllIndexArray[2] = 2;
    /* clear MLL entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 2);


    /* AUTODOC: restore L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: restore Outer Tag0 VID for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillOuterVid0);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* AUTODOC: restore default of MAC to ME entry 1 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* AUTODOC: restore default of Adjacency table entry 1 */
    rc = prvTgfTtiTrillAdjacencyCheckEntrySet(prvTgfDevNum, PRV_TGF_ADJACENCY_INDEX_CNS, &adjacencyPattern);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillAdjacencyCheckEntrySet);

    /* AUTODOC: restore ePort 1001 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT1_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort 1001 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT1_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* AUTODOC: restore default of tunnel start table entry 8 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* AUTODOC: restore VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnable(prvTgfRestoreCfg.vlanEgressFilteringState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* AUTODOC: restore tag state mode for egress ePort 700 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_SOURCE_EPORT_CNS,
                                                prvTgfRestoreCfg.vlanEgressPortTagStateMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: restore port tag state for egress ePort 700 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SOURCE_EPORT_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* AUTODOC: restore ingress filtering for ePort 700 */
    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, PRV_TGF_SOURCE_EPORT_CNS, prvTgfRestoreCfg.SrcPortIngFlt);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanPortIngFltEnable);
}


