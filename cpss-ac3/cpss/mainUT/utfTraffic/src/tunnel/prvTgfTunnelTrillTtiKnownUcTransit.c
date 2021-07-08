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
* @file prvTgfTunnelTrillTtiKnownUcTransit.c
*
* @brief Tunnel: Known Unicast: Transit TRILL packet -> TRILL packet
*
* @version   8
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
#include <tunnel/prvTgfTunnelTrillTtiKnownUcTransit.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* TRILL Hop Count (in bytes)*/
#define PRV_TGF_TRILL_HOP_COUNT_SIZE_CNS    2

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            5

/* Destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS    1000

/* Source ePort */
#define PRV_TGF_SOURCE_EPORT_CNS          700

/* Tti entry index */
#define PRV_TGF_TTI_INDEX_CNS              80

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
#define PRV_TGF_VFD_NUM_CNS                 10

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

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x55, 0x44, 0x33, 0x22, 0x11},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x05}                 /* saMac */
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
    GT_FALSE,           /* mBit */
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
    0, 0, PRV_TGF_TRILL_VLANID_CNS          /* pri, cfi, VlanId */
};

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
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of transmitted packet */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2   + \
    TGF_VLAN_TAG_SIZE_CNS  * 2   + \
    TGF_ETHERTYPE_SIZE_CNS       + \
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

/* LENGTH of expected packet (original packet tunneled over TRILL packet) */
#define PRV_TGF_EXPECTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS  + sizeof(prvTgfPayloadDataArr)


/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfTunnelStartEntryMacDa = {
    0x66, 0x55, 0x44, 0x33, 0x22, 0x11
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

} prvTgfRestoreCfg;

/******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTrillKnownUcTransitConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelTrillKnownUcTransitConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT                        pattern;
    PRV_TGF_TTI_RULE_UNT                        mask;
    PRV_TGF_TTI_ACTION_STC                      ruleAction = {0};
    PRV_TGF_TTI_TRILL_ADJACENCY_STC             adjacencyPattern;
    PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC      iRbidEntry;
    PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC      eRbidEntry;
    /*PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  iRbidLttEntry;
    PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  eRbidLttEntry;*/
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;
    PRV_TGF_TUNNEL_START_ENTRY_UNT              tunnelEntry;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &adjacencyPattern, 0, sizeof(adjacencyPattern));
    cpssOsMemSet((GT_VOID*) &iRbidEntry,       0, sizeof(iRbidEntry));
    cpssOsMemSet((GT_VOID*) &eRbidEntry,       0, sizeof(eRbidEntry));
    /*cpssOsMemSet((GT_VOID*) &iRbidLttEntry,    0, sizeof(iRbidLttEntry));
    cpssOsMemSet((GT_VOID*) &eRbidLttEntry,    0, sizeof(eRbidLttEntry));*/
    cpssOsMemSet((GT_VOID*) &macToMePattern,   0, sizeof(macToMePattern));
    cpssOsMemSet((GT_VOID*) &macToMeMask,      0, sizeof(macToMeMask));
    cpssOsMemSet((GT_VOID*) &tunnelEntry,      0, sizeof(tunnelEntry));
    cpssOsMemSet((GT_VOID*) &egressInfo,       0, sizeof(egressInfo));
    cpssOsMemSet((GT_VOID*) &physicalInfo,     0, sizeof(physicalInfo));

    /* AUTODOC: create TRILL VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 4);
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
    rc = prvTgfTtiPortTrillEnableGet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT_CNS, &(prvTgfRestoreCfg.trillEnableDestinationPort));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableGet);

    /* AUTODOC: enable TRILL engine on ePort 1000 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT_CNS, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_TRILL */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_TRILL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* configure TTI rule for TRILL */
    pattern.trill.trillMBit = GT_FALSE;

    /* configure TTI rule mask for TRILL */
    mask.trill.trillMBit = GT_TRUE;

    /* configure TTI rule action */
    ruleAction.command                 = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.tunnelTerminate         = GT_FALSE;
    ruleAction.enableDecTtl            = GT_TRUE;
    ruleAction.ingressPipeBypass       = GT_TRUE;
    /*changed from PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E; because when 'no modify' the re-parse not take vid0 from the passenger ! */
    ruleAction.tag0VlanCmd             = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ruleAction.passengerPacketType     = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;

    /* set TTI rule - A single TTI TRILL rule is used to match all Unicast TRILL transit packets.
       Use any index greater than that used for Egress RBridge rules (This TCAM rule serves as a default Unicast
       TRILL rule and must be located after the TTI TRILL rule used for matching TRILL Unicast-to-me packets */

    /* AUTODOC: add TTI rule 80 with: */
    /* AUTODOC:   key TRILL, cmd FORWARD */
    /* AUTODOC:   pattern trillMBit=0 */
    /* AUTODOC:   action tag0VlanCmd=DO_NOT_MODIFY,tag1VlanCmd=MODIFY_UNTAGGED */
    /* AUTODOC:   action passenger=ETHERNET_NO_CRC */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_TRILL_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

     /* save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort 1000 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT_CNS,
                                                       &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* set ePort mapping configuration */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: map port 3 to ePort 1000 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
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
    /* AUTODOC:   vlanId=5, version=mBit=opLength=0 */
    /* AUTODOC:   DA=66:55:44:33:22:11 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

     /* set the Outer Tag0 VID that must be for all TRILL packets from the port
        The TRILL standard specifies that all TRILL packets on a link, if tagged, must use the VLAN-ID as specified by the designated RBridge */
    rc = prvTgfTtiPortTrillOuterVid0Get(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.trillOuterVid0));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Get);

    /* AUTODOC: set Outer Tag0 VID to 5 for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_TRILL_VLANID_CNS;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table 1: */
    /* AUTODOC:   DA=00:55:44:33:22:11, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.saMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));
    adjacencyPattern.trillMBit      = 0;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* AUTODOC: config entry in TRILL Adjacency Check dedicated TCAM: */
    /* AUTODOC:   SA=00:00:00:00:00:05, port=0 */
    rc = prvTgfTtiTrillAdjacencyCheckEntrySet(prvTgfDevNum, PRV_TGF_ADJACENCY_INDEX_CNS, &adjacencyPattern);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillAdjacencyCheckEntrySet);

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

    /* RBID lookup table configuration */
    iRbidEntry.srcTrgPort = PRV_TGF_SOURCE_EPORT_CNS;
    iRbidEntry.trgHwDevice = prvTgfDevNum; /* own device */

    /* AUTODOC: set srcTrgPort=700 for RBID lookup table 11 */
    /*rc = prvTgfTtiTrillRbidEntrySet(prvTgfDevNum, iRbidLttEntry.rbidEntryIndex, &iRbidEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidEntrySet);*/

    eRbidEntry.srcTrgPort = PRV_TGF_DESTINATION_EPORT_CNS;
    eRbidEntry.trgHwDevice = prvTgfDevNum; /* own device */

    /* AUTODOC: set srcTrgPort=1000 for RBID lookup table 22 */
    /*rc = prvTgfTtiTrillRbidEntrySet(prvTgfDevNum, eRbidLttEntry.rbidEntryIndex, &eRbidEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidEntrySet);*/

}

