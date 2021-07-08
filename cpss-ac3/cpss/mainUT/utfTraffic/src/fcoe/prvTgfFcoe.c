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
* @file prvTgfFcoe.c
*
* @brief FCOE tests
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#endif

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>

#include <fcoe/prvTgfFcoe.h>




/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to  port 8*/
#define PRV_TGF_SEND_PORT_IDX_CNS           1

/* port number to receive traffic from port 18 */
#define PRV_TGF_GET_PORT_IDX_CNS            2

/* port number to send traffic to  - port 23 */
#define PRV_TGF_FDB_PORT_IDX_CNS            3

/* send VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS             2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS              4




/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;


/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;


/* VLANs array */
static GT_U16         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS};

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS
};

/* stored default FCOE State */
static GT_BOOL   prvTgfDefFcoeState;
static GT_BOOL   prvTgfDefIpPortFcoeState;
static GT_BOOL   prvTgfDefVlanFcoeState;

/* stored default FCOE EtherType */
static GT_U16   prvTgfDefFcoeEtherType;

/* the Arp Address index of the ARP Table
   to write to the arpPointer field */
static GT_U32        prvTgfArpLineIndex      = 1;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x34, 0x01};

/* the ARP MAC address to check in the arrived packet */
static TGF_MAC_ADDR  prvTgfMacSa = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};


/* PCL rule index */
static GT_U32        prvTgfRuleIndex;

/* LPM Db id used in tests */
static GT_U32        prvTgfLpmDBId = 55;

/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* ethertype part of FCOE packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketFcoeEtherTypePart = {TGF_ETHERTYPE_8906_OAM_TAG_CNS};

/* Data of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x36, /* SOF */
    0x50, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* VFT header */
    0x11, 0x22, 0x33, 0x00, 0x44, 0x55, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x88, 0x88, 0x99, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

CPSS_MAC_SA_LSB_MODE_ENT    prvTgfMacSaModeOrig;

/***************************** FCOE packet ********************************/

/* PARTS of packet FCOE */
static TGF_PACKET_PART_STC prvTgfFcoePacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketFcoeEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

extern GT_STATUS prvUtfIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC *ramDbCfgPtr
);

/* Length of FCOE packet */
#define PRV_TGF_FCOE_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadDataArr)

/* Length of FCOE packet with CRC */
#define PRV_TGF_FCOE_PACKET_CRC_LEN_CNS  \
    PRV_TGF_FCOE_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfFcoeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 FCOE packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:11,
