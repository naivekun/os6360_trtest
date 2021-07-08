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
* @file prvTgfPclTcpRstFinMirroring.c
*
* @brief Specific PCL features testing
*
* @version   18
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
#include <common/tgfMirror.h>
#include <pcl/prvTgfPclTcpRstFinMirroring.h>

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to forward traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS      1

#define PRV_UTF_VERIFY_RC1(rc, name)                                         \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
        rc1 = rc;                                                            \
    }

/*************** packet 1 (IPv4 TCP not RST and not FIN) *******************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
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
    6,                  /* protocol */
    0x4C9B,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacket1TcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x00,               /* flags */
    4096,               /* window */
    0xC1A9,             /* csum */
    0                   /* urgent pointer */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
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
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket1TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/************************* packet 2 (not TCP) *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket2L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x04},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                 /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPacket2PayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPacket2PayloadDataArr),            /* dataLength */
    prvTgfPacket2PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacket2L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacket2PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacket2Info = {
    (TGF_L2_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket2PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket2PartArray                                        /* partsArray */
};

/*************** packet 3 (IPv4 TCP RST) *******************/

/* packet's TCP RST part */
static TGF_PACKET_TCP_STC prvTgfPacket3TcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x04,               /* flags */
    4096,               /* window */
    0xC1A5,             /* csum */
    0                   /* urgent pointer */
};
/* PARTS of packet IPv4 TCP RST */
static TGF_PACKET_PART_STC prvTgfPacket3PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket3TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET3 IPv4 TCP RST to send */
static TGF_PACKET_STC prvTgfPacket3Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket3PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket3PartArray                                        /* partsArray */
};

/*************** packet 4 (IPv4 TCP FIN) *******************/

/* packet's TCP FIN part */
static TGF_PACKET_TCP_STC prvTgfPacket4TcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x01,               /* flags */
    4096,               /* window */
    0xC1A8,             /* csum */
    0                   /* urgent pointer */
};
/* PARTS of packet IPv4 TCP FIN */
static TGF_PACKET_PART_STC prvTgfPacket4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket4TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET3 IPv4 TCP FIN to send */
static TGF_PACKET_STC prvTgfPacket4Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket4PartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* PCL rule indexes */
static GT_U32   prvTgfPclRuleIndex1 = 1;

static const GT_CHAR prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "sending 'Not TCP' packet (Not Matched) ...\n",
    "sending 'TCP' packet (Not Matched) ...\n",
    "sending 'TCP RST' packet (Matched) - MIRRORING to CPU port [18] ...\n",
    "sending 'TCP FIN' packet (Matched) - MIRRORING to CPU port [18] ...\n"
};
static TGF_PACKET_STC *prvTgfPacketInfoArray[] = {
    &prvTgfPacket2Info,
    &prvTgfPacket1Info,
    &prvTgfPacket3Info,
    &prvTgfPacket4Info
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfDefConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    return rc;
};

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: invalidate PCL Rule 1 */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_EXT_E, prvTgfPclRuleIndex1, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    sendIter;
    GT_U32    sendIterMax = sizeof(prvTgfPacketInfoArray)/sizeof(TGF_PACKET_STC *);

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG3_MAC("sending [%d] packets to port [%d] "
            "and FORWARDING to FDB port [%d]\n\n", sendIterMax,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS]);

    /* reset counters */
    rc = prvTgfCountersEthReset();
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersEthReset");

    /* sending packets - matched and unmatched */
    for (sendIter = 0; sendIter < sendIterMax; sendIter++)
    {
        PRV_UTF_LOG0_MAC(prvTgfIterationNameArray[sendIter]);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, prvTgfPacketInfoArray[sendIter],
            1 /*burstCount*/, 0 /*numVfd*/, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* AUTODOC: send Not TCP packet (Not Matched) on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:04, SA=00:00:00:00:00:33 */

        /* AUTODOC: send TCP packet (Not Matched) on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
        /* AUTODOC:   srcPort=1, dstPort=2, flags=0x00 */

        /* AUTODOC: send TCP RST packet (Matched) on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
        /* AUTODOC:   srcPort=1, dstPort=2, flags=0x04 */

        /* AUTODOC: send TCP FIN packet (Matched) on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
        /* AUTODOC:   srcPort=1, dstPort=2, flags=0x01 */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }

    return rc;
};

