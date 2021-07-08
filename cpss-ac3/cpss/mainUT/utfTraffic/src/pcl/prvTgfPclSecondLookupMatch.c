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
* @file prvTgfPclSecondLookupMatch.c
*
* @brief Second Lookup match
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclSecondLookupMatch.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS              2

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                5

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            PRV_TGF_PORTS_NUM_CNS

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_1_IDX_CNS       0

/* port 1 number to forward traffic to */
#define PRV_TGF_FDB_PORT_1_IDX_CNS        2

/* port 2 number to forward traffic to */
#define PRV_TGF_FDB_PORT_2_IDX_CNS        3

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/******************************* IPv4 packet 1 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C9D,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
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
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/******************************* IPv4 packet 2 **********************************/

/* L2 part of packet2 */
static TGF_PACKET_L2_STC prvTgfPacket2L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x04},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};
/* ethertype part of packet2 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket2EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* IPv4 part of packet2 */
static TGF_PACKET_IPV4_STC prvTgfPacket2Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C9D,             /* csum */
    { 22, 22, 22, 22},  /* srcAddr */
    { 1,   1,  1,  3}    /* dstAddr */
};
/* DATA of packet2 */
static GT_U8 prvTgfPacket2PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part of packet2 */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPacket2PayloadDataArr),                 /* dataLength */
    prvTgfPacket2PayloadDataArr                          /* dataPtr */
};
/* PARTS of packet2 */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket2L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket2EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket2Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket2PayloadPart}
};

/************** packets and ations for the each iteration ****************/

/* The names of each iteration */
static GT_U8 prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "Check PCL rule 0: FORWARD packet 1 according to FDB to port 18",
    "Check PCL rule 1: DROP_HARD packet 2"
};

/* Packets for the each iteration */
static TGF_PACKET_PART_STC *prvTgfPacketsArray[] = {
    prvTgfPacket1PartArray,
    prvTgfPacket2PartArray
};

/* PartsCount of Packets for the each iteration */
static GT_U32 prvTgfPartsCountArray[] = {
    sizeof(prvTgfPacket1PartArray) / sizeof(prvTgfPacket1PartArray[0]),
    sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0])
};

/* Send port for the each iteration */
static GT_U8 prvTgfSendPortIdxArray[] = {
    PRV_TGF_SEND_PORT_1_IDX_CNS,
    PRV_TGF_SEND_PORT_1_IDX_CNS
};

/* Enable to reset counters before the iteration */
static GT_BOOL prvTgfActionContersReset[] = {
    GT_TRUE, GT_TRUE
};

/* Checking ethernet counters */
static GT_BOOL prvTgfActionEthContersCheck[] = {
    GT_TRUE, GT_TRUE
};

/* define expected number of received packets (Sent packets) */
static GT_U8 prvTgfPacketsCountRxArray[][PRV_TGF_PORT_COUNT_CNS] = {
/* ports: SEND1 SEND2 FDB CPU */
             {1,  0,  0,  0},
             {1,  0,  0,  0}
};