*         FCOE EtherType = 0x8906
*         FCOE D_ID = 0x112233
*         FCOE S_ID = 0x445566
*         FCOE OX_ID = 0x8888
*         FCOE RX_ID = 0x9999
*         Success Criteria:
*         Packet is captured on port 23 and mirrored to CPU
*         Packet DA is changed according to ARP entry = 00:00:00:00:34:01
*         Packet SA is changed according to vlan value = 00:00:00:00:00:02
*/
GT_VOID prvTgfFcoeTrafficGenerate
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
    PRV_UTF_LOG0_MAC("======= prvTgfFcoeTrafficGenerate =======\n");

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

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS];

    /* enable capture on port 23 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* number of parts in packet */
    partsCount = sizeof(prvTgfFcoePacketPartArray) / sizeof(prvTgfFcoePacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfFcoePacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfFcoePacketPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* send packet */
    /* AUTODOC: send FCOE packet from port 0 with:                            */
    /* AUTODOC: DA=00:00:00:00:34:02, SA=00:00:00:00:00:11                    */
    /* AUTODOC: VID=2, EtherType = 0x8906, D_ID = 0x112233, S_ID = 0x445566   */
    /* AUTODOC: OX_ID = 0x8888, RX_ID = 0x9999                                */

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS]);


    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC:   verify to get FCOE packet on port 3 with:                  */
    /* AUTODOC:   DA=00:00:00:00:34:01, SA=00:00:00:00:00:02                 */
    /* AUTODOC:   Vlan tag, 81 00 00 02                                      */
    /* AUTODOC:   EtherType = 0x8906, D_ID = 0x112233, S_ID = 0x445566       */
    /* AUTODOC:   OX_ID = 0x8888, RX_ID = 0x9999                             */

    /* read and check ethernet counters */
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

            case PRV_TGF_GET_PORT_IDX_CNS:

                /* packetSize not changed*/
                expectedPacketSize = (packetSize + TGF_CRC_LEN_CNS);
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

    /* check if captured packet has the same MAC DA as "00:00:00:00:34:01" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

    /* check if captured packet has the same MAC SA as "00:00:00:00:00:02" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 6;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfMacSa, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfMacSa[0], prvTgfMacSa[1], prvTgfMacSa[2],
            prvTgfMacSa[3], prvTgfMacSa[4], prvTgfMacSa[5]);


    /* check if captured packet has the expected  vlan tag "81 00 00 02" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].cycleCount = 6;
    vfdArray[0].patternPtr[0] = 0x81;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x02;
    vfdArray[0].patternPtr[4] = 0x89;
    vfdArray[0].patternPtr[5] = 0x06;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n vlan tag captured packet must be: 81 00 10 03\nEtherType packet must be: 89 06");

   /* check if captured packet has the expected inner vlan tag "81 00 10 04" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 41;
    vfdArray[0].cycleCount = 7;
    vfdArray[0].patternPtr[0] = 0x11;
    vfdArray[0].patternPtr[1] = 0x22;
    vfdArray[0].patternPtr[2] = 0x33;
    vfdArray[0].patternPtr[3] = 0x00;
    vfdArray[0].patternPtr[4] = 0x44;
    vfdArray[0].patternPtr[5] = 0x55;
    vfdArray[0].patternPtr[6] = 0x66;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n D_ID packet must be: 11 22 33\n S_ID packet must be: 44 55 66");

   /* check if captured packet has the expected inner vlan tag "81 00 10 04" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 56;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = 0x88;
    vfdArray[0].patternPtr[1] = 0x88;
    vfdArray[0].patternPtr[2] = 0x99;
    vfdArray[0].patternPtr[3] = 0x99;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n OX_ID packet must be: 88 88\n RX_ID packet must be: 99 99");
}

/**
* @internal prvTgfFcoeBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfFcoeBaseConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                     isTagged  = GT_TRUE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_ETHERADDR                arpMacAddr;

    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    PRV_UTF_LOG0_MAC("======= VLAN Configuration =======\n");

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
    vlanInfo.ipv4UcastRouteEn     = GT_TRUE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;

    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* AUTODOC: create VLAN 2 with tagged ports [0,1,2,3] */
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

   /* AUTODOC: get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: set PVID 2 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 2 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= FDB Configuration =======\n");

    /* AUTODOC:  create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
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

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 2, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

     /* AUTODOC: write a ARP MAC 00:00:00:00:34:01 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfArpLineIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {    
        rc = prvTgfIpPortRouterMacSaLsbModeSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS],
            CPSS_SA_LSB_PER_VLAN_E);


            rc = cpssDxChIpRouterVlanMacSaLsbSet(prvTgfDevNum,PRV_TGF_SEND_VLANID_CNS,PRV_TGF_SEND_VLANID_CNS);  
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterVlanMacSaLsbSet: %d", prvTgfDevNum);  
    }
    /* AUTODOC: set MAC SA according to the packet's VLAN ID */
    else
    {
        rc = prvTgfIpPortRouterMacSaLsbModeSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS],
            CPSS_SA_LSB_PER_PKT_VID_E);
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpPortRouterMacSaLsbModeSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfFcoeBaseConfigurationRestore function
* @endinternal
*
* @brief   Base Configuration Restore
*/
GT_VOID prvTgfFcoeBaseConfigurationRestore
(
    GT_VOID
)
{
    GT_U32                      vlanIter  = 0;
    GT_U32                      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS                   rc        = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

     /* -------------------------------------------------------------------------
     * 1. Restore BASE Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

   /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: restore default Precedence on port 0 */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);

    /* AUTODOC: invalidate VLANs 2 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }
    /* AUTODOC: set MAC SA VLAN mode for egress port (default)*/
    rc = prvTgfIpPortRouterMacSaLsbModeSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS],
        CPSS_SA_LSB_PER_VLAN_E);
}

