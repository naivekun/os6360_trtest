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
* @file prvTgfConfigInit.c
*
* @brief CPSS Config.
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#endif

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfConfigGen.h>
#include <config/prvTgfConfigInit.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  0x1FF

#define PRV_TGF_VLANID_5_CNS 5

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* another port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS 0

/* port number to receive traffic */
#define PRV_TGF_RCV1_PORT_IDX_CNS         2

/* port number to receive traffic */
#define PRV_TGF_RCV_PORT_IDX_CNS          3

#define SOURCE_ID_MASK_CNS                0xFFF

#define SOURCE_ID_CNS                     0x800
#define INVALID_SOURCE_ID_CNS             0x700
/* default number of packets to send */
static GT_U32   prvTgfBurstCount   = 1;
static GT_U16   prvTgfDefVlanId = 0;
static GT_BOOL  isEgress = GT_FALSE;
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x00, 0x5E, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};


/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static TGF_MAC_ADDR                 prvTgfDaMacArr[] = {{0x00, 0x00, 0x00, 0x00, 0x34, 0x02},
                                                        {0x00, 0x00, 0x00, 0x00, 0x34, 0x04}};
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {
    TGF_ETHERTYPE_0806_ARP_TAG_CNS
};
/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacket1ArpPart = {
    0x1, 0x800, 0x6, 0x4, 0x1,
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x01, 0x01, 0x01, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00}
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_ARP_E,       &prvTgfPacket1ArpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_ARP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/* define expected number of received packets (Sent packets) */
static GT_U8 prvTgfPacketsCountRxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports: FREE SEND FDB1 FDB2 */
             {0,  1,  0,  0},
             {0,  1,  0,  0}
};

/* define expected number of transmitted packets */
static GT_U8 prvTgfPacketsCountTxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports: FREE SEND FDB1 FDB2 */
             {0,  1,  0,  0},
             {0,  1,  0,  1}
};



/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCfgIngressDropCounterTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portIndex                - port index to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
*                                       None
*/
static GT_VOID prvTgfCfgIngressDropCounterTestPacketSend
(
    IN GT_U32          portIndex,
    IN TGF_PACKET_STC *packetInfoPtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portIter = 0;

    /* reset the ingress drop counter */
    rc = prvTgfCfgIngressDropCntrSet(0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrSet");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* AUTODOC: send Ethernet packet to port 1 with: */
    /* AUTODOC:   DA=01:00:5e:00:00:01, SA=00:00:00:00:00:02, VID=0x1FF */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIndex]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
    
}



/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfCfgIngressDropCounterSet function
* @endinternal
*
* @brief   Set test configuration:
*         Set configuration to check ingress drop counter.
*/
GT_VOID prvTgfCfgIngressDropCounterSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 0x1FF with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: discard all non-Control-classified Ethernet MC packets */
    rc = prvTgfBrgGenDropIpMcEnable(GT_TRUE); 
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenDropIpMcEnable");

}

