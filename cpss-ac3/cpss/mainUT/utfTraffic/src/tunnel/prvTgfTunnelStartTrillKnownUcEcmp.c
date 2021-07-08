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
* @file prvTgfTunnelStartTrillKnownUcEcmp.c
*
* @brief Tunnel: TRILL Known Unicast Multipath (ECMP)
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
#include <common/tgfTrunkGen.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeL2EcmpGen.h>
#include <tunnel/prvTgfTunnelStartTrillKnownUcEcmp.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Ethernet VLAN Id */
#define PRV_TGF_ETHERNET_VLANID_CNS         5

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            6

/* Global ePort Representing an L2 ECMP Group */
#define PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS   1025

/* ePort that is represented by the primary ePort PRV_TGF_DESTINATION_PRIMERY_EPORT */
#define PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS   700

/* ePort that is represented by the primary ePort PRV_TGF_DESTINATION_PRIMERY_EPORT */
#define PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS   800

/* ePort that is represented by the primary ePort PRV_TGF_DESTINATION_PRIMERY_EPORT */
#define PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS   900

/* number of members in the ECMP group */
#define PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS    3

/* number of packets to send */
#define PRV_TGF_NUM_OF_PACKETS_TO_SEND_CNS          30

/* The index to the L2 ECMP LTT is <Target ePort>-<Base ePort> */
#define PRV_TGF_INDEX_BASE_EPORT_CNS        0

/* support all global ePorts in range 1024-4294966272 (bits [10:31]) */
#define ALL_GLOBAL_EPORT_MASK_CNS           0xFFFFFC00

/* member 0 of the ECMP group */
#define ECMP_ENTRY_INDEX_0_CNS              0

/* member 1 of the ECMP group */
#define ECMP_ENTRY_INDEX_1_CNS              1

/* member 2 of the ECMP group */
#define ECMP_ENTRY_INDEX_2_CNS              2

/* FDB entry destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS    1000

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           0

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT1_IDX_CNS       1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT2_IDX_CNS       2

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT3_IDX_CNS       3

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
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of transmitted packet */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_TRANSMITTED_PACKET_LEN_CNS,                          /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* LENGTH of expected packet (original packet tunneled over TRILL packet) */
#define PRV_TGF_EXPECTED_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS * 2 + TGF_TRILL_HEADER_SIZE_CNS + PRV_TGF_TRANSMITTED_PACKET_LEN_CNS)

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfTunnelStartEntryMacDa[PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS] = {
    {0x66, 0x55, 0x44, 0x33, 0x22, 0x11},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
    {0x99, 0x88, 0x77, 0x66, 0x55, 0x44}
};

/* Tunnel start entry index */
static GT_U32 PRV_TGF_TUNNEL_START_INDEX_CNS[PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS] = {8, 9, 10};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo[PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS];
    CPSS_INTERFACE_INFO_STC                     physicalInfo[PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS];
    GT_BOOL                                     vlanEgressFilteringState;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   vlanEgressPortTagStateMode;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT           vlanEgressPortTagState;
    PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_ENT     selectionMode;
    PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT           hashMode;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                ecmp;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                global;
} prvTgfRestoreCfg;