/**
* @internal prvTgfFcoePclConfigurationSet function
* @endinternal
*
* @brief   Set IPCL Configuration
*/
GT_VOID prvTgfFcoePclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                 interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC              lookupCfg;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT        ruleFormat;
    PRV_TGF_PCL_RULE_FORMAT_UNT             pclMask;
    PRV_TGF_PCL_RULE_FORMAT_UNT             pclPattern;
    PRV_TGF_PCL_ACTION_STC                  pclAction;
    PRV_TGF_PCL_UDB_SELECT_STC              udbSelect;

    PRV_UTF_LOG0_MAC("======= Setting PCL Configuration =======\n");

    /* AUTODOC:  UDB confihuration */

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                0,
                                PRV_TGF_PCL_OFFSET_L4_E,
                                1); /* skip 8 lsb */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                1,
                                PRV_TGF_PCL_OFFSET_L4_E,
                                2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                2,
                                PRV_TGF_PCL_OFFSET_L4_E,
                                3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                3,
                                PRV_TGF_PCL_OFFSET_L4_E,
                                5); /* skip 8 lsb */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                4,
                                PRV_TGF_PCL_OFFSET_L4_E,
                                6);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                5,
                                PRV_TGF_PCL_OFFSET_L4_E,
                                7);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                35,
                                PRV_TGF_PCL_OFFSET_METADATA_E,
                                41);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);


    /* AUTODOC:  PCL  confihuration */

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));
    udbSelect.udbSelectArr[0] = 0;
    udbSelect.udbSelectArr[1] = 1;
    udbSelect.udbSelectArr[2] = 2;
    udbSelect.udbSelectArr[3] = 3;
    udbSelect.udbSelectArr[4] = 4;
    udbSelect.udbSelectArr[5] = 5;
    udbSelect.udbSelectArr[6] = 35;
    udbSelect.udbSelectArr[7] = 7;
    udbSelect.udbSelectArr[8] = 8;
    udbSelect.udbSelectArr[9] = 9;

    rc = prvTgfPclUserDefinedBytesSelectSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E /*ruleFormat*/,
            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E /*packetType*/,
            CPSS_PCL_LOOKUP_0_E /*lookupNum*/,
            &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d", prvTgfDevNum);

    /* AUTODOC:  Enables Ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC:  Enable ingress policy per port */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* AUTODOC:  Sets PCL Configuration table entry's lookup configuration for interface */

    /* AUTODOC:  PCL configuration table */

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));

    interfaceInfo.type=CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum=prvTgfDevNum;
    interfaceInfo.devPort.portNum=prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.lookupType             = PRV_TGF_PCL_LOOKUP_TYPE_SINGLE_E;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.externalLookup         = GT_FALSE;
    lookupCfg.pclId                  = 0;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;

    rc = prvTgfPclCfgTblSet( &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");



    /* AUTODOC:  Configures PORT access mode to Ingress Policy configuration table per lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                CPSS_PCL_DIRECTION_INGRESS_E,
                                                CPSS_PCL_LOOKUP_0_E,0,
                                                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");



    /* AUTODOC:  Set PCL Action */
    cpssOsMemSet(&pclAction, 0, sizeof(pclAction));
    cpssOsMemSet(&pclMask, 0, sizeof(pclMask));
    cpssOsMemSet(&pclPattern, 0, sizeof(pclPattern));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
    prvTgfRuleIndex = 0;

    pclMask.ruleIngrUdbOnly.udb[0]=0xFF;
    pclPattern.ruleIngrUdbOnly.udb[0]=0x11;

    pclMask.ruleIngrUdbOnly.udb[1]=0xFF;
    pclPattern.ruleIngrUdbOnly.udb[1]=0x22;

    pclMask.ruleIngrUdbOnly.udb[2]=0xFF;
    pclPattern.ruleIngrUdbOnly.udb[2]=0x33;

    pclMask.ruleIngrUdbOnly.udb[3]=0xFF;
    pclPattern.ruleIngrUdbOnly.udb[3]=0x44;

    pclMask.ruleIngrUdbOnly.udb[4]=0xFF;
    pclPattern.ruleIngrUdbOnly.udb[4]=0x55;

    pclMask.ruleIngrUdbOnly.udb[5]=0xFF;
    pclPattern.ruleIngrUdbOnly.udb[5]=0x66;

    pclMask.ruleIngrUdbOnly.udb[6]=0x50;
    pclPattern.ruleIngrUdbOnly.udb[6]=0x50;


    cpssOsMemSet(&pclAction, 0, sizeof(pclAction));
    pclAction.bypassBridge = GT_TRUE;
    pclAction.pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    pclAction.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    pclAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    pclAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS];

    pclAction.redirect.data.outIf.modifyMacSa = GT_TRUE;

    pclAction.redirect.data.outIf.modifyMacDa = GT_TRUE;
    pclAction.redirect.data.outIf.outlifPointer.arpPtr = prvTgfArpLineIndex;


    rc = prvTgfPclRuleSet(ruleFormat, prvTgfRuleIndex, &pclMask, &pclPattern, &pclAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet");
}