/**
* @internal prvTgfCfgIngressDropCounterTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*         - Set Ingress drop counter mode to be port mode and
*         set port number to 0.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 0.
*         - Set set port number to 8.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*         - Set Ingress drop counter mode to be port mode and
*         set port number to 0.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 0.
*         - Set Ingress drop counter mode to be vlan mode and
*         set vlan number to 0.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 0.
*         - Set Ingress drop counter mode to be vlan mode and
*         set vlan number to 5.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*         - Set Ingress drop counter mode to be ALL.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*/
GT_VOID prvTgfCfgIngressDropCounterTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       dropCounter;  /* ingress drop counter */

    /* AUTODOC: GENERATE TRAFFIC: */

    /* send packet */
    prvTgfCfgIngressDropCounterTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS, &prvTgfPacketInfo);

    /* AUTODOC: verify to get no traffic and ingress dropCounter=1 */
    rc = prvTgfCfgIngressDropCntrGet(&dropCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, dropCounter,
                                 "get another dropCounter counter than expected");

    /* AUTODOC: set Ingress Drop Counter Mode to MODE_PORT for port=0 */
    rc = prvTgfCfgIngressDropCntrModeSet(PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrModeSet");

    /* send packet */
    prvTgfCfgIngressDropCounterTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS, 
                                              &prvTgfPacketInfo);

    /* AUTODOC: verify to get no traffic and ingress dropCounter=0 */
    rc = prvTgfCfgIngressDropCntrGet(&dropCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dropCounter,
                                 "get another dropCounter counter than expected");

    /* AUTODOC: set Ingress Drop Counter Mode to MODE_PORT for port=1 */
    rc = prvTgfCfgIngressDropCntrModeSet(PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E,
                                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrModeSet");

    /* send packet */
    prvTgfCfgIngressDropCounterTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS, 
                                              &prvTgfPacketInfo);

    /* AUTODOC: verify to get no traffic and ingress dropCounter=1 */
    rc = prvTgfCfgIngressDropCntrGet(&dropCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, dropCounter,
                                 "get another dropCounter counter than expected");

    /* AUTODOC: set Ingress Drop Counter Mode to MODE_VLAN for vid=8 */
    rc = prvTgfCfgIngressDropCntrModeSet(PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E, 0, 8);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrModeSet");

    /* send packet */
    prvTgfCfgIngressDropCounterTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                               &prvTgfPacketInfo);

    /* AUTODOC: verify to get no traffic and ingress dropCounter=0 */
    rc = prvTgfCfgIngressDropCntrGet(&dropCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dropCounter,
                                 "get another dropCounter counter than expected");

    /* AUTODOC: set Ingress Drop Counter Mode to MODE_VLAN for vid=0x1FF */
    rc = prvTgfCfgIngressDropCntrModeSet(PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E, 0, PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrModeSet");

    /* send packet */
    prvTgfCfgIngressDropCounterTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                               &prvTgfPacketInfo);

    /* AUTODOC: verify to get no traffic and ingress dropCounter=1 */
    rc = prvTgfCfgIngressDropCntrGet(&dropCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, dropCounter,
                                 "get another dropCounter counter than expected");

    /* AUTODOC: set Ingress Drop Counter Mode to MODE_ALL */
    rc = prvTgfCfgIngressDropCntrModeSet(PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrModeSet");

    /* send packet */
    prvTgfCfgIngressDropCounterTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                               &prvTgfPacketInfo);

    /* AUTODOC: verify to get no traffic and ingress dropCounter=1 */
    rc = prvTgfCfgIngressDropCntrGet(&dropCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, dropCounter,
                                 "get another dropCounter counter than expected");

}

/**
* @internal prvTgfCfgIngressDropCounterRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCfgIngressDropCounterRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: reset Ingress Drop counters */
    rc = prvTgfCfgIngressDropCntrSet(0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrSet");

    /* AUTODOC: disable discarding of all non-Control-classified Ethernet MC packets */
    rc = prvTgfBrgGenDropIpMcEnable(GT_FALSE); 
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenDropIpMcEnable");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfCfgProbePacketCountersEthCheck function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCfgProbePacketCountersEthCheck
(
    IN GT_U32  portNum,
    IN GT_U8  packetsCountRx,
    IN GT_U8  packetsCountTx,
    IN GT_U32 packetSize,
    IN GT_U32 burstCount
)
{
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_BOOL                         isCountersOk;
    GT_STATUS                       rc = GT_OK;

    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, portNum, GT_TRUE, &portCntrs);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfReadPortCountersEth");

    /* calculate expected numder of pakets */
    /* all packets MUST have the same totalLen */
    cpssOsMemSet(&expectedCntrs, 0, sizeof(expectedCntrs));
    expectedCntrs.goodPktsSent.l[0]   = packetsCountTx * burstCount;
    expectedCntrs.goodPktsRcv.l[0]    = packetsCountRx * burstCount;
    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * packetsCountTx * burstCount;
    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * packetsCountRx * burstCount;

    /* check Tx and Rx counters */
    isCountersOk =
        portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
        portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
        portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
        portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0];

    /* print expected values if bug */
    if (GT_TRUE != isCountersOk) {
        PRV_UTF_LOG0_MAC("Expected values:\n");
        PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
        PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
        PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
        PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);

        /* print error message if bug */
        if (GT_FALSE == utfEqualVerify(isCountersOk, GT_TRUE, __LINE__, __FILE__)) {
            rc = utfFailureMsgLog("get another counters values.", NULL, 0);
        }

        PRV_UTF_LOG0_MAC("\n");
    }

    return rc;
};

