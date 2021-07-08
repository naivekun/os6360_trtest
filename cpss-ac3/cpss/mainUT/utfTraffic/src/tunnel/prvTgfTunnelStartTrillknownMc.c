/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfTunnelStartTrillKnownMc.c
*
* DESCRIPTION:
*       Tunnel: TRILL tunnel start - Known MC
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
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
#include <common/tgfTunnelGen.h>
#include <common/tgfL2MllGen.h>
#include <l2mll/prvTgfL2MllUtils.h>
#include <tunnel/prvTgfTunnelStartTrillKnownMc.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Ethernet VLAN Id */
#define PRV_TGF_ETHERNET_VLANID_CNS         5

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            6

/* eVidx to a pointer of linked-list of L2MLL */
#define PRV_TGF_EVIDX_CNS                  4096

/* FDB entry destination ePort */
#define PRV_TGF_DESTINATION_EPORT1_CNS      900

/* FDB entry destination ePort */
#define PRV_TGF_DESTINATION_EPORT2_CNS      1000

/* number of ePorts in the MLL group */
#define PRV_TGF_NUM_OF_EPORTS_IN_MLL_GROUP_CNS  2

/* first ePort in the MLL group */
#define MLL_ENTRY_INDEX_0_CNS               0

/* second ePort in the MLL group */
#define MLL_ENTRY_INDEX_1_CNS               1

/* Tunnel start entry index 1 */
#define PRV_TGF_TUNNEL_START_INDEX1_CNS     8

/* Tunnel start entry index 2 */
#define PRV_TGF_TUNNEL_START_INDEX2_CNS     4

/* Tunnel start up 1*/
#define PRV_TGF_TUNNEL_START_UP1_CNS        3

/* Tunnel start up 2*/
#define PRV_TGF_TUNNEL_START_UP2_CNS        2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           0

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT1_IDX_CNS       2

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT2_IDX_CNS       3

/* max number of VFD to use */
#define PRV_TGF_VFD_NUM_CNS                 10

/* offset of the passenger in the packet*/
#define PASSENGER_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
                                 TGF_ETHERTYPE_SIZE_CNS + TGF_TRILL_HEADER_SIZE_CNS)

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};

/* VLAN_TAG part of packet */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_ETHERNET_VLANID_CNS                   /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {0x600};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
};