/**
* @internal prvTgfFcoePclConfigurationRestore function
* @endinternal
*
* @brief   Restore IPCL Configuration
*/
GT_VOID prvTgfFcoePclConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;
    PRV_TGF_PCL_UDB_SELECT_STC  udbSelect;

    /* AUTODOC: restore UDB CFG table */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                0,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                1,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                2,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                3,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                5);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                4,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                6);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                5,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                7);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                35,
                                PRV_TGF_PCL_OFFSET_INVALID_E,
                                41);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    rc = prvTgfPclUserDefinedBytesSelectSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E /*ruleFormat*/,
            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E /*packetType*/,
            CPSS_PCL_LOOKUP_0_E /*lookupNum*/,
            &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d", prvTgfDevNum);

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));

    interfaceInfo.type=CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum=prvTgfDevNum;
    interfaceInfo.devPort.portNum=prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    lookupCfg.enableLookup              = GT_FALSE;
    lookupCfg.lookupType                = PRV_TGF_PCL_LOOKUP_TYPE_SINGLE_E;
    lookupCfg.dualLookup                = GT_FALSE;
    lookupCfg.externalLookup            = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey    =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key     =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key     =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC:  disable Ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: disable ingress policy on port */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, prvTgfRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");
}

/**
* @internal prvTgfTtiFcoeForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE Global Configuration
*/
GT_VOID prvTgfTtiFcoeForwardingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enable;
    GT_U16      etherType;

    PRV_UTF_LOG0_MAC("======= Setting FCOE Configuration =======\n");

    /* Get default FCoE (Fiber Channel over Ethernet) state */
    rc = prvTgfTtiFcoeForwardingEnableGet(&prvTgfDefFcoeState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeForwardingEnableGet: %d", prvTgfDevNum);

    /* AUTODOC: Enable FCoE (Fiber Channel over Ethernet) */
    rc = prvTgfTtiFcoeForwardingEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeForwardingEnableSet: %d", prvTgfDevNum);

    /* Get FCoE (Fiber Channel over Ethernet) state */
    rc = prvTgfTtiFcoeForwardingEnableGet(&enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeForwardingEnableGet: %d", prvTgfDevNum);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, enable, "FCOE enable: %d", enable);

    /* Get default FCoE (Fiber Channel over Ethernet) Ether Type value */
    rc = prvTgfTtiFcoeEtherTypeGet(&prvTgfDefFcoeEtherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeEtherTypeGet: %d", prvTgfDevNum);

    /* AUTODOC: Set FCoE (Fiber Channel over Ethernet) Ether Type value. */
    rc = prvTgfTtiFcoeEtherTypeSet(TGF_ETHERTYPE_8906_OAM_TAG_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeEtherTypeSet: %d", prvTgfDevNum);

    /* Get FCoE (Fiber Channel over Ethernet) Ether Type value. */
    rc = prvTgfTtiFcoeEtherTypeGet(&etherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeEtherTypeGet: %d", prvTgfDevNum);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(TGF_ETHERTYPE_8906_OAM_TAG_CNS, etherType, "FCOE etherType: %d", etherType);
}

/**
* @internal prvTgfTtiFcoeForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE Global Configuration
*/
GT_VOID prvTgfTtiFcoeForwardingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= prvTgfTtiFcoeForwardingConfigurationRestore =======\n");

    /* AUTODOC: Restore Default FCoE (Fiber Channel over Ethernet) state */
    rc = prvTgfTtiFcoeForwardingEnableSet(prvTgfDefFcoeState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeForwardingEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: Restore FCoE (Fiber Channel over Ethernet) Ether Type value. */
    rc = prvTgfTtiFcoeEtherTypeSet(prvTgfDefFcoeEtherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeEtherTypeSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpPortFcoeForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE Configuration
*/
GT_VOID prvTgfIpPortFcoeForwardingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_BOOL                                 enable;
    GT_U32                                  prvTgfRouteEntryBaseIndex = 1;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntry[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    CPSS_IP_PROTOCOL_STACK_ENT              protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT        shadowType=CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;
    GT_U32                                  prvUtfVrfId = 0x101;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defaultFcoeForwardingNextHopInfo;

    PRV_UTF_LOG0_MAC("======= prvTgfIpPortForwardingConfigurationSet =======\n");

    cpssOsBzero((GT_VOID*)&ramDbCfg, sizeof(ramDbCfg));

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfDevNum, shadowType);

    prvTgfIpLpmDBDelete(prvTgfLpmDBId);
    rc = prvTgfIpLpmRamDBCreate(prvTgfLpmDBId, protocolStack, &ramDbCfg);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "%d, %d, %d, "
            "ramDbCfg.numOfBlocks = %d,"
            "ramDbCfg.blocksAllocationMethod = %d",
            prvTgfLpmDBId, shadowType, protocolStack,
            ramDbCfg.numOfBlocks,
            ramDbCfg.blocksAllocationMethod);

    prvTgfIpLpmDBDevListAdd(prvTgfLpmDBId, &prvTgfDevNum, 1);

    rc = prvTgfIpPortFcoeForwardingEnableGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefIpPortFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortFcoeForwardingEnableGet: %d %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfIpPortFcoeForwardingEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortFcoeForwardingEnableSet: %d %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Get FCoE (Fiber Channel over Ethernet) state */
    rc = prvTgfIpPortFcoeForwardingEnableGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortFcoeForwardingEnableGet: %d %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, enable,
                                 "FCOE enable: %d", enable);

    rc = prvTgfTtiFcoeAssignVfIdEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiFcoeAssignVfIdEnableSet: %d %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    rc = prvTgfTtiFcoeAssignVfIdEnableGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &enable);

    cpssOsMemSet(&routeEntry, 0, sizeof(routeEntry));
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    regularEntryPtr = &routeEntry[0];

    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->isTunnelStart              = GT_FALSE;

    regularEntryPtr->nextHopVlanId              = PRV_TGF_SEND_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = prvTgfArpLineIndex;

    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    cpssOsMemSet(&defaultFcoeForwardingNextHopInfo, 0, sizeof(defaultFcoeForwardingNextHopInfo));

    defaultFcoeForwardingNextHopInfo.ipLttEntry.routeEntryBaseIndex        = prvTgfRouteEntryBaseIndex;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel      = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.numOfPaths                 = 0;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.routeType                  = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.ucRPFCheckEnable           = GT_FALSE;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable   = GT_FALSE;

    rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId, NULL, NULL, NULL, NULL, &defaultFcoeForwardingNextHopInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd: %d", prvTgfDevNum);

    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

}

/**
* @internal prvTgfIpPortFcoeForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE Configuration
*/
GT_VOID prvTgfIpPortFcoeForwardingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      prvUtfVrfId = 0x101;

    PRV_UTF_LOG0_MAC("======= prvTgfIpPortForwardingConfigurationRestore =======\n");

    rc = prvTgfIpPortFcoeForwardingEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfDefIpPortFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortFcoeForwardingEnableSet: %d %d", prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);

    rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", prvUtfVrfId);

    rc = prvTgfIpLpmDBDelete(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDBDelete: %d", prvTgfLpmDBId);

}

/**
* @internal prvTgfVlanFcoeForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE Configuration
*/
GT_VOID prvTgfVlanFcoeForwardingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enable;

    PRV_UTF_LOG0_MAC("======= prvTgfVlanFcoeForwardingConfigurationSet =======\n");

    rc = prvTgfBrgVlanFcoeForwardingEnableGet(PRV_TGF_SEND_VLANID_CNS, &prvTgfDefVlanFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFcoeForwardingEnableGet: %d %d", prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgVlanFcoeForwardingEnableSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFcoeForwardingEnableSet: %d %d", prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* Get FCoE (Fiber Channel over Ethernet) state */
    rc = prvTgfBrgVlanFcoeForwardingEnableGet(PRV_TGF_SEND_VLANID_CNS, &enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFcoeForwardingEnableGet: %d %d", prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, enable, "FCOE enable: %d", enable);
}

/**
* @internal prvTgfVlanFcoeForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE Configuration
*/
GT_VOID prvTgfVlanFcoeForwardingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= prvTgfVlanFcoeForwardingConfigurationRestore =======\n");

    rc = prvTgfBrgVlanFcoeForwardingEnableSet(PRV_TGF_SEND_VLANID_CNS, prvTgfDefVlanFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFcoeForwardingEnableSet: %d %d", prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

/**
* @internal prvTgfBasicFcoeLpmForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE LPM Configuration
*/
GT_VOID prvTgfBasicFcoeLpmForwardingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_FCID                                 fcoeAddr;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_U32                                  prvUtfVrfId = 0x101;
    GT_U32                                  prvTgfRouteEntryBaseIndex = 1;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntry[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    CPSS_IP_PROTOCOL_STACK_ENT              protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT        shadowType=CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defaultFcoeForwardingNextHopInfo;

    PRV_UTF_LOG0_MAC("======= prvTgfBasicFcoeLpmForwardingConfigurationSet =======\n");

    cpssOsBzero((GT_VOID*)&ramDbCfg, sizeof(ramDbCfg));

    rc = prvUtfIpLpmRamDefaultConfigCalc(prvTgfDevNum, (CPSS_DXCH_LPM_RAM_CONFIG_STC *)&ramDbCfg);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfDevNum, shadowType);

    prvTgfIpLpmDBDelete(prvTgfLpmDBId);
    rc = prvTgfIpLpmRamDBCreate(prvTgfLpmDBId, protocolStack, &ramDbCfg);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "%d, %d, %d, "
            "ramDbCfg.numOfBlocks = %d,"
            "ramDbCfg.blocksAllocationMethod = %d",
            prvTgfLpmDBId, shadowType, protocolStack,
            ramDbCfg.numOfBlocks,
            ramDbCfg.blocksAllocationMethod);

    prvTgfIpLpmDBDevListAdd(prvTgfLpmDBId, &prvTgfDevNum, 1);

    /* Save and enable FCOE forwarding for port */
    rc = prvTgfIpPortFcoeForwardingEnableGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefIpPortFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortFcoeForwardingEnableGet: %d %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfIpPortFcoeForwardingEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortFcoeForwardingEnableSet: %d %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Save and enable FCOE forwarding for vlan */
    rc = prvTgfBrgVlanFcoeForwardingEnableGet(PRV_TGF_SEND_VLANID_CNS, &prvTgfDefVlanFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFcoeForwardingEnableGet: %d %d", prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgVlanFcoeForwardingEnableSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFcoeForwardingEnableSet: %d %d", prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);


    cpssOsMemSet(&routeEntry, 0, sizeof(routeEntry));
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, 0, routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    regularEntryPtr = &routeEntry[0];

    regularEntryPtr->cmd                                = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex                       = 0;
    regularEntryPtr->ingressMirror                      = GT_FALSE;
    regularEntryPtr->isTunnelStart                      = GT_FALSE;

    regularEntryPtr->nextHopVlanId                      = PRV_TGF_SEND_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type              = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_GET_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer                  = prvTgfArpLineIndex;

    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    cpssOsMemSet(&defaultFcoeForwardingNextHopInfo, 0, sizeof(defaultFcoeForwardingNextHopInfo));

    defaultFcoeForwardingNextHopInfo.ipLttEntry.routeEntryBaseIndex        = prvTgfRouteEntryBaseIndex+10; /* Set wrong entry index to check nextHop */
    defaultFcoeForwardingNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel      = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.numOfPaths                 = 0;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.routeType                  = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.ucRPFCheckEnable           = GT_FALSE;
    defaultFcoeForwardingNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable   = GT_FALSE;

    rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId, NULL, NULL, NULL, NULL, &defaultFcoeForwardingNextHopInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd: %d", prvTgfDevNum);

    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

    fcoeAddr.fcid[0] = 0x11;
    fcoeAddr.fcid[1] = 0x22;
    fcoeAddr.fcid[2] = 0x33;

    rc = prvTgfIpLpmFcoePrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    fcoeAddr, 24, &nextHopInfo, GT_FALSE,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmFcoePrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicFcoeLpmForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE LPM Configuration
*/
GT_VOID prvTgfBasicFcoeLpmForwardingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  prvUtfVrfId = 0x101;
    GT_FCID                                 fcoeAddr;

    PRV_UTF_LOG0_MAC("======= prvTgfBasicFcoeLpmForwardingConfigurationRestore =======\n");

    fcoeAddr.fcid[0] = 0x11;
    fcoeAddr.fcid[1] = 0x22;
    fcoeAddr.fcid[2] = 0x33;
    rc = prvTgfIpLpmFcoePrefixDel(prvTgfLpmDBId, prvUtfVrfId, fcoeAddr, 24);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmFcoePrefixDel: %d", prvTgfDevNum);

    rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", 0x101);

    rc = prvTgfIpLpmDBDelete(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDBDelete: %d", prvTgfLpmDBId);

    rc = prvTgfIpPortFcoeForwardingEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfDefIpPortFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortFcoeForwardingEnableSet: %d %d", prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);

    rc = prvTgfBrgVlanFcoeForwardingEnableSet(PRV_TGF_SEND_VLANID_CNS, prvTgfDefVlanFcoeState);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFcoeForwardingEnableSet: %d %d", prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