/**
* @internal prvTgfCfgProbePacketBasicConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfCfgProbePacketBasicConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 5 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfDaMacArr[0], PRV_TGF_VLANID_5_CNS, prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RCV1_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCfgProbePacketPclConfigSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfCfgProbePacketPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    CPSS_DXCH_CFG_PROBE_PACKET_STC   probeCfg;

    /* AUTODOC: init PCL Engine for sending to port 1 */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: add PCL rule 0 with following parameters: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd DROP_HARD */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfDaMacArr[0], sizeof(prvTgfDaMacArr[0]));

    ruleIndex                   = 0;
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd               = CPSS_PACKET_CMD_DROP_HARD_E;
    action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VID_E;
    action.sourceId.assignSourceId = GT_TRUE;
    action.sourceId.sourceIdValue = SOURCE_ID_CNS;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);

    rc = prvTgfPclSourceIdMaskSet(CPSS_PCL_LOOKUP_0_E, SOURCE_ID_MASK_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d",
                                 prvTgfDevNum);

    action.mirror.cpuCode        = CPSS_NET_FIRST_USER_DEFINED_E;

    rc = prvTgfPclRuleActionUpdate(
        CPSS_PCL_RULE_SIZE_STD_E,
        ruleIndex, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleActionUpdate",prvTgfDevNum);

    /* AUTODOC: Set the probe packet configuration. */
    probeCfg.bitLocation = 11;
    probeCfg.enable      = GT_TRUE;

    rc = cpssDxChCfgProbePacketConfigSet(prvTgfDevNum, &probeCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgProbePacketConfigSet: %d",prvTgfDevNum);

}

/**
* @internal prvTgfCfgProbePacketSourceIdUpdate function
* @endinternal
*
* @brief  Source-Id probe packet bit location update.
*/

GT_VOID prvTgfCfgProbePacketSourceIdUpdate
(
    GT_VOID
)
{
    GT_STATUS    rc         = GT_OK;
    PRV_TGF_PCL_ACTION_STC      action;
    GT_U32         ruleIndex;

    ruleIndex                   = 0;
    /* clear action */
    cpssOsMemSet(&action, 0, sizeof(action));

    action.sourceId.assignSourceId = GT_TRUE;
    action.pktCmd                  = CPSS_PACKET_CMD_DROP_HARD_E;
    action.sourceId.sourceIdValue  = INVALID_SOURCE_ID_CNS;
    action.mirror.cpuCode          = CPSS_NET_LAST_USER_DEFINED_E;

    rc = prvTgfPclRuleActionUpdate(
        CPSS_PCL_RULE_SIZE_STD_E,
        ruleIndex, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleActionUpdate",prvTgfDevNum);
}


/**
* @internal prvTgfPclEgressTrapMirrorConfigurationSet function
* @endinternal
*
* @brief  Configure Egress PCL Rule.
*/

GT_VOID prvTgfCfgProbePacketEgressPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                         rc;
    GT_U32                            ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat;
    CPSS_DXCH_CFG_PROBE_PACKET_STC   probeCfg;

    isEgress = GT_TRUE;

    /* AUTODOC: init PCL Engine for sending to port 1 */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: add PCL rule 0 with following parameters: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd DROP_HARD */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfDaMacArr[0], sizeof(prvTgfDaMacArr[0]));

    ruleIndex                   = 0;
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    action.sourceId.assignSourceId = GT_TRUE;
    action.sourceId.sourceIdValue = 1;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);

    rc = prvTgfPclSourceIdMaskSet(CPSS_PCL_LOOKUP_0_E, SOURCE_ID_MASK_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d",
                                 prvTgfDevNum);

    action.mirror.cpuCode        = CPSS_NET_FIRST_USER_DEFINED_E;

    rc = prvTgfPclRuleActionUpdate(
        CPSS_PCL_RULE_SIZE_STD_E,
        ruleIndex, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleActionUpdate",prvTgfDevNum);

    /* AUTODOC: Set the probe packet configuration. */
    probeCfg.bitLocation = 0;
    probeCfg.enable      = GT_TRUE;

    rc = cpssDxChCfgProbePacketConfigSet(prvTgfDevNum, &probeCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgProbePacketConfigSet: %d",prvTgfDevNum);

    /* PCL Configuration */

    /* init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_RCV1_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* set PCL rule 0 - */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                                    = 3;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    action.pktCmd                                = CPSS_PACKET_CMD_DROP_HARD_E;
    action.egressPolicy                          = GT_TRUE;
    action.redirect.redirectCmd                  = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VID_E;
    action.sourceId.assignSourceId               = GT_TRUE;
    action.sourceId.sourceIdValue                = SOURCE_ID_CNS;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
    rc = prvTgfPclSourceIdMaskSet(CPSS_PCL_LOOKUP_0_E, SOURCE_ID_MASK_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d",
                                 prvTgfDevNum);

    action.mirror.cpuCode        = CPSS_NET_FIRST_USER_DEFINED_E;
    rc = prvTgfPclRuleActionUpdate(
        CPSS_PCL_RULE_SIZE_STD_E,
        ruleIndex, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleActionUpdate",prvTgfDevNum);
}

/*******************************************************************************
* prvTgfCfgProbePacketTrafficGenerate
*
* DESCRIPTION:
*       Generate traffic
*******************************************************************************/
GT_VOID prvTgfCfgProbePacketTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portIter, sendIter = 0;
    CPSS_NET_RX_CPU_CODE_ENT dropCode;
    CPSS_DIRECTION_ENT       direction;
    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* set DA */
    cpssOsMemCpy(prvTgfPacket1L2Part.daMac, prvTgfDaMacArr[0], sizeof(prvTgfDaMacArr[sendIter]));

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacket1Info, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("------- Sending packet %d to port [%d] -------\n",
            sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: send 2 packets on port 1 with: */
    /* AUTODOC:   1 packet - DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   2 packet - DA=00:00:00:00:34:04, SA=00:00:00:00:00:11 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* check counter of FDB port 1 */
        rc = prvTgfCfgProbePacketCountersEthCheck(
            prvTgfPortsArray[portIter],
            prvTgfPacketsCountRxArray[sendIter][portIter],
            prvTgfPacketsCountTxArray[sendIter][portIter],
            prvTgfPacket1Info.totalLen, /* all packets MUST have the same totalLen */
            prvTgfBurstCount);
    }

    /* AUTODOC: Verify the Probe Packet Drop Code at the ingress*/

    if(isEgress)
        direction = CPSS_DIRECTION_EGRESS_E;
    else
        direction = CPSS_DIRECTION_INGRESS_E;

    rc = cpssDxChCfgProbePacketDropCodeGet(prvTgfDevNum,direction,&dropCode);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,rc,"cpssDxChCfgProbePacketDropCodeGet: %d",prvTgfDevNum);
    PRV_UTF_LOG1_MAC("Last Dropped Probe Packet Drop Code: %d \n",dropCode);
    UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E, dropCode, "prvTgfCfgProbePacketDropCodeGet: %d",prvTgfDevNum);
}

/**
* @internal prvTgfCfgProbePacketConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCfgProbePacketConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    CPSS_DXCH_CFG_PROBE_PACKET_STC probeCfg;

    probeCfg.bitLocation = 0;
    probeCfg.enable      = GT_FALSE;
    isEgress             = GT_FALSE;
    /* AUTODOC: RESTORE CONFIGURATION: */

    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 3, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    rc = cpssDxChCfgProbePacketConfigSet(prvTgfDevNum, &probeCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCfgProbePacketConfigSet: %d", prvTgfDevNum);
}
