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
* @file prvTgfCscdCentralizedChassisRemotePhysicalPortMap.c
*
* @brief Mapping of a target physical port to a remote physical port that
* resides over a DSA-tagged interface
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <cscd/prvTgfCscd.h>

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* egress port number to receive traffic from */
#define PRV_TGF_TARGET_PORT_IDX_CNS     2

/* egress eport to receive traffic from */
#define PRV_TGF_TARGET_EPORT_CNS        1000

#define PRV_TGF_NUMBER_OF_BYTES_BEFORE_DSA_TAG_CNS  12
#define PRV_TGF_WORD_0_OF_EDSA_TAG_CNS  PRV_TGF_NUMBER_OF_BYTES_BEFORE_DSA_TAG_CNS
#define PRV_TGF_WORD_1_OF_EDSA_TAG_CNS  PRV_TGF_NUMBER_OF_BYTES_BEFORE_DSA_TAG_CNS + 4
#define PRV_TGF_WORD_2_OF_EDSA_TAG_CNS  PRV_TGF_NUMBER_OF_BYTES_BEFORE_DSA_TAG_CNS + 8
#define PRV_TGF_WORD_3_OF_EDSA_TAG_CNS  PRV_TGF_NUMBER_OF_BYTES_BEFORE_DSA_TAG_CNS + 12

/* remote physical port number in the outgoing DSA tag */
#define PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS     100

/* remote physical device id in the outgoing DSA tag */
#define PRV_TGF_REMOTE_PHYSICAL_DEV_IDX_CNS     7