/**
* @internal prvTgfTunnelTrillKnownUcTransitTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelTrillKnownUcTransitTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter    = 0;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8                   packetsCountRx;
    GT_U8                   packetsCountTx;
    GT_U32                  packetSize = 0;
    GT_U32                  packetIter  = 0;
    GT_U32                  triggerBitmap = 0;
    TGF_VFD_INFO_STC        vfdArray[PRV_TGF_VFD_NUM_CNS];
    GT_U32                  vfdIndex = 0;/* index in vfdArray */
    PRV_TGF_BRG_MAC_ENTRY_STC macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC macEntryKey;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
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
    /* AUTODOC:   DA=66:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x1F, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (portIter == PRV_TGF_SEND_PORT_IDX_CNS)
        {
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS;
        }
        else if (portIter == PRV_TGF_RECEIVE_PORT_IDX_CNS)
        {
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS;
        }
        else
        {
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
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
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)(PRV_TGF_TRILL_VLANID_CNS);
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
                                     PRV_TGF_TRILL_VLANID_CNS);

        /* check if captured packet has expected passenger native VLAN */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_5) >> 5,
                                     "\n   TS VLAN (not of passenger) of captured packet must be: %02X",
                                     PRV_TGF_TRILL_VLANID_CNS);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }

    /* Verify No MAC learning for forwarding is performed */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_TRILL_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2PartInt.saMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

    UTF_VERIFY_EQUAL6_STRING_MAC(GT_NOT_FOUND, rc, "Device has learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                                 prvTgfPacketL2PartInt.saMac[0],prvTgfPacketL2PartInt.saMac[1],prvTgfPacketL2PartInt.saMac[2],
                                 prvTgfPacketL2PartInt.saMac[3],prvTgfPacketL2PartInt.saMac[4],prvTgfPacketL2PartInt.saMac[5]);
}

/**
* @internal prvTgfTunnelTrillKnownUcTransmitConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTrillKnownUcTransmitConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;

    /* AUTODOC: RESTORE CONFIGURATION: */
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

    /* AUTODOC: restore TRILL ethertype 0x22F3 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E, prvTgfRestoreCfg.trillEtherType);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeSet);

    /* AUTODOC: restore TRILL engine state on port 0 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillEnableTransmitPort);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: restore TRILL engine state on ePort 1000 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillEnableDestinationPort);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_TRILL */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_TRILL_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);

    /* AUTODOC: restore Outer Tag0 VID for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillOuterVid0);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* AUTODOC: restore default of MAC to ME entry 1 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* AUTODOC: restore ePort 1000 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort 1000 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
}


