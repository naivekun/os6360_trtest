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
* @file prvTgfTunnelStartTrillUnknownUcFlooding.c
*
* @brief Tunnel: TRILL tunnel start - Unknown UC Flooding
*
* @version   9
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
#include <common/tgfTunnelGen.h>
#include <common/tgfL2MllGen.h>
#include <l2mll/prvTgfL2MllUtils.h>
#include <tunnel/prvTgfTunnelStartTrillUnknownUcFlooding.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Ethernet VLAN Id */
#define PRV_TGF_ETHERNET_VLANID_CNS         5

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            6

/* eVidx to a pointer of linked-list of L2MLL */
#define PRV_TGF_FLOOD_EVIDX_CNS            4096

/* MLL entry index */
#define PRV_TGF_MLL_INDEX_CNS               2

/* FDB entry destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS    1000

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           0

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS        3

/* max number of VFD to use */
#define PRV_TGF_VFD_NUM_CNS                 10

/* offset of the passenger in the packet*/
#define PASSENGER_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
                                 TGF_ETHERTYPE_SIZE_CNS + TGF_TRILL_HEADER_SIZE_CNS)

/* number of ports in PRV_TGF_ETHERNET_VLANID_CNS */
static GT_U8  prvTgfPortsInVlanNum = 2;

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
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part of packet */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_ETHERNET_VLANID_CNS                   /* pri, cfi, VlanId */
};

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
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of transmitted packet */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

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
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
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
* @internal prvTgfTunnelStartTrillUnknownUcFloodingConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelStartTrillUnknownUcFloodingConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT              tunnelEntry;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   stateMode;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                lttEntry;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: define "Flood" eVIDX for native VLAN with value > 4K (indicating L2MLL duplication list) */
    prvTgfL2MllVlanInit(PRV_TGF_ETHERNET_VLANID_CNS, PRV_TGF_FLOOD_EVIDX_CNS);

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: enable eVidx for all lookup */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d", GT_TRUE);

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    /* AUTODOC: map the eVIDX to a pointer of linked-list of L2MLL */
    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_FLOOD_EVIDX_CNS, &lttEntry); /* according to LTT table mode maybe evidx - 4k*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    /* MLL entry :Trg Vidx = 0xFFF (All native Ethernet VLAN port members */

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
    mllPairEntry.nextPointer=0;
    mllPairEntry.entrySelector=PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VID_E; /* vidx = 0xFFF: All native Ethernet VLAN port members */
    mllPairEntry.firstMllNode.egressInterface.vlanId = PRV_TGF_ETHERNET_VLANID_CNS;

    /* MLL entry : Trg ePort */
    mllPairEntry.secondMllNode.last = GT_TRUE;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;


    /* AUTODOC: set L2 MLL entry 0: */
    /* AUTODOC:  vlanId=5, ePort=1000 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* AUTODOC: create TRILL VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

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

    /* AUTODOC: enable trill on egress targetEport=1000 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT_CNS , GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");


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
    /* AUTODOC:   hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=01:80:C2:00:00:40 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* disable VLAN egress filtering to allow destination port to be member
       in TRILL VLAN and not be member of the native ETHERNET VLAN */

    /* save VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnableGet(prvTgfDevNum,&(prvTgfRestoreCfg.vlanEgressFilteringState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnableGet);

    /* AUTODDOC: disable VLAN egress filtering */
    rc = prvTgfBrgVlanEgressFilteringEnable(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* save VLAN egress port tag state mode */
    rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT_CNS,
                                                &stateMode);
    prvTgfRestoreCfg.vlanEgressPortTagStateMode = stateMode;
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeGet);

    /* AUTODOC: set EPORT tag state mode for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT_CNS,
                                                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* save VLAN egress port tag state */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT_CNS,
                                            &(prvTgfRestoreCfg.vlanEgressPortTagState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateGet);

    /* AUTODOC: set TAG0_CMD port tag state for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT_CNS,
                                            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);
}

/**
* @internal prvTgfTunnelStartTrillUnknownUcFloodingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelStartTrillUnknownUcFloodingTrafficGenerate
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

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /******** Check packet on port prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] ********/
    PRV_UTF_LOG1_MAC("\nCheck packet on port %d\n",prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 1. Reset Counters
     */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
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
    /* AUTODOC:   DA=01:80:C2:00:00:40, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   Trill EtherType=0x22F3 */
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
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;
            break;
        case 2:
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
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

        /* check if captured packet has expected passenger mac da */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_2) >> 2,
                                     "\n   Passenger MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2Part.daMac[0],
                                     prvTgfPacketL2Part.daMac[1],
                                     prvTgfPacketL2Part.daMac[2],
                                     prvTgfPacketL2Part.daMac[3],
                                     prvTgfPacketL2Part.daMac[4],
                                     prvTgfPacketL2Part.daMac[5]);

        /* check if captured packet has expected passenger native VLAN */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_3) >> 3,
                                     "\n   Passenger native VLAN of captured packet must be: %02X",
                                     PRV_TGF_ETHERNET_VLANID_CNS);

        /* check if captured packet has expected passenger native VLAN */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_4) >> 4,
                                     "\n   TS VLAN (not of passenger) of captured packet must be: %02X",
                                     PRV_TGF_TRILL_VLANID_CNS);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }

    /******** Check packet on port prvTgfPortsArray[1] ********/
    PRV_UTF_LOG1_MAC("\nCheck packet on port %d\n",prvTgfPortsArray[1]);
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
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;
            break;
        case 2:
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
            break;
        case 3: /* PRV_TGF_RECEIVE_PORT_IDX_CNS */
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
    portInterface.devPort.portNum = prvTgfPortsArray[1];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check Ethernet mac da */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 0;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check Ethernet mac sa */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 6;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    /* check Ethernet payload */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[vfdIndex].cycleCount = 6;
    vfdArray[vfdIndex].patternPtr[0] = prvTgfPacketPayloadDataArr[0];
    vfdArray[vfdIndex].patternPtr[1] = prvTgfPacketPayloadDataArr[1];
    vfdArray[vfdIndex].patternPtr[2] = prvTgfPacketPayloadDataArr[2];
    vfdArray[vfdIndex].patternPtr[3] = prvTgfPacketPayloadDataArr[3];
    vfdArray[vfdIndex].patternPtr[4] = prvTgfPacketPayloadDataArr[4];
    vfdArray[vfdIndex].patternPtr[5] = prvTgfPacketPayloadDataArr[5];
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
                                     prvTgfPacketL2Part.daMac[0],
                                     prvTgfPacketL2Part.daMac[1],
                                     prvTgfPacketL2Part.daMac[2],
                                     prvTgfPacketL2Part.daMac[3],
                                     prvTgfPacketL2Part.daMac[4],
                                     prvTgfPacketL2Part.daMac[5]);

        /* check if captured packet has expected Ethernet mac da*/
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_1) >> 1,
                                     "\n   TRILL MAC SA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2Part.saMac[0],
                                     prvTgfPacketL2Part.saMac[1],
                                     prvTgfPacketL2Part.saMac[2],
                                     prvTgfPacketL2Part.saMac[3],
                                     prvTgfPacketL2Part.saMac[4],
                                     prvTgfPacketL2Part.saMac[5]);

        /* check if captured packet has expected Ethernet payload */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (triggerBitmap & BIT_2) >> 2,
                                     "\n   Payload of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketPayloadDataArr[0],
                                     prvTgfPacketPayloadDataArr[1],
                                     prvTgfPacketPayloadDataArr[2],
                                     prvTgfPacketPayloadDataArr[3],
                                     prvTgfPacketPayloadDataArr[4],
                                     prvTgfPacketPayloadDataArr[5]);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }
}

/**
* @internal prvTgfTunnelStartTrillUnknownUcFloodingConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelStartTrillUnknownUcFloodingConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[3];

    lttIndexArray[0] = PRV_TGF_FLOOD_EVIDX_CNS;
    mllIndexArray[0] = 0;
    mllIndexArray[1] = 1;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: clear L2 MLL & LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 2);

    /* AUTODOC: restore L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: disable trill on egress targetEport=1000 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_EPORT_CNS , GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");

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

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, tgfTrafficTableRxPcktTblClear);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ETHERNET_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* AUTODOC: restore VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnable(prvTgfRestoreCfg.vlanEgressFilteringState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* AUTODOC: restore tag state mode for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_EPORT_CNS,
                                                prvTgfRestoreCfg.vlanEgressPortTagStateMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: restore port tag state for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

}