/* PAYLOAD part of packet*/
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of transmitted packet */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_PACKET_PART_ETHERTYPE_E + sizeof(prvTgfPacketPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_TRANSMITTED_PACKET_LEN_CNS,                          /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* LENGTH of expected packet (original packet tunneled over TRILL packet) */
#define PRV_TGF_EXPECTED_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_TRILL_HEADER_SIZE_CNS + PRV_TGF_TRANSMITTED_PACKET_LEN_CNS)

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* MAC for Tunnel Start Entry - "All-RBridges MC address" */
static TGF_MAC_ADDR prvTgfTunnelStartEntryMacDa = {
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x40
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo[PRV_TGF_NUM_OF_EPORTS_IN_MLL_GROUP_CNS];
    CPSS_INTERFACE_INFO_STC                     physicalInfo[PRV_TGF_NUM_OF_EPORTS_IN_MLL_GROUP_CNS];
    GT_BOOL                                     vlanEgressFilteringState;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   vlanEgressPortTagStateMode[PRV_TGF_NUM_OF_EPORTS_IN_MLL_GROUP_CNS];
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT           vlanEgressPortTagState[PRV_TGF_NUM_OF_EPORTS_IN_MLL_GROUP_CNS];
} prvTgfRestoreCfg;

/******************************************************************************/
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelStartTrillKnownMcConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelStartTrillKnownMcConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT              tunnelEntry;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                lttEntry;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create Ethernet VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_ETHERNET_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* AUTODOC: create TRILL VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* AUTODOC: add FDB entry with MAC 01:00:5E:01:01:01, VLAN 5, eVidx 4096 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_ETHERNET_VLANID_CNS,
                                          PRV_TGF_EVIDX_CNS, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefFdbMacEntryOnVidxSet);

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: enable eVidx for all lookup */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d", GT_TRUE);

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    /* Map the eVIDX to a pointer of linked-list of L2MLL */
    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: map eVIDX 4096 to 0 pointer of linked-list of L2MLL */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_EVIDX_CNS, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    /* MLL entry :Trg Vidx = 0xFFF (All native Ethernet VLAN port members */
    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
    mllPairEntry.nextPointer = 1;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VID_E; /* vidx = 0xFFF: All native Ethernet VLAN port members */
    mllPairEntry.firstMllNode.egressInterface.vlanId = PRV_TGF_ETHERNET_VLANID_CNS;

    /* MLL entry : Trg ePort */
    mllPairEntry.secondMllNode.last = GT_FALSE;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT1_CNS;


    /* AUTODOC: set L2 MLL entry 0: */
    /* AUTODOC:   nextPointer=1, vlanId=5 , ePort=900 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");


    /* MLL entry : Trg ePort */
    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT2_CNS;

    /* AUTODOC: set L2 MLL entry 1: */
    /* AUTODOC:   nextMllPointer=0, ePort=1000 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 1");

    /* save ePort PRV_TGF_DESTINATION_EPORT1_CNS attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT1_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo[MLL_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* save ePort PRV_TGF_DESTINATION_EPORT2_CNS attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT2_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo[MLL_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* set ePort PRV_TGF_DESTINATION_EPORT1_CNS attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX1_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort 900 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT1_CNS,
                                                       &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* set ePort PRV_TGF_DESTINATION_EPORT2_CNS attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX2_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort 1000 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=4 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT2_CNS,
                                                       &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* save ePort PRV_TGF_DESTINATION_EPORT1_CNS mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT1_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo[MLL_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* save ePort PRV_TGF_DESTINATION_EPORT2_CNS mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT2_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo[MLL_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* set ePort PRV_TGF_DESTINATION_EPORT1_CNS mapping configuration */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT1_IDX_CNS];

    /* AUTODOC: map port 2 to ePort 900 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT1_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* set ePort PRV_TGF_DESTINATION_EPORT2_CNS mapping configuration */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT2_IDX_CNS];

    /* AUTODOC: map port 3 to ePort 1000 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT2_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* set TRILL tunnel start entry index PRV_TGF_TUNNEL_START_INDEX1_CNS */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.trillCfg.upMarkMode             = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.trillCfg.up                     = 0x3;
    tunnelEntry.trillCfg.tagEnable              = GT_TRUE;
    tunnelEntry.trillCfg.vlanId                 = PRV_TGF_TRILL_VLANID_CNS;
    cpssOsMemCpy(tunnelEntry.trillCfg.nextHopMacDa.arEther, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));
    tunnelEntry.trillCfg.trillHeader.version    = 0;
    tunnelEntry.trillCfg.trillHeader.mBit       = 1;
    tunnelEntry.trillCfg.trillHeader.opLength   = 0;
    tunnelEntry.trillCfg.trillHeader.hopCount   = 0x28;
    tunnelEntry.trillCfg.trillHeader.eRbid      = 0x6543;
    tunnelEntry.trillCfg.trillHeader.iRbid      = 0x4321;

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, up=0x3, upMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   mBit=1, hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=01:80:C2:00:00:40 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX1_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* set TRILL tunnel start entry index PRV_TGF_TUNNEL_START_INDEX2_CNS */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.trillCfg.upMarkMode             = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.trillCfg.up                     = 0x2;
    tunnelEntry.trillCfg.tagEnable              = GT_TRUE;
    tunnelEntry.trillCfg.vlanId                 = PRV_TGF_TRILL_VLANID_CNS;
    cpssOsMemCpy(tunnelEntry.trillCfg.nextHopMacDa.arEther, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));
    tunnelEntry.trillCfg.trillHeader.version    = 0;
    tunnelEntry.trillCfg.trillHeader.mBit       = 1;
    tunnelEntry.trillCfg.trillHeader.opLength   = 0;
    tunnelEntry.trillCfg.trillHeader.hopCount   = 0x22;
    tunnelEntry.trillCfg.trillHeader.eRbid      = 0x6543;
    tunnelEntry.trillCfg.trillHeader.iRbid      = 0x4321;

    /* AUTODOC: add Tunnel Start entry 4 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, up=0x2, upMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   mBit=1, hopCount=0x22, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=01:80:C2:00:00:40 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX2_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* disable VLAN egress filtering to allow destination port to be member
       in TRILL VLAN and not be member of the native ETHERNET VLAN */

    /* save VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnableGet(prvTgfDevNum,&(prvTgfRestoreCfg.vlanEgressFilteringState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnableGet);

    /* AUTODOC: disable VLAN egress filtering */
    rc = prvTgfBrgVlanEgressFilteringEnable(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* save VLAN egress port PRV_TGF_DESTINATION_EPORT1_CNS tag state mode */
    rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT1_CNS,
                                                &(prvTgfRestoreCfg.vlanEgressPortTagStateMode[MLL_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeGet);

    /* save VLAN egress port PRV_TGF_DESTINATION_EPORT2_CNS tag state mode */
    rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT2_CNS,
                                                &(prvTgfRestoreCfg.vlanEgressPortTagStateMode[MLL_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeGet);

    /* AUTODOC: set EPORT tag state mode for egress ePort 900 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT1_CNS,
                                                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: set EPORT tag state mode for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT2_CNS,
                                                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* save VLAN egress port PRV_TGF_DESTINATION_EPORT1_CNS tag state */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT1_CNS,
                                            &(prvTgfRestoreCfg.vlanEgressPortTagState[MLL_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateGet);

    /* save VLAN egress port PRV_TGF_DESTINATION_EPORT2_CNS tag state */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT2_CNS,
                                            &(prvTgfRestoreCfg.vlanEgressPortTagState[MLL_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateGet);

    /* AUTODOC: set TAG0_CMD port tag state for egress ePort 900 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT1_CNS,
                                            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* AUTODOC: set TAG0_CMD port tag state for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT2_CNS,
                                            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);
}

/**
* @internal prvTgfTunnelStartTrillKnownMcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelStartTrillKnownMcTrafficGenerate
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

    /* AUTODOC: GENERATE TRAFFIC: */
    /******** Check packet on port prvTgfPortsArray[PRV_TGF_RECEIVE_PORT1_IDX_CNS] ********/
    PRV_UTF_LOG1_MAC("\nCheck packet on port %d\n",prvTgfPortsArray[PRV_TGF_RECEIVE_PORT1_IDX_CNS]);

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

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:5E:01:01:01, SA=00:00:00:00:00:33, VID=5 */
    /* AUTODOC:   EtherType=0x0600 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT1_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get tunneled packet on port 2 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:40, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3, mBit=1, hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=01:00:5E:01:01:01, SA=00:00:00:00:00:33, VID=5 */
    /* AUTODOC:   passenger EtherType=0x0600 */
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
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;
            break;
        case 2: /* PRV_TGF_RECEIVE_PORT1_IDX_CNS */
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_EXPECTED_PACKET_LEN_CNS ;
            break;
        case 3: /* PRV_TGF_RECEIVE_PORT2_IDX_CNS */
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = 0;
            packetSize     = PRV_TGF_EXPECTED_PACKET_LEN_CNS;
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
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT1_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check TRILL da mac */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 0;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check UP and TRILL vlan */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 4;
    vfdArray[vfdIndex].patternPtr[0] = 0x81;
    vfdArray[vfdIndex].patternPtr[1] = 0x00;
    vfdArray[vfdIndex].patternPtr[2] = (GT_U8)(PRV_TGF_TUNNEL_START_UP1_CNS << 5);
    vfdArray[vfdIndex].patternPtr[3] = (GT_U8)PRV_TGF_TRILL_VLANID_CNS;
    vfdIndex++;

    /* check TRILL Ethertype and TRILL header format */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 8;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)((TGF_ETHERTYPE_22F3_TRILL_TAG_CNS >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[1] = (GT_U8)(TGF_ETHERTYPE_22F3_TRILL_TAG_CNS & 0xFF);
    vfdArray[vfdIndex].patternPtr[2] = 0x10;
    vfdArray[vfdIndex].patternPtr[3] = (GT_U8)(0x28 << 2);
    vfdArray[vfdIndex].patternPtr[4] = (GT_U8)((0x6543 >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[5] = (GT_U8)(0x6543 & 0xFF);
    vfdArray[vfdIndex].patternPtr[6] = (GT_U8)((0x4321 >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[7] = (GT_U8)(0x4321 & 0xFF);
    vfdIndex++;

    /* check passenger mac da */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = PASSENGER_OFFSET_CNS;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check passenger native VLAN */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = PASSENGER_OFFSET_CNS + TGF_L2_HEADER_SIZE_CNS + 3;
    vfdArray[vfdIndex].cycleCount = 1; /* 8 bits of vlanId of passenger*/
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)(PRV_TGF_ETHERNET_VLANID_CNS);
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

         /* check if captured packet has expected UP and TRILL vlan */
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_1) >> 1,
                                     "\n   UP of captured packet must be: %02X\n VLAN of captured packet must be: %02X",
                                     PRV_TGF_TUNNEL_START_UP1_CNS,
                                     PRV_TGF_TRILL_VLANID_CNS);

        /* check if captured packet has expected TRILL Ethertype and TRILL header format */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_2) >> 2,
                                     "\n Ethertype of captured packet must be: %04X\n TRILL M bit must be: 0x1"
                                     "\n TRILL hop count must be: 0x28\n TRILL eRbid must be: 0x6543\n TRILL iRbid must be: 0x4321",
                                     TGF_ETHERTYPE_22F3_TRILL_TAG_CNS);

        /* check if captured packet has expected passenger mac da */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_3) >> 3,
                                     "\n   Passenger MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2Part.daMac[0],
                                     prvTgfPacketL2Part.daMac[1],
                                     prvTgfPacketL2Part.daMac[2],
                                     prvTgfPacketL2Part.daMac[3],
                                     prvTgfPacketL2Part.daMac[4],
                                     prvTgfPacketL2Part.daMac[5]);

        /* check if captured packet has expected passenger native VLAN */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_4) >> 4,
                                     "\n   Passenger native VLAN of captured packet must be: %02X",
                                     PRV_TGF_ETHERNET_VLANID_CNS);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }

    /******** Check packet on port prvTgfPortsArray[PRV_TGF_RECEIVE_PORT2_IDX_CNS] ********/
    PRV_UTF_LOG1_MAC("\nCheck packet on port %d\n",prvTgfPortsArray[PRV_TGF_RECEIVE_PORT2_IDX_CNS]);
    /* -------------------------------------------------------------------------
     * 1. Reset Counters
     */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
    vfdIndex = 0;

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:5E:01:01:01, SA=00:00:00:00:00:33, VID=5 */
    /* AUTODOC:   EtherType=0x0600 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT2_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get tunneled packet on port 2 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:40, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3, mBit=1, hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=01:00:5E:01:01:01, SA=00:00:00:00:00:33, VID=5 */
    /* AUTODOC:   passenger EtherType=0x0600 */
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
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;
            break;
        case 2:
            packetsCountRx = 0;
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetSize = PRV_TGF_EXPECTED_PACKET_LEN_CNS;
            break;
        case 3: /* PRV_TGF_RECEIVE_PORT_IDX_CNS */
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_EXPECTED_PACKET_LEN_CNS;
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
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT2_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check TRILL da mac */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 0;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check UP and TRILL vlan */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 4;
    vfdArray[vfdIndex].patternPtr[0] = 0x81;
    vfdArray[vfdIndex].patternPtr[1] = 0x00;
    vfdArray[vfdIndex].patternPtr[2] = (GT_U8)(PRV_TGF_TUNNEL_START_UP2_CNS << 5);
    vfdArray[vfdIndex].patternPtr[3] = (GT_U8)PRV_TGF_TRILL_VLANID_CNS;
    vfdIndex++;

    /* check TRILL Ethertype and TRILL header format */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 8;
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)((TGF_ETHERTYPE_22F3_TRILL_TAG_CNS >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[1] = (GT_U8)(TGF_ETHERTYPE_22F3_TRILL_TAG_CNS & 0xFF);
    vfdArray[vfdIndex].patternPtr[2] = 0x10;
    vfdArray[vfdIndex].patternPtr[3] = (GT_U8)(0x22 << 2);
    vfdArray[vfdIndex].patternPtr[4] = (GT_U8)((0x6543 >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[5] = (GT_U8)(0x6543 & 0xFF);
    vfdArray[vfdIndex].patternPtr[6] = (GT_U8)((0x4321 >> 8) & 0xFF);
    vfdArray[vfdIndex].patternPtr[7] = (GT_U8)(0x4321 & 0xFF);
    vfdIndex++;

    /* check passenger mac da */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = PASSENGER_OFFSET_CNS;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check passenger native VLAN */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = PASSENGER_OFFSET_CNS + TGF_L2_HEADER_SIZE_CNS + 3;
    vfdArray[vfdIndex].cycleCount = 1; /* 8 bits of vlanId of passenger*/
    vfdArray[vfdIndex].patternPtr[0] = (GT_U8)(PRV_TGF_ETHERNET_VLANID_CNS);
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

         /* check if captured packet has expected UP and TRILL vlan */
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_1) >> 1,
                                     "\n   UP of captured packet must be: %02X\n VLAN of captured packet must be: %02X",
                                     PRV_TGF_TUNNEL_START_UP2_CNS,
                                     PRV_TGF_TRILL_VLANID_CNS);

        /* check if captured packet has expected TRILL Ethertype and TRILL header format */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_2) >> 2,
                                     "\n Ethertype of captured packet must be: %04X\n TRILL M bit must be: 0x1"
                                     "\n TRILL hop count must be: 0x28\n TRILL eRbid must be: 0x6543\n TRILL iRbid must be: 0x4321",
                                     TGF_ETHERTYPE_22F3_TRILL_TAG_CNS);

        /* check if captured packet has expected passenger mac da */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_3) >> 3,
                                     "\n   Passenger MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2Part.daMac[0],
                                     prvTgfPacketL2Part.daMac[1],
                                     prvTgfPacketL2Part.daMac[2],
                                     prvTgfPacketL2Part.daMac[3],
                                     prvTgfPacketL2Part.daMac[4],
                                     prvTgfPacketL2Part.daMac[5]);

        /* check if captured packet has expected passenger native VLAN */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_4) >> 4,
                                     "\n   Passenger native VLAN of captured packet must be: %02X",
                                     PRV_TGF_ETHERNET_VLANID_CNS);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }
}