/**
* @internal prvTgfTrafficPclRulesSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficPclRulesSet
(
    IN GT_U32                           ruleIndex
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT patt;
    PRV_TGF_PCL_ACTION_STC      action;

    PRV_UTF_LOG0_MAC("------- action TCP RST/FIN mirroring TEST -------\n");

    /* clear mask, pattern and action */
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    /*action TCP RST/FIN mirroring*/
    action.pktCmd          = CPSS_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode  = CPSS_NET_TCP_RST_FIN_TRAP_E;
    action.mirror.mirrorTcpRstAndFinPacketsToCpu = GT_TRUE;

    /* AUTODOC: set PCL rule 1 - FORWARD, action TCP RST/FIN mirror for all packets */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        ruleIndex, &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    return rc;
};

/**
* @internal prvTgfPclTcpRstFinCpuRxCounterGet function
* @endinternal
*
* @brief   Count packets, received by CPU
*/
static GT_VOID prvTgfPclTcpRstFinCpuRxCounterGet
(
    IN  GT_U8          srcDev,
    IN  GT_U32          srcPort,
    OUT GT_U32         *packetsNumPtr
)
{
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_STATUS       rc       = GT_OK;
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    TGF_NET_DSA_STC rxParam = {0};
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = srcDev;
    portInterface.devPort.portNum = srcPort;

    *packetsNumPtr = 0;

    while (1)
    {
        /* disable packet trace */
        tgfTrafficTracePacketByteSet(GT_FALSE);

        buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;

        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
             TGF_PACKET_TYPE_REGULAR_E,
            getFirst, GT_TRUE, packetBuff,
            &buffLen, &packetActualLength,
            &devNum, &queue, &rxParam);
        if (rc != GT_OK)
        {
            if (rc != GT_NO_MORE)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(
                    GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");
            }
            break;
        }

        getFirst = GT_FALSE; /* now we get the next */

        /* calculate number of captured packets */
        (*packetsNumPtr)++;

        /* enable packet trace */
        tgfTrafficTracePacketByteSet(GT_TRUE);

        /* trace packet */
        tgfTrafficTracePacket(packetBuff, packetActualLength, GT_TRUE);
    }

    /* enable packet trace */
    tgfTrafficTracePacketByteSet(GT_TRUE);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclTcpRstFinMirroring function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclTcpRstFinMirroring
(
   GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           cpuPacketsNum;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    /* create vlan and ports */
    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfDefConfigurationSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:04, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket2L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Set PCL rules
     */

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* set PCL rule for TCP FIN - matches any packets */
    rc = prvTgfTrafficPclRulesSet(prvTgfPclRuleIndex1);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRulesSet: %d", prvTgfDevNum);


    /* -------------------------------------------------------------------------
     * 3. Generate traffic and Check counters
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* generate traffic */
    rc = prvTgfTrafficGenerate();

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: check counter of FDB port 1 - 4 egress packets expected */
    rc = prvTgfEthCountersCheck(
        prvTgfDevsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        0 /*ingress packets*/, 4/*egress packets*/,
        prvTgfPacketInfoArray[0]->totalLen, 1);

    /* AUTODOC: check CPU Rx packets - 2 packets expected */
    prvTgfPclTcpRstFinCpuRxCounterGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &cpuPacketsNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        2, cpuPacketsNum, "CPU RX packets amount");

    /* -------------------------------------------------------------------------
     * 4. Restore configuration
     */
    rc = prvTgfConfigurationRestore();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);
}