/* define expected number of transmitted packets */
static GT_U8 prvTgfPacketsCountTxArray[][PRV_TGF_PORT_COUNT_CNS] = {
/* ports: SEND1 SEND2 FDB CPU */
             {1,  0,  1,  0},
             {1,  0,  0,  0}
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/**
* @internal prvTgfPclSecondLookupMatchConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclSecondLookupMatchConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum, prvTgfPortsArray[PRV_TGF_FDB_PORT_1_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:04, VLAN 5, port 3 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacket2L2Part.daMac, PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum, prvTgfPortsArray[PRV_TGF_FDB_PORT_2_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS]);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfPclSecondLookupMatchPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclSecondLookupMatchPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, second lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass ?
                                CPSS_PCL_LOOKUP_0_1_E :
                                CPSS_PCL_LOOKUP_1_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS]);

    /* set PCL rule 0 - FORWARD packet 1 according to FDB to port 18 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask.ruleStdIpL2Qos.macDa, 0xFF, sizeof(mask.ruleStdIpL2Qos.macDa));
    cpssOsMemCpy(&pattern.ruleStdIpL2Qos.macDa, prvTgfPacket1L2Part.daMac, sizeof(prvTgfPacket1L2Part.daMac));

    ruleIndex     = 0;
    ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    /* AUTODOC: set PCL rule 0 - FORWARD IP packet with DA=00:00:00:00:34:02 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

    /* set PCL rule 1 - DROP_HARD packet 2 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask.ruleStdIpL2Qos.macDa, 0xFF, sizeof(mask.ruleStdIpL2Qos.macDa));
    cpssOsMemCpy(&pattern.ruleStdIpL2Qos.macDa, prvTgfPacket2L2Part.daMac, sizeof(prvTgfPacket2L2Part.daMac));

    ruleIndex     = 1;
    ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;

    /* AUTODOC: set PCL rule 0 - DROP_HARD IP packet with DA=00:00:00:00:34:04 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclSecondLookupMatchTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPclSecondLookupMatchTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          sendIter    = 0;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          packetsCountRx;
    GT_U32                          packetsCountTx;
    GT_BOOL                         isCountersOk;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* make iteration */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC("======= Generating Traffic %d =======\n", sendIter);
        PRV_UTF_LOG1_MAC("%s\n", prvTgfIterationNameArray[sendIter]);

        /* -------------------------------------------------------------------------
         * 1. Reseting all counters and clearing the capturing table
         */

        if (GT_TRUE == prvTgfActionContersReset[sendIter])
        {
            /* PRV_UTF_LOG0_MAC("------- Reseting All Counters -------\n"); */

            for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
            {
                /* reset ethernet counters */
                rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                             prvTgfDevNum, prvTgfPortsArray[portIter]);
            }

            /* Clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
        }

        /* -------------------------------------------------------------------------
         * 2. Generating Traffic
         */

        /* number of parts in packet */
        partsCount = prvTgfPartsCountArray[sendIter];

        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfPacketsArray[sendIter], partsCount, &packetSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* build packet */
        packetInfo.totalLen   = packetSize;
        packetInfo.numOfParts = partsCount;
        packetInfo.partsArray = prvTgfPacketsArray[sendIter];

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* AUTODOC: send 2 IPv4 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:55 */
        /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
        /* AUTODOC:   DA=00:00:00:00:34:04, SA=00:00:00:00:00:11 */
        /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdxArray[sendIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdxArray[sendIter]]);

        /* -------------------------------------------------------------------------
         * 3. Checking Ethernet Counters
         */

        if (GT_TRUE == prvTgfActionEthContersCheck[sendIter])
        {
            /* PRV_UTF_LOG0_MAC("------- Checking Ethernet Counters -------\n"); */

            /* read and check ethernet counters */
            for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
            {
                /* read counters */
                rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                             prvTgfDevNum, prvTgfPortsArray[portIter]);

                /* calculate expected numder of pakets */
                packetsCountRx = prvTgfBurstCount * prvTgfPacketsCountRxArray[sendIter][portIter];
                packetsCountTx = prvTgfBurstCount * prvTgfPacketsCountTxArray[sendIter][portIter];

                cpssOsMemSet(&expectedCntrs, 0, sizeof(expectedCntrs));
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * packetsCountTx;
                expectedCntrs.goodPktsSent.l[0]   = packetsCountTx;
                expectedCntrs.ucPktsSent.l[0]     = packetsCountTx;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * packetsCountRx;
                expectedCntrs.goodPktsRcv.l[0]    = packetsCountRx;
                expectedCntrs.ucPktsRcv.l[0]      = packetsCountRx;

                /* AUTODOC: verify traffic: */
                /* AUTODOC:   1 IPv4 packet - forwarded to port 2 */
                /* AUTODOC:   2 IPv4 packet - dropped */
                PRV_TGF_VERIFY_COUNTERS_MAC(isCountersOk, expectedCntrs, portCntrs);

                /* print expected values if bug */
                PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isCountersOk, expectedCntrs, portCntrs);
            }
        }
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclSecondLookupMatchConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore PCL Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfPclSecondLookupMatchConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 2. Restore PCL Configuration
     */

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL Rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL Rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 1, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

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


