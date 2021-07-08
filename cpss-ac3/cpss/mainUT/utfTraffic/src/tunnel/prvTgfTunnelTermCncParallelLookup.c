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
* @file prvTgfTunnelTermCncParallelLookup.c
*
* @brief Verify the functionality of TTI Cnc Parallel lookup
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

#include <common/tgfPacketGen.h>
#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfCncGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS          6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

static struct{

    GT_U32 isValid;
    GT_U32 ttiRuleIndex;

}lookupInfoStc[PRV_TGF_MAX_LOOKUPS_NUM_CNS];


/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* packet size */
static GT_U32   packetSize;

/******************************* Test packet **********************************/

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x05},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x15}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart = {
    0x1000,             /* label */
    0,                  /* CoS */
    1,                  /* S */
    0xff                /* TTL */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of basic packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

#define CNC_CLIENT_FOR_LOOKUP_MAC(lookupNum)                        \
    ((PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum)) ?                  \
        ((lookupNum == 0) ? PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E :   \
         (lookupNum == 1) ? PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E :   \
         (lookupNum == 2) ? PRV_TGF_CNC_CLIENT_TTI_PARALLEL_2_E :   \
                            PRV_TGF_CNC_CLIENT_TTI_PARALLEL_3_E) :  \
        ((lookupNum == 0) ? PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E :   \
         (lookupNum == 1) ? PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E :   \
         (lookupNum == 2) ? PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E :   \
                            PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E))

#define CNC_BMP_FOR_LOOKUP_MAC(lookupNum,range)     \
    ((PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum)) ?  (range) :             \
        (lookupNum > 1 ? (range << 16) : range))

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermCncParallelLookupBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID prvTgfTunnelTermCncParallelLookupBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermCncParallelLookupTtiConfigPerLookupSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermCncParallelLookupTtiConfigPerLookupSet
(
    GT_U8 lookupNum
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction,  0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Rule =======\n");

    /* AUTODOC: set the TTI Rule Action for all the packets */
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiAction.bindToCentralCounter = GT_TRUE;
    ttiAction.centralCounterIndex  = lookupNum + 1;

    ttiAction.ingressPipeBypass = GT_TRUE;

    ttiAction.tag0VlanCmd       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId        = PRV_TGF_EGR_VLANID_CNS;

    if(lookupNum != (PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1) )
    {
        ttiAction.continueToNextTtiLookup = GT_TRUE;
    }

    /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with action: bind to cnc index */
    rc = prvTgfTtiRule2Set(lookupInfoStc[lookupNum].ttiRuleIndex,
                            PRV_TGF_TTI_KEY_MPLS_E, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prvTgfTunnelTermCncParallelLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfTunnelTermCncParallelLookupTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:05, SA=00:00:00:00:00:15 */
    /* AUTODOC:   VID=5, Ethertype=0x8847, MPLS Label=0x1000 */
    /* AUTODOC:   Passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:12 */

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        {
            GT_BOOL isOk;
            GT_U32  expectedPacketSize;
            CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

            /* AUTODOC: check Tx and Rx counters */
            switch (portIter)
            {
                case PRV_TGF_SEND_PORT_IDX_CNS:

                    /* AUTODOC: packetSize is not changed */
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

            case PRV_TGF_EGR_PORT_IDX_CNS:

                    expectedPacketSize = packetSize;

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

            UTF_VERIFY_EQUAL1_STRING_MAC(isOk, GT_TRUE, "get another counters values, port %d",
                                                                prvTgfPortsArray[portIter]);

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

#if 0
        if(PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
            /* AUTODOC: verify to get Ethernet packet on port 3 with: */
            UTF_VERIFY_EQUAL1_STRING_MAC(portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                "there is no traffic on expected port, packet Ether type: 0x%X\n",
                                                prvTgfPacketEtherTypePart.etherType);
        }
#endif
    }

}

/**
* @internal prvTgfTunnelTermCncParallelLookupConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
static GT_VOID prvTgfTunnelTermCncParallelLookupConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      lookupNum;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MPLS */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    for(lookupNum = 0; lookupNum < PRV_TGF_MAX_LOOKUPS_NUM_CNS; lookupNum++)
    {
        if(0 == lookupInfoStc[lookupNum].isValid)
        {
            break;
        }

        /* AUTODOC: invalidate TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(lookupInfoStc[lookupNum].ttiRuleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");
    }

    /* AUTODOC: disable CNC for TTI unit */
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}


/**
* @internal prvTgfTunnelTermCncParallelLookupCncConfigPerLookupSet function
* @endinternal
*
* @brief   build cnc config per lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermCncParallelLookupCncConfigPerLookupSet
(
    GT_U32 lookupNum
)
{
    GT_STATUS  rc = GT_OK;

    GT_U32  const range = 0x1;

    PRV_TGF_CNC_CONFIGURATION_STC cncCfg;
    PRV_TGF_CNC_COUNTER_STC counterValue0;

    /* AUTODOC: define CNC configuration */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    cpssOsMemSet(&counterValue0, 0, sizeof(counterValue0));

    /* common parameters */
    cncCfg.clientType = CNC_CLIENT_FOR_LOOKUP_MAC(lookupNum);

    cncCfg.counterNum          = lookupNum + 1;

    if ((lookupNum > 1) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum)) &&
        (! PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum)))
    {
        cncCfg.blockNum        = lookupNum + 16; 
    }
    else
    {
        cncCfg.blockNum        = lookupNum; 
    }

    cncCfg.indexRangeBitMap[0] = CNC_BMP_FOR_LOOKUP_MAC(lookupNum,range);

    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* prvTgfCncCountingEnableSet */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;

    /* prvTgfCncBlockClientEnableSet */
    cncCfg.configureClientEnable       = GT_TRUE;
    cncCfg.clientEnable                = GT_TRUE;

    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureIndexRangeBitMap   = GT_TRUE;

    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue       = GT_TRUE;
    cncCfg.counterValue                = counterValue0;

    /* prvTgfCncClientByteCountModeSet */
    cncCfg.configureByteCountMode      = GT_TRUE;
    cncCfg.byteCountMode               = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;

    /* prvTgfCncCounterClearByReadEnableSet */
    cncCfg.configureClearByReadEnable  = GT_TRUE;
    cncCfg.clearByReadEnable           = GT_TRUE;

    rc = prvTgfCncGenConfigure(&cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenConfigure");

}