/**
* @internal prvTgfTunneStartlTrillKnownMcRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownMcRestore
(
    GT_VOID
)
{

    GT_STATUS                       rc = GT_OK;
    GT_U32                          lttIndexArray[1];
    GT_U32                          mllIndexArray[3];
    PRV_TGF_TUNNEL_START_ENTRY_UNT  tunnelEntry;

    /* AUTODOC: RESTORE CONFIGURATION: */
    lttIndexArray[0] = PRV_TGF_EVIDX_CNS;
    mllIndexArray[0] = 0;
    mllIndexArray[1] = 1;
    mllIndexArray[2] = 2;

    cpssOsMemSet((GT_VOID*) &tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: clear L2 MLL & LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 3);

    /* Restore L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: restore ePort 900 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT1_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo[MLL_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort 1000 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_EPORT2_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo[MLL_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort 900 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT1_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo[MLL_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* AUTODOC: restore ePort 1000 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT2_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo[MLL_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* AUTODOC: restore tunnel start entries [8,4] */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX1_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX2_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, tgfTrafficTableRxPcktTblClear);

     /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgFdbFlush);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ETHERNET_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* AUTODOC: restore VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnable(prvTgfRestoreCfg.vlanEgressFilteringState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* AUTODOC: restore tag state mode for egress ePort 900 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT1_CNS,
                                                prvTgfRestoreCfg.vlanEgressPortTagStateMode[MLL_ENTRY_INDEX_0_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: restore tag state mode for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT2_CNS,
                                                prvTgfRestoreCfg.vlanEgressPortTagStateMode[MLL_ENTRY_INDEX_1_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: restore port tag state for egress ePort 900 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT1_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState[MLL_ENTRY_INDEX_0_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* AUTODOC: restore port tag state for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT2_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState[MLL_ENTRY_INDEX_1_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

}


