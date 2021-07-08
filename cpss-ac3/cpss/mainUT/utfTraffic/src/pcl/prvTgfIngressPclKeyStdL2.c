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
* @file prvTgfIngressPclKeyStdL2.c
*
* @brief Ingress PCL key STD L2
*
* @version   17
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfIngressPclKeyStdL2.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS        2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   1

/* port number to forward packet 1 to */
#define PRV_TGF_FDB_PORT_1_IDX_CNS  2

/* port number to forward packet 2 to */
#define PRV_TGF_FDB_PORT_2_IDX_CNS  3

/* default VLAN ID */
#define PRV_TGF_VLANID_CNS          5

/* default number of packets to send */
static GT_U32                       prvTgfBurstCount = 1;

/****************************** ARP packet ************************************/

/* array of destination MACs */
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

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

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
* @internal prvTgfCountersEthCheck function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCountersEthCheck
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

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG1_MAC("\nVLAN %d CONFIGURATION:\n", PRV_TGF_VLANID_CNS);
    PRV_UTF_LOG4_MAC("  Port members: [%d], [%d], [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1],
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("FDB CONFIGURATION:\n");
    PRV_UTF_LOG1_MAC("  Forwarding packet 1 to port [%d]\n",
            prvTgfPortsArray[PRV_TGF_FDB_PORT_1_IDX_CNS]);
    PRV_UTF_LOG1_MAC("  Forwarding packet 2 to port [%d]\n",
            prvTgfPortsArray[PRV_TGF_FDB_PORT_2_IDX_CNS]);

    PRV_UTF_LOG0_MAC("PCL CONFIGURATION:\n");
    PRV_UTF_LOG0_MAC("  DROP_HARD packet 1\n");

    PRV_UTF_LOG0_MAC("\nTRAFFIC GENERATION:\n");
    PRV_UTF_LOG1_MAC("  Sending packet 1 and packet 2 to port [%d]\n",
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    PRV_UTF_LOG0_MAC("EXPECTED RESULTS:\n");
    PRV_UTF_LOG1_MAC("  packet 1 does not reach port [%d]\n",
            prvTgfPortsArray[PRV_TGF_FDB_PORT_1_IDX_CNS]);
    PRV_UTF_LOG1_MAC("  packet 2 reaches port [%d]\n\n",
            prvTgfPortsArray[PRV_TGF_FDB_PORT_2_IDX_CNS]);

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 5 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfDaMacArr[0], PRV_TGF_VLANID_CNS, prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_FDB_PORT_1_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:04, VLAN 5, port 3 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfDaMacArr[1], PRV_TGF_VLANID_CNS, prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_FDB_PORT_2_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclConfigurationSet
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

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);
}

/*******************************************************************************
* prvTgfTrafficGenerate
*
* DESCRIPTION:
*       Generate traffic
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_U32  pclTestNumOfPackets = PRV_TGF_MAX_ITER_CNS;
GT_VOID prvTgfTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portIter, sendIter = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* iterate thru stages: */
    for (sendIter = 0; sendIter < pclTestNumOfPackets; sendIter++)
    {
        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* set DA */
        cpssOsMemCpy(prvTgfPacket1L2Part.daMac, prvTgfDaMacArr[sendIter], sizeof(prvTgfDaMacArr[sendIter]));

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

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 packet - dropped */
        /* AUTODOC:   2 packet - forwarded on port 3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* check counter of FDB port 1 */
            rc = prvTgfCountersEthCheck(
                prvTgfPortsArray[portIter],
                prvTgfPacketsCountRxArray[sendIter][portIter],
                prvTgfPacketsCountTxArray[sendIter][portIter],
                prvTgfPacket1Info.totalLen, /* all packets MUST have the same totalLen */
                prvTgfBurstCount);
        }
    }
}

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable ingress policy on port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}