/**
* @internal prvTgfTunnelTermCncParallelLookupCncConfigPerLookupRestore function
* @endinternal
*
* @brief   Restore CNC Configuration per lookup.
*/
static GT_VOID prvTgfTunnelTermCncParallelLookupCncConfigPerLookupRestore
(
    GT_U32 lookupNum
)
{
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E;
    cncCfg.clientType    = CNC_CLIENT_FOR_LOOKUP_MAC(lookupNum);

    cncCfg.counterNum          = lookupNum + 1;
    cncCfg.blockNum            = lookupNum;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    cncCfg.configureCounterValue = GT_TRUE;     /*counterValue == 0*/

    prvTgfCncGenConfigure(
        &cncCfg, GT_FALSE /*stopOnErrors*/);
}

/**
* @internal prvTgfTunnelTermCncParallelLookupCncCheck function
* @endinternal
*
* @brief   check cnc counters per lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermCncParallelLookupCncCheck
(
    GT_U32 lookupNum
)
{
    GT_STATUS  rc = GT_OK;

    PRV_TGF_CNC_CHECK_STC cncCheck;

    /* AUTODOC: get and print CNC counters values */
    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));

    /* common parameters */
    if ((lookupNum > 1) &&
        (PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum)) &&
        (! PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum)))
    {
        cncCheck.blockNum  = lookupNum + 16;
    }
    else
    {
        cncCheck.blockNum  = lookupNum;
    }
    cncCheck.counterNum    = lookupNum + 1; 
    cncCheck.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* prvTgfCncCounterGet */
    cncCheck.compareCounterValue = GT_TRUE;
    cncCheck.counterValue.packetCount.l[0] = prvTgfBurstCount;
    cncCheck.counterValue.byteCount.l[0]   = packetSize + 4/*crc*/;

    /* AUTODOC: verify CNC counters for CNC format mode 0: */
    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenCheck");

    PRV_UTF_LOG2_MAC("CNC counters for counter 0 :[%d] [%d]\n\n",
                                cncCheck.counterValue.packetCount.l[0],
                                cncCheck.counterValue.byteCount.l[0] );
}


/**
* @internal prvTgfTunnelTermCncParallelLookup function
* @endinternal
*
* @brief   Test of TTI Cnc Parallel lookup (main func)
*/
GT_VOID prvTgfTunnelTermCncParallelLookup
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       lookupNum;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    prvTgfTunnelTermCncParallelLookupBridgeConfigSet();

    /* AUTODOC: enable the TTI lookup for MPLS at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                        PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    /* AUTODOC: set TTI lookups configuration */
    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS; lookupNum++)
    {
        PRV_UTF_LOG1_MAC("\n============================ Process lookup %d =======================\n", lookupNum);

        /* AUTODOC: init lookup data */
        lookupInfoStc[lookupNum].isValid = 1;
        lookupInfoStc[lookupNum].ttiRuleIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(lookupNum , 3);

        /* AUTODOC: set the TTI Rule Action for all the packets */
        /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with */
        /* AUTODOC: TTI action: bind to cnc counter, modify vid to 6, redirect to port 36 */
        prvTgfTunnelTermCncParallelLookupTtiConfigPerLookupSet(lookupNum);

        /* AUTODOC: set CNC configuration per lookup */
        prvTgfTunnelTermCncParallelLookupCncConfigPerLookupSet(lookupNum);
    }


    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 */
    prvTgfTunnelTermCncParallelLookupTrafficGenerate();


    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        PRV_UTF_LOG1_MAC("\n========= Check CNC counters for lookup %d ===========\n", lookupNum);

        /* AUTODOC: check cnc counters */
        prvTgfTunnelTermCncParallelLookupCncCheck(lookupNum);

        /* AUTODOC: restore cnc counters configurations */
        prvTgfTunnelTermCncParallelLookupCncConfigPerLookupRestore(lookupNum);
    }


    /*--------------------------------*/
    /* AUTODOC: test configuration restore */
    prvTgfTunnelTermCncParallelLookupConfigRestore();
}


