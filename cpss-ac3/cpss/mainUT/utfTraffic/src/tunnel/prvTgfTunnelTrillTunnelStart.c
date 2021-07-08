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
* @file prvTgfTunnelTrillTunnelStart.c
*
* @brief Tunnel: TRILL tunnel start
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
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTrillTunnelStart.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Ethernet VLAN Id */
#define PRV_TGF_ETHERNET_VLANID_CNS         5

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            6

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
    GT_BOOL                                     vlanEgressFilteringState;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   vlanEgressPortTagStateMode;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT           vlanEgressPortTagState;
} prvTgfRestoreCfg;

/******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_INTERFACE_INFO_STC             physicalInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   stateMode;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: egress ePort 1000 <Enable TRILL Interface> = 1*/
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum,PRV_TGF_DESTINATION_EPORT_CNS,GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_ETHERNET_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* AUTODOC: create TRILL VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, ePort 1000 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_ETHERNET_VLANID_CNS, prvTgfDevNum,
            PRV_TGF_DESTINATION_EPORT_CNS, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefFdbMacEntryOnPortSet);

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
    tunnelEntry.trillCfg.upMarkMode             = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.trillCfg.up                     = 0x3;
    tunnelEntry.trillCfg.tagEnable              = GT_TRUE;
    tunnelEntry.trillCfg.vlanId                 = PRV_TGF_TRILL_VLANID_CNS;
    cpssOsMemCpy(tunnelEntry.trillCfg.nextHopMacDa.arEther, prvTgfTunnelStartEntryMacDa, sizeof(TGF_MAC_ADDR));
    tunnelEntry.trillCfg.trillHeader.version    = 0;
    tunnelEntry.trillCfg.trillHeader.mBit       = 0;
    tunnelEntry.trillCfg.trillHeader.opLength   = 0;
    tunnelEntry.trillCfg.trillHeader.hopCount   = 0x28;
    tunnelEntry.trillCfg.trillHeader.eRbid      = 0x6543;
    tunnelEntry.trillCfg.trillHeader.iRbid      = 0x4321;

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, up=0x3, upMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=66:55:44:33:22:11 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* disable VLAN egress filtering to allow destination port to be member
       in TRILL VLAN and not be member of the native ETHERNET VLAN */

    /* save VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnableGet(prvTgfDevNum,&(prvTgfRestoreCfg.vlanEgressFilteringState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnableGet);

    /* AUTODOC: disable VLAN egress filtering */
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
* @internal prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter    = 0;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8                   packetsCountRx;
    GT_U8                   packetsCountTx;
    GT_U32                  packetSize;
    GT_U32                  packetIter  = 0;
    GT_U32                  triggerBitmap = 0;
    TGF_VFD_INFO_STC        vfdArray[PRV_TGF_VFD_NUM_CNS];
    GT_U32                  vfdIndex = 0;/* index in vfdArray */

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
    /* AUTODOC:   DA=66:55:44:33:22:11, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   Trill EtherType=0x22F3 */
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
            packetSize     = PRV_TGF_EXPECTED_PACKET_LEN_CNS;
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
}

/**
* @internal prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

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

    /* AUTODOC: egress ePort 1000 <Enable TRILL Interface> = 0*/
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum,PRV_TGF_DESTINATION_EPORT_CNS,GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

}


