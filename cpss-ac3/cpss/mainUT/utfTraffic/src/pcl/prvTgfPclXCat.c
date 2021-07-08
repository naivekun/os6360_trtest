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
* @file prvTgfPclXCat.c
*
* @brief XCAT A1 specific PCL features testing
*
* @version   4
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
#include <pcl/prvTgfPclXCat.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

#ifdef CHX_FAMILY

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

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

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadPart) + TGF_CRC_LEN_CNS),       /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/********************************************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket2L2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* srcMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* dstMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket2VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, 1                                 /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPacket2PayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPacket2PayloadDataArr),            /* dataLength */
    prvTgfPacket2PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacket2L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacket2VlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacket2PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacket2Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + sizeof(prvTgfPacket2PayloadPart) + TGF_CRC_LEN_CNS),       /* totalLen */
    sizeof(prvTgfPacket2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket2PartArray                                        /* partsArray */
};

/**
* @internal prvTgfPclXCatStdUdbTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclXCatStdUdbTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ruleIndex = 0;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           fieldIndex;
    TGF_PACKET_STC                   *packet1InfoPtr;
    TGF_PACKET_STC                   *packet2InfoPtr;
    CPSS_PORT_MAC_COUNTER_SET_STC    portCntrs;

    /* to prevent compiler warning */
    packet1InfoPtr = NULL;
    packet2InfoPtr = NULL;

    /* Init and configure all needed per port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* action deny */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;

    for (fieldIndex = 0; (fieldIndex < 1); fieldIndex++)
    {

        cpssOsMemSet(&mask, 0, sizeof(mask));
        cpssOsMemSet(&pattern, 0, sizeof(pattern));

        switch (fieldIndex)
        {
            case 0:
                /* IsIP field */
                mask.ruleStdUdb.commonUdb.isIp    = 1;
                pattern.ruleStdUdb.commonUdb.isIp = 1;
                packet1InfoPtr = &prvTgfPacket1Info;
                packet2InfoPtr = &prvTgfPacket2Info;
                break;
            default:
                break;
        }


        rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
            ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
            prvTgfDevNum, ruleIndex, fieldIndex);

        rc = prvTgfResetCountersEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

        /* Packet to be matched */

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, packet1InfoPtr, 1 /*prvTgfBurstCount*/,
            0 /*numVfd*/, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC(
            "------- Sending matched packet to port [%d] -------\n",
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* Packet to be not matched */

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, packet2InfoPtr, 1 /*prvTgfBurstCount*/,
            0 /*numVfd*/, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC(
            "------- Sending matched packet to port [%d] -------\n",
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


        /* read counters */
        rc = prvTgfReadPortCountersEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            1, portCntrs.goodPktsRcv.l[0],
            "goodPktsRcv received %d packets but expected %d",
            portCntrs.goodPktsRcv.l[0], 1);
    }

    /* disable ingress policy on ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfPclXCatDummy function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclXCatDummy
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("***<<< tgfPclXCatDummy >>>***\n");

}

#endif /*CHX_FAMILY*/