/******************************************************************************/
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfL2EcmpSeconderyEportsInit function
* @endinternal
*
* @brief   Initialize ECMP Secondary ePorts Settings
*
* @param[in] portNum                  - ePort number
* @param[in] physicalPortNum          - physical port number
* @param[in] confIndex                - configuration index
*                                       None
*/
static GT_VOID prvTgfL2EcmpSeconderyEportsInit
(
    IN  GT_PORT_NUM     portNum,
    IN  GT_PORT_NUM     physicalPortNum,
    IN  GT_U8           confIndex
)
{
    CPSS_INTERFACE_INFO_STC             physicalInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;
    GT_STATUS                           rc = GT_OK;

    /* save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       portNum,
                                                       &(prvTgfRestoreCfg.egressInfo[confIndex]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoGet %d", portNum);

    /* set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS[confIndex];
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       portNum,
                                                       &egressInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet %d", portNum);

    /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           portNum,
                                                           &(prvTgfRestoreCfg.physicalInfo[confIndex]));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet %d", portNum);

    /* set ePort mapping configuration */
    cpssOsMemSet(&physicalInfo,0,sizeof(physicalInfo));
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = physicalPortNum;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           portNum,
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet %d", portNum);

    /* set TRILL tunnel start entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.trillCfg.upMarkMode             = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.trillCfg.up                     = 0x3;
    tunnelEntry.trillCfg.tagEnable              = GT_TRUE;
    tunnelEntry.trillCfg.vlanId                 = PRV_TGF_TRILL_VLANID_CNS;
    cpssOsMemCpy(tunnelEntry.trillCfg.nextHopMacDa.arEther, prvTgfTunnelStartEntryMacDa[confIndex], sizeof(TGF_MAC_ADDR));
    tunnelEntry.trillCfg.trillHeader.version    = 0;
    tunnelEntry.trillCfg.trillHeader.mBit       = 0;
    tunnelEntry.trillCfg.trillHeader.opLength   = 0;
    tunnelEntry.trillCfg.trillHeader.hopCount   = 0x28;
    tunnelEntry.trillCfg.trillHeader.eRbid      = 0x6543;
    tunnelEntry.trillCfg.trillHeader.iRbid      = 0x4321;
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS[confIndex], CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet %d", portNum);
}

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpTrafficGeneratePerPort function
* @endinternal
*
* @brief   Generate traffic.
*
* @param[in] portNumIndex             - port number index in prvTgfPortsArray
* @param[in] testNum                  - In test 1: Each secondary ePort is mapped to a different physical port.
*                                      - In test 2: 2 secondary ePorts are mapped to the same physical port.
*                                       None
*/
static GT_VOID prvTgfTunneStartlTrillKnownUcEcmpTrafficGeneratePerPort
(
    IN  GT_U8     portNumIndex,
    IN  GT_U8     testNum
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter    = 0;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                  packetIter  = 0;
    GT_U32                  triggerBitmap = 0;
    TGF_VFD_INFO_STC        vfdArray[PRV_TGF_VFD_NUM_CNS];
    GT_U32                  vfdIndex = 0;/* index in vfdArray */
    GT_U32                  numVfdTransmit = 1;/* number of VFDs in vfdArrayTransmit */
    TGF_VFD_INFO_STC        vfdArrayTransmit[1];/* vfd Array -- used for increment the tested bytes */
    GT_U32                  total_goodPktsSent;
    GT_U32                  total_goodPktsRcv;
    GT_U32                  total_goodOctetsSent;
    GT_U32                  total_goodOctetsRcv;
    CPSS_PORT_MAC_COUNTER_SET_STC portCounters[PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS];

    /******** Check packet on port prvTgfPortsArray[PRV_TGF_RECEIVE_PORT1_IDX_CNS] ********/
    PRV_UTF_LOG1_MAC("\nCheck packet on port %d\n\n",prvTgfPortsArray[portNumIndex]);

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));
    cpssOsMemSet((GT_VOID*) vfdArrayTransmit, 0, sizeof(vfdArrayTransmit));

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

        /* modify the packet */
        vfdArrayTransmit->mode = TGF_VFD_MODE_INCREMENT_E;
        vfdArrayTransmit->modeExtraInfo = 0; /* for increment,decrement  it is the current step (0,1,2...) */
        vfdArrayTransmit->offset = 11; /* offset in the packet to override - prvTgfPacketL2Part.saMac[5]*/
        cpssOsMemSet(vfdArrayTransmit->patternPtr,0,sizeof(TGF_MAC_ADDR));
        vfdArrayTransmit->patternPtr[0] = (GT_U8)(0x55 + testNum); /* the pattern for the packet in the needed offset */
        vfdArrayTransmit->cycleCount = 1; /* single byte in patternPtr */

        /* send packet */
        rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              &prvTgfPacketInfo, prvTgfBurstCount * PRV_TGF_NUM_OF_PACKETS_TO_SEND_CNS, numVfdTransmit, vfdArrayTransmit,
                                              prvTgfDevNum,
                                              prvTgfPortsArray[portNumIndex],
                                              TGF_CAPTURE_MODE_MIRRORING_E, 10);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

        /* clear counters*/
        total_goodPktsSent = 0;
        total_goodPktsRcv = 0;
        total_goodOctetsSent = 0;
        total_goodOctetsRcv = 0;

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_FALSE, &portCounters[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* accomulate the read result */
        total_goodPktsSent    += portCounters[portIter].goodPktsSent.l[0];
        total_goodPktsRcv     += portCounters[portIter].goodPktsRcv.l[0];
        total_goodOctetsSent  += portCounters[portIter].goodOctetsSent.l[0];
        total_goodOctetsRcv   += portCounters[portIter].goodOctetsRcv.l[0];


        /* print retrieved values for port */
        PRV_UTF_LOG1_MAC("Port %d - Retrieved Counter values:\n", prvTgfPortsArray[portIter]);
        if (portCounters[portIter].goodPktsSent.l[0])
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", portCounters[portIter].goodPktsSent.l[0]);
        if (portCounters[portIter].goodPktsRcv.l[0])
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", portCounters[portIter].goodPktsRcv.l[0]);
        if (portCounters[portIter].goodOctetsSent.l[0])
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", portCounters[portIter].goodOctetsSent.l[0]);
        if (portCounters[portIter].goodOctetsRcv.l[0])
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", portCounters[portIter].goodOctetsRcv.l[0]);
        PRV_UTF_LOG0_MAC(" \n");

        if (testNum == 1)
        {
            /* check Rx counters */
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(portCounters[portIter].goodPktsSent.l[0], 0,
                                             "prvTgfReadPortCountersEth: %d, %d\n",
                                             total_goodPktsSent, 0);
        }
        else /* test 2 */
        {
            if (portIter == 1) /* no packets are sent to this physical port */
            {
                /* check Rx counters */
            UTF_VERIFY_EQUAL2_STRING_MAC(portCounters[portIter].goodPktsSent.l[0], 0,
                                             "prvTgfReadPortCountersEth: %d, %d\n",
                                             total_goodPktsSent, 0);
            }
        }
    }

    /* print total retrieved values */
    PRV_UTF_LOG0_MAC("TOTAL packets Sent & Recieved:\n");
    if (total_goodPktsSent)
        PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", total_goodPktsSent);
    if (total_goodPktsRcv)
        PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", total_goodPktsRcv);
    if (total_goodOctetsSent)
        PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", total_goodOctetsSent);
    if (total_goodOctetsRcv)
        PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", total_goodOctetsRcv);
    PRV_UTF_LOG0_MAC(" \n");

    /* check Rx counters */
    UTF_VERIFY_EQUAL2_STRING_MAC(total_goodPktsSent, PRV_TGF_NUM_OF_PACKETS_TO_SEND_CNS * 2,
                                 "prvTgfReadPortCountersEth: %d, %d\n",
                                 total_goodPktsSent, PRV_TGF_NUM_OF_PACKETS_TO_SEND_CNS * 2);


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[portNumIndex];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check TRILL da mac */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 0;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, &prvTgfTunnelStartEntryMacDa[portNumIndex - 1], sizeof(TGF_MAC_ADDR));
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
                                     prvTgfTunnelStartEntryMacDa[portNumIndex-1][0],
                                     prvTgfTunnelStartEntryMacDa[portNumIndex-1][1],
                                     prvTgfTunnelStartEntryMacDa[portNumIndex-1][2],
                                     prvTgfTunnelStartEntryMacDa[portNumIndex-1][3],
                                     prvTgfTunnelStartEntryMacDa[portNumIndex-1][4],
                                     prvTgfTunnelStartEntryMacDa[portNumIndex-1][5]);

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
                                     "\n   TS VLAN (not of passenger) of captured packet must be: %02X",
                                     PRV_TGF_TRILL_VLANID_CNS);

        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   stateMode;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                ecmp;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                global;
    PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC           ecmpLttEntry;
    PRV_TGF_BRG_L2_ECMP_ENTRY_STC               ecmpEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

     /* create L2 ECMP LTT entries */
    cpssOsMemSet(&ecmpLttEntry, 0, sizeof(PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC));

    ecmpLttEntry.ecmpNumOfPaths = PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS; /* the number of members in the ECMP group; value of 3 means that there is 3 members */
    ecmpLttEntry.ecmpStartIndex = 0; /* the start index of the ECMP block; Each ECMP group is stored as a set of consecutive entries in this table, and the L2 ECMP LTT<L2 ECMP Start Index>
                                        indicates the first entry of an ECMP group */
    ecmpLttEntry.ecmpEnable = GT_TRUE; /* whether the packet is load balanced over an ECMP */

    /* Map the global ePort PRV_TGF_DESTINATION_PRIMERY_EPORT to a pointer of linked-lidt of L2ECMP */
    /* AUTODOC: configure L2 ECMP LTT entry info: */
    /* AUTODOC:   ecmpNumOfPaths=3, ecmpStartIndex=0 */
    rc = prvTgfBrgL2EcmpLttTableSet(prvTgfDevNum, PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS, &ecmpLttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpLttTableSet");

    /* Defines the first ePort number in the L2 ECMP ePort number range.
       The index to the L2 ECMP LTT is <Target ePort>-<Base ePort> */
    /* AUTODOC: define first ePort 0 in the L2 ECMP ePort number range */
    rc = prvTgfBrgL2EcmpIndexBaseEportSet(prvTgfDevNum, PRV_TGF_INDEX_BASE_EPORT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpIndexBaseEportSet");

    /* AUTODOC: enable trill on egress targetEport=700 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS , GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");

    /* AUTODOC: enable trill on egress targetEport=800 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS , GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");

    /* AUTODOC: enable trill on egress targetEport=900 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS , GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");

     /* create L2 ECMP entries */
    cpssOsMemSet(&ecmpEntry, 0, sizeof(PRV_TGF_BRG_L2_ECMP_ENTRY_STC));

    /* Set L2 ECMP entry 0 info */
    ecmpEntry.targetEport = PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS;
    ecmpEntry.targetHwDevice = prvTgfDevNum;

    /* AUTODOC: set L2 ECMP entry 0 with targetEport=700 */
    rc = prvTgfBrgL2EcmpTableSet(prvTgfDevNum, ECMP_ENTRY_INDEX_0_CNS, &ecmpEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpTableSet entry 0");

     /* Set L2 ECMP entry 1 info */
    ecmpEntry.targetEport = PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS;
    ecmpEntry.targetHwDevice = prvTgfDevNum;

    /* AUTODOC: set L2 ECMP entry 1 with targetEport=800 */
    rc = prvTgfBrgL2EcmpTableSet(prvTgfDevNum, ECMP_ENTRY_INDEX_1_CNS, &ecmpEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpTableSet entry 1");

     /* Set L2 ECMP entry 2 info */
    ecmpEntry.targetEport = PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS;
    ecmpEntry.targetHwDevice = prvTgfDevNum;

    /* AUTODOC: set L2 ECMP entry 2 with targetEport=900 */
    rc = prvTgfBrgL2EcmpTableSet(prvTgfDevNum, ECMP_ENTRY_INDEX_2_CNS, &ecmpEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpTableSet entry 2");

    /* save mode used to calculate the index of secondary ePort */
    rc = prvTgfBrgL2EcmpMemberSelectionModeGet(prvTgfDevNum, &(prvTgfRestoreCfg.selectionMode));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpMemberSelectionModeGet");

    /* Set mode used to calculate the Index of the secondary ePort;
       Use only the 6 least significant bits in the member selection ((Hash[5:0] * #members)/64) */
    /* AUTODOC: set use 6_LSB L2 calculation mode for Index of secondary ePort */
    rc = prvTgfBrgL2EcmpMemberSelectionModeSet(prvTgfDevNum, PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_6_LSB_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpMemberSelectionModeSet");

    /* AUTODOC: create Ethernet VLAN 5 with untagged port [0] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_ETHERNET_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 1);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* AUTODOC: create TRILL VLAN 6 with untagged ports [1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray + 1,
                                           NULL, NULL, 3);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* save global ePort configuration */
    rc = prvTgfCfgGlobalEportGet(prvTgfDevNum, &prvTgfRestoreCfg.global , &prvTgfRestoreCfg.ecmp);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCfgGlobalEportGet);

    /* set PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS as global ePort: define global ePort range as 1024-2048 */
    ecmp.enable = GT_TRUE;
    ecmp.pattern = BIT_10; /* support all global ePorts in range 1024-2048 */
    ecmp.mask = ALL_GLOBAL_EPORT_MASK_CNS; /* support all global ePorts in range 1024-4294966272 (bits [10:31]) */

    global = ecmp;

    /* AUTODOC: configure Global ePorts ranges as 1024-2048 */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &global , &ecmp);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCfgGlobalEportSet);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, ePort 1025 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_ETHERNET_VLANID_CNS, prvTgfDevNum,
            PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefFdbMacEntryOnPortSet);

    /* set ePort PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS attributes configuration */

    /* AUTODOC: config ePort 700 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    /* AUTODOC: map port 1 to ePort 700 */

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, up=0x3, upMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=66:55:44:33:22:11 */
    prvTgfL2EcmpSeconderyEportsInit(PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS, prvTgfPortsArray[1], 0);

    /* set ePort PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS attributes configuration */

    /* AUTODOC: config ePort 800 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=9 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    /* AUTODOC: map port 2 to ePort 800 */

    /* AUTODOC: add Tunnel Start entry 9 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, up=0x3, upMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=01:02:03:04:05:06 */
    prvTgfL2EcmpSeconderyEportsInit(PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS, prvTgfPortsArray[2], 1);

    /* set ePort PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS attributes configuration */

    /* AUTODOC: config ePort 900 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=9 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    /* AUTODOC: map port 3 to ePort 900 */

    /* AUTODOC: add Tunnel Start entry 10 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, up=0x3, upMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=99:88:77:66:55:44 */
    prvTgfL2EcmpSeconderyEportsInit(PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS, prvTgfPortsArray[3], 2);

    /* save the general hashing mode */
    rc = prvTgfTrunkHashGlobalModeGet(prvTgfDevNum, &(prvTgfRestoreCfg.hashMode));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTrunkHashGlobalModeGet);

    /* AUTODOC: set general hashing mode - hash is based on the packets data */
    rc = prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTrunkHashGlobalModeSet);

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
                                                PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS,
                                                &stateMode);
    prvTgfRestoreCfg.vlanEgressPortTagStateMode = stateMode;
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeGet);

    /* AUTODOC: set EPORT tag state mode for egress ePort 1025 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS,
                                                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* save VLAN egress port tag state */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS,
                                            &(prvTgfRestoreCfg.vlanEgressPortTagState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateGet);

    /* AUTODOC: set TAG0_CMD port tag state for egress ePort 1025 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS,
                                            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);
}

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpConfig2Set function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpConfig2Set
(
    GT_VOID
)
{
        /* AUTODOC: SETUP CONFIGURATION: */
    /* 2 secondary ePorts are mapped to the same physical port */

    /* set ePort PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS attributes configuration */

        /* AUTODOC: config ePort 700 attributes: */
        /* AUTODOC:   Tunnel Start, tunnelStartPtr=8 */
        /* AUTODOC:   tsPassenger=ETHERNET */
        /* AUTODOC: map port 2 to ePort 700 */

        /* AUTODOC: add Tunnel Start entry 8 with: */
        /* AUTODOC:   tunnelType=TRILL */
        /* AUTODOC:   vlanId=6, up=0x3, upMarkMode=MARK_FROM_ENTRY */
        /* AUTODOC:   hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
        /* AUTODOC:   DA=66:55:44:33:22:11 */
    prvTgfL2EcmpSeconderyEportsInit(PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS, prvTgfPortsArray[2], 0);

    /* set ePort PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS attributes configuration */

    /* AUTODOC: config ePort 800 attributes: */
    /* AUTODOC:   Tunnel Start, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    /* AUTODOC: map port 2 to ePort 800 */

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=TRILL */
    /* AUTODOC:   vlanId=6, up=0x3, upMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   DA=66:55:44:33:22:11 */
    prvTgfL2EcmpSeconderyEportsInit(PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS, prvTgfPortsArray[2], 1);

}
/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpTrafficGenerate
(
    GT_VOID
)
{
        /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("\nTest 1: Each secondary ePort is mapped to a different physical port\n");

    /* AUTODOC: send 30 Ethernet packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:11:22:33:44:[56..74], VID=5 */
    /* AUTODOC:   EtherType=0x0x600 */

    /* AUTODOC: verify to get 2 tunneled packets on port 1 with: */
    /* AUTODOC:   DA=66:55:44:33:22:11, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:[56..57] */
    /* AUTODOC:   EtherType=0x0x600 */
    prvTgfTunneStartlTrillKnownUcEcmpTrafficGeneratePerPort(PRV_TGF_RECEIVE_PORT1_IDX_CNS, 1);

    /* AUTODOC: send 30 Ethernet packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:11:22:33:44:[56..74], VID=5 */
    /* AUTODOC:   EtherType=0x0x600 */

    /* AUTODOC: verify to get 19 tunneled packets on port 2 with: */
    /* AUTODOC:   DA=01:02:03:04:05:06, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:11:22:33:44:[58..6b] */
    /* AUTODOC:   EtherType=0x0x600 */
    prvTgfTunneStartlTrillKnownUcEcmpTrafficGeneratePerPort(PRV_TGF_RECEIVE_PORT2_IDX_CNS, 1);

    /* AUTODOC: send 30 Ethernet packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:11:22:33:44:[56..74], VID=5 */
    /* AUTODOC:   EtherType=0x0x600 */

    /* AUTODOC: verify to get 9 tunneled packets on port 3 with: */
    /* AUTODOC:   DA=99:88:77:66:55:44, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:11:22:33:44:[69..73] */
    /* AUTODOC:   EtherType=0x0x600 */
    prvTgfTunneStartlTrillKnownUcEcmpTrafficGeneratePerPort(PRV_TGF_RECEIVE_PORT3_IDX_CNS, 1);

}

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpTraffic2Generate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpTraffic2Generate
(
    GT_VOID
)
{
        /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("\nTest 2: 2 secondary ePorts are mapped to the same physical port\n");

    /* AUTODOC: send 30 Ethernet packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:11:22:33:44:[57..75], VID=5 */
    /* AUTODOC:   EtherType=0x0x600 */

    /* AUTODOC: verify to get 1 tunneled packet on port 2 with: */
    /* AUTODOC:   DA=66:55:44:33:22:11, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:11:22:33:44:57 */
    /* AUTODOC:   EtherType=0x0x600 */

    /* AUTODOC: verify to get 19 tunneled packets on port 2 with: */
    /* AUTODOC:   DA=01:02:03:04:05:06, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:11:22:33:44:[58..6b] */
    /* AUTODOC:   EtherType=0x0x600 */
    prvTgfTunneStartlTrillKnownUcEcmpTrafficGeneratePerPort(PRV_TGF_RECEIVE_PORT2_IDX_CNS, 2);

    /* AUTODOC: send 30 Ethernet packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:11:22:33:44:[57..75], VID=5 */
    /* AUTODOC:   EtherType=0x0x600 */

    /* AUTODOC: verify to get 10 tunneled packets on port 3 with: */
    /* AUTODOC:   DA=99:88:77:66:55:44, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   EtherType=0x22F3 hopCount=0x28, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:11:22:33:44:[69, 6c..74] */
    /* AUTODOC:   EtherType=0x0x600 */
    prvTgfTunneStartlTrillKnownUcEcmpTrafficGeneratePerPort(PRV_TGF_RECEIVE_PORT3_IDX_CNS, 2);

}

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;
    GT_U32                              lttIndexArray[1];
    GT_U32                              ecmpIndexArray[3];

    /* AUTODOC: RESTORE CONFIGURATION: */
    cpssOsMemSet((GT_VOID*) &tunnelEntry,      0, sizeof(tunnelEntry));

    /* AUTODOC: disable trill on egress targetEport=700 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS , GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");

    /* AUTODOC: disable trill on egress targetEport=800 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS , GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");

    /* AUTODOC: disable trill on egress targetEport=900 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS , GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortTrillEnableSet");

    /* AUTODOC: disable ePort 700 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo[ECMP_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort 800 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo[ECMP_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort 900 attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo[ECMP_ENTRY_INDEX_2_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: restore ePort 700 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo[ECMP_ENTRY_INDEX_0_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* AUTODOC: restore ePort 800 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_SECONDARY2_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo[ECMP_ENTRY_INDEX_1_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* AUTODOC: restore ePort 900 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_SECONDARY3_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo[ECMP_ENTRY_INDEX_2_CNS]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* clear TRILL tunnel start entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: clear TRILL tunnel start entries [8,9,10] */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS[ECMP_ENTRY_INDEX_0_CNS], CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* clear TRILL tunnel start entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS[ECMP_ENTRY_INDEX_1_CNS], CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* clear TRILL tunnel start entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS[ECMP_ENTRY_INDEX_2_CNS], CPSS_TUNNEL_TRILL_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    lttIndexArray[0]  = PRV_TGF_DESTINATION_PRIMERY_EPORT_CNS;
    ecmpIndexArray[0] = ECMP_ENTRY_INDEX_0_CNS;
    ecmpIndexArray[1] = ECMP_ENTRY_INDEX_1_CNS;
    ecmpIndexArray[2] = ECMP_ENTRY_INDEX_2_CNS;

    /* AUTODOC: clear MLL entries */
    prvTgfBrgL2EcmpAndLttEntriesClear(lttIndexArray, 1, ecmpIndexArray, 3);

    /* AUTODOC: restore mode used to calculate index of the secondary ePort */
    rc = prvTgfBrgL2EcmpMemberSelectionModeSet(prvTgfDevNum, prvTgfRestoreCfg.selectionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpMemberSelectionModeSet");

    /* AUTODOC: restore global ePort configuration */
    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &prvTgfRestoreCfg.global, &prvTgfRestoreCfg.ecmp);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCfgGlobalEportSet);

    /* AUTODOC: restore the general hashing mode */
    rc = prvTgfTrunkHashGlobalModeSet(prvTgfRestoreCfg.hashMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTrunkHashGlobalModeSet);

    /* AUTODOC: restore def general hashing mode - hash is based on the packets data */
    rc = prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTrunkHashGlobalModeSet);

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

    /* AUTODOC:  restore port tag state for egress ePort 1000 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_DESTINATION_EPORT_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);
}