/* parameters that are needed to be restored */
static struct
{
    CPSS_INTERFACE_INFO_STC             physicalInfo;
    GT_BOOL                             portMapEnable;
    GT_HW_DEV_NUM                       remotePhysicalHwDevNum;
    GT_PHYSICAL_PORT_NUM                remotePhysicalPortNum;
    GT_U32                              portsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/*********************** Test configuration section ***************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
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
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         1. Set VLAN and FDB entry
*         2. Set E2PHY entry
*         3. Enable and Set remote physical port mapping
*/
GT_VOID prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    CPSS_INTERFACE_INFO_STC     physicalInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE, PRV_TGF_TARGET_PORT_IDX_CNS, -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }


    /* AUTODOC: create VLAN 5 with untagged ports 0,1,2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, NULL, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* add a MAC entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = PRV_TGF_TARGET_EPORT_CNS;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_FALSE;
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

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: %d", prvTgfDevNum);

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet(prvTgfDevNum,
                                                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                                    &(prvTgfRestoreCfg.portMapEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                                    GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet(prvTgfDevNum,
                                                              prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                              &(prvTgfRestoreCfg.remotePhysicalHwDevNum),
                                                              &(prvTgfRestoreCfg.remotePhysicalPortNum));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                              prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                              PRV_TGF_REMOTE_PHYSICAL_DEV_IDX_CNS,
                                                              PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d", prvTgfDevNum);

    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                    GT_TRUE,
                                                    CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfCscdCentralizedChassisRemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
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
    GT_U8                           pattern;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup target portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];

    /* enable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send L24 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify traffic */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

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

            case PRV_TGF_TARGET_PORT_IDX_CNS:
                /* packetSize is changed:
                   added: 16 bytes DSA tag, 4 bytes CRC
                   removed: 4 bytes VLAN tag */
                expectedPacketSize = (packetSize + 16 + TGF_CRC_LEN_CNS - 4);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;

            default:
                /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* check word 1, bits 24-31, of the DSA tag:
       bit 31 (extend) should be 1 - indicates that the DSA tag is 16 bytes long (eDSA)
       bit 28 (egress filter registered) should be 1
       All other bits should be 0 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = PRV_TGF_WORD_1_OF_EDSA_TAG_CNS;
    vfdArray[0].cycleCount = 1;
    pattern = 0x90 | ((3&(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] >> 5)) << 5);/*30:29 Src ePort[6:5]*/
    cpssOsMemCpy(vfdArray[0].patternPtr, &pattern, sizeof(GT_U8));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = ((rc == GT_NO_MORE) ? GT_OK : rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
                                 "\n   Word 1 byte 0 of captured packet must be 0x%x",pattern);

    /* check word 2, bits 24-31, of the DSA tag:
       bit 31 (extend) should be 1
       bit 30 (reserved) should be 0 - for SIP6 - skipFdbSaLookup == 1
       bit 29 (Is Trg Phy Port Valid) should be 1
       bit 28 (Trg Phy Port [7]) should be bit 7 of PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS
       bits 25-27 (Src Id [11:8]) should be 0
     */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = PRV_TGF_WORD_2_OF_EDSA_TAG_CNS;
    vfdArray[0].cycleCount = 1;
    pattern = ((1 << 7) | (1 << 5) | (((PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS & BIT_7) >> 7) << 4));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        pattern |= (1 << 6); /*skipFdbSaLookup == 1*/
    }
    cpssOsMemCpy(vfdArray[0].patternPtr, &pattern, sizeof(GT_U8));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = ((rc == GT_NO_MORE) ? GT_OK : rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
                                 "\n   Word 2 byte 0 of captured packet must be 0x%02X", pattern);

    /* check word 1, bits 8-15, of the DSA tag:
       bits 13-15 ({TC, DP} / Global QoS Profile) should be 0
       bit 12 (use eVIDX) should be 0
       bits 8-11 (Trg Phy Port [6:3]) should be bits 3-6 of PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS
     */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = PRV_TGF_WORD_1_OF_EDSA_TAG_CNS + 2;
    vfdArray[0].cycleCount = 1;
    pattern = ((PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS & 0x78) >> 3);
    cpssOsMemCpy(vfdArray[0].patternPtr, &pattern, sizeof(GT_U8));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = ((rc == GT_NO_MORE) ? GT_OK : rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
                                 "\n   Word 1 byte 2 of captured packet must be 0x%02X", pattern);

    /* check word 1, bits 0-7, of the DSA tag:
       bits 5-7 (Trg Phy Port [2:0]) should be bits 0-2 of PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS
       bits 0-4 (Trg Phy Dev [4:0]) should be bits 0-4 of PRV_TGF_REMOTE_PHYSICAL_DEV_IDX_CNS
     */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = PRV_TGF_WORD_1_OF_EDSA_TAG_CNS + 3;
    vfdArray[0].cycleCount = 1;
    pattern = (((PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS & 0x7) << 5) |
               (PRV_TGF_REMOTE_PHYSICAL_DEV_IDX_CNS & 0x1F));
    cpssOsMemCpy(vfdArray[0].patternPtr, &pattern, sizeof(GT_U8));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = ((rc == GT_NO_MORE) ? GT_OK : rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
                                 "\n   Word 1 byte 3 of captured packet must be 0x%02X", pattern);

    /* check word 3, bits 16-23, of the DSA tag:
       should be bits 9-16 of the remote physical port
     */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = PRV_TGF_WORD_3_OF_EDSA_TAG_CNS + 1;
    vfdArray[0].cycleCount = 1;
    pattern = ((PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS >> 9) & 0xFF);
    cpssOsMemCpy(vfdArray[0].patternPtr, &pattern, sizeof(GT_U8));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = ((rc == GT_NO_MORE) ? GT_OK : rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
                                 "\n   Word 3 byte 1 of captured packet must be 0x%02X", pattern);

    /* check word 3, bits 8-15, of the DSA tag:
       should be bits 1-8 of the remote physical port
     */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = PRV_TGF_WORD_3_OF_EDSA_TAG_CNS + 2;
    vfdArray[0].cycleCount = 1;
    pattern = ((PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS >> 1) & 0xFF);
    cpssOsMemCpy(vfdArray[0].patternPtr, &pattern, sizeof(GT_U8));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = ((rc == GT_NO_MORE) ? GT_OK : rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
                                 "\n   Word 3 byte 2 of captured packet must be 0x%02X", pattern);

    /* check word 3, bits 0-7, of the DSA tag:
       bit 7 should be bit 0 of the remote physical port
       bits 0-6 (Trg Phy Dev [11:5]) should be bits 5-11 of PRV_TGF_REMOTE_PHYSICAL_DEV_IDX_CNS
     */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = PRV_TGF_WORD_3_OF_EDSA_TAG_CNS + 3;
    vfdArray[0].cycleCount = 1;
    pattern = (((PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS & BIT_0) << 7) | ((PRV_TGF_REMOTE_PHYSICAL_DEV_IDX_CNS >> 5) & 0x7F));
    cpssOsMemCpy(vfdArray[0].patternPtr, &pattern, sizeof(GT_U8));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = ((rc == GT_NO_MORE) ? GT_OK : rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
                                 "\n   Word 3 byte 3 of captured packet must be 0x%02X", pattern);

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                 rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                    GT_FALSE,
                                                    CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                                    prvTgfRestoreCfg.portMapEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                              prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                              prvTgfRestoreCfg.remotePhysicalHwDevNum,
                                                              prvTgfRestoreCfg.remotePhysicalPortNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d", prvTgfDevNum);

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);

    /* flush the FDB table */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* delete VLAN 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfRestoreCfg.portsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));
}

