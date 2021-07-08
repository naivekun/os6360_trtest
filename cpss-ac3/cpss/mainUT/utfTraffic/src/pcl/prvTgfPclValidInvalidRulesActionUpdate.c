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
* @file prvTgfPclValidInvalidRulesActionUpdate.c
*
* @brief Ingress PCL key STD L2
*
* @version   21
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
#include <pcl/prvTgfPclValidInvalidRulesActionUpdate.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS              6

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                5

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            PRV_TGF_PORTS_NUM_CNS

/* port 1 to send traffic to */
#define PRV_TGF_SEND_PORT_1_IDX_CNS       0

/* port 2 to send traffic to */
#define PRV_TGF_SEND_PORT_2_IDX_CNS       1

/* port number to forward traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          2

/* analyzer port number */
#define PRV_TGF_CPU_PORT_IDX_CNS          3

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/******************************* IPv4 packet 1 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                             /* pri, cfi, VlanId */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x87,               /* protocol 135 */
    0x4B49,             /* csum */
    {22,  1, 23,  1},   /* srcAddr */
    { 1,  2,  1,  1}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};
/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/******************************* IPv4 packet 2 **********************************/

/* L2 part of packet2 */
static TGF_PACKET_L2_STC prvTgfPacket2L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                 /* saMac */
};
/* VLAN_TAG part of packet2 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket2VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                             /* pri, cfi, VlanId */
};
/* ethertype part of packet2 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket2EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* IPv4 part of packet2 */
static TGF_PACKET_IPV4_STC prvTgfPacket2Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x86,               /* protocol 134 */
    0x7648,             /* csum */
    { 1,  1,  0,  2},   /* srcAddr */
    { 3,  3,  0,  1}    /* dstAddr */
};
/* DATA of packet2 */
static GT_U8 prvTgfPacket2PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15
};
/* PAYLOAD part of packet2 */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPacket2PayloadDataArr),                 /* dataLength */
    prvTgfPacket2PayloadDataArr                          /* dataPtr */
};
/* PARTS of packet2 */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket2L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket2VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket2EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket2Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket2PayloadPart}
};

/************** packets and ations for the each iteration ****************/

/* The names of each iteration */
static GT_U8 prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "Check PCL rule 0: TRAP_TO_CPU any Ipv4 with ipProtocol = 135 (0x87)",
    "Check PCL rule 1: REDIRECT to port 23 any Not Ipv6 with ipProtocol = 134 (0x86)",
    "Invalidate PCL rule 0, FORWARD packet to port 18",
    "Invalidate PCL rule 1, FORWARD packet to port 18",
    "Update PCL Action 0: REDIRECT to port 8",
    "Update PCL Action 1: REDIRECT to port 23 and TRAP_TO_CPU"
};

/* Packets for the each iteration */
static TGF_PACKET_PART_STC *prvTgfPacketsArray[] = {
    prvTgfPacket1PartArray,
    prvTgfPacket2PartArray,
    prvTgfPacket1PartArray,
    prvTgfPacket2PartArray,
    prvTgfPacket1PartArray,
    prvTgfPacket2PartArray
};

/* PartsCount of Packets for the each iteration */
static GT_U32 prvTgfPartsCountArray[] = {
    sizeof(prvTgfPacket1PartArray) / sizeof(prvTgfPacket1PartArray[0]),
    sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]),
    sizeof(prvTgfPacket1PartArray) / sizeof(prvTgfPacket1PartArray[0]),
    sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]),
    sizeof(prvTgfPacket1PartArray) / sizeof(prvTgfPacket1PartArray[0]),
    sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0])
};

/* Send port for the each iteration */
static GT_U8 prvTgfSendPortIdxArray[] = {
    PRV_TGF_SEND_PORT_1_IDX_CNS,
    PRV_TGF_SEND_PORT_2_IDX_CNS,
    PRV_TGF_SEND_PORT_1_IDX_CNS,
    PRV_TGF_SEND_PORT_2_IDX_CNS,
    PRV_TGF_SEND_PORT_1_IDX_CNS,
    PRV_TGF_SEND_PORT_2_IDX_CNS
};

/* Invalidating PCL Rules before the iteration */
static GT_BOOL prvTgfActionPclRulesInvalidate[] = {
    GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE, GT_FALSE
};

/* Validating PCL Rules before the iteration */
static GT_BOOL prvTgfActionPclRulesValidate[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE
};

/* Updating PCL Action before the iteration */
static GT_BOOL prvTgfActionPclActionUpdate[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE
};

/* Enable to reset counters before the iteration */
static GT_BOOL prvTgfActionContersReset[] = {
    GT_TRUE, GT_TRUE, GT_TRUE, GT_FALSE, GT_TRUE, GT_FALSE
};

/* Checking ethernet counters */
static GT_BOOL prvTgfActionEthContersCheck[] = {
    GT_TRUE, GT_TRUE, GT_FALSE, GT_TRUE, GT_FALSE, GT_TRUE
};

/* define expected number of received packets (Sent packets) */
static GT_U8 prvTgfPacketsCountRxArray[][PRV_TGF_PORT_COUNT_CNS] = {
/* ports: SEND1 SEND2 FDB CPU */
             {1,  0,  0,  0},
             {0,  1,  0,  1},
             {0,  0,  0,  0},
             {1,  1,  0,  0},
             {0,  0,  0,  0},
             {1,  1,  0,  0}
};

/* define expected number of transmitted packets */
static GT_U8 prvTgfPacketsCountTxArray[][PRV_TGF_PORT_COUNT_CNS] = {
/* ports: SEND1 SEND2 FDB CPU */
             {1,  0,  0,  0},
             {0,  1,  0,  1},
             {0,  0,  0,  0},
             {1,  1,  2,  0},
             {0,  0,  0,  0},
             {1,  2,  0,  0}
};

/* Checking trapped packets to CPU */
static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_TRUE, GT_TRUE, GT_FALSE, GT_TRUE, GT_FALSE, GT_TRUE
};

/* Checking triggers */
static GT_BOOL prvTgfActionTriggersCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/**
* @internal prvTgfPclValidInvalidRulesActionUpdateConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdateConfigurationSet
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
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
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
* @internal prvTgfPclValidInvalidRulesActionUpdatePclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdatePclConfigurationSet
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
    /* AUTODOC:   ingress direction */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS]);

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_2_IDX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_2_IDX_CNS]);

    /* set PCL rule 0 - CMD_TRAP_TO_CPU any Ipv4 with ipProtocol = 135 (0x87) */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
    mask.ruleStdIpv4L4.common.isIp               = GT_TRUE;
    mask.ruleStdIpv4L4.common.isIpv4             = GT_TRUE;
    mask.ruleStdIpv4L4.commonStdIp.ipProtocol    = 255;
    pattern.ruleStdIpv4L4.common.isIp            = GT_TRUE;
    pattern.ruleStdIpv4L4.common.isIpv4          = GT_TRUE;
    pattern.ruleStdIpv4L4.commonStdIp.ipProtocol = 135;
    action.pktCmd                                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode                        = CPSS_NET_CLASS_KEY_MIRROR_E;

    /* AUTODOC: set PCL rule 0 - CMD_TRAP_TO_CPU any IPv4 with ipProtocol=135 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

    /* set PCL rule 1 - REDIRECT to port 23 any Not Ipv6 with ipProtocol = 134 (0x86) */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                                               = 1;
    ruleFormat                                              = PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
    mask.ruleExtNotIpv6.common.isIp                         = GT_TRUE;
    mask.ruleExtNotIpv6.commonExt.isIpv6                    = GT_TRUE;
    mask.ruleExtNotIpv6.commonExt.ipProtocol                = 255;
    pattern.ruleExtNotIpv6.common.isIp                      = GT_TRUE;
    pattern.ruleExtNotIpv6.commonExt.ipProtocol             = 134;

    action.pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPU_PORT_IDX_CNS];

    /* AUTODOC: set PCL rule 1 - REDIRECT to port 3 any NOT_IPv6 with ipProtocol=134 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclValidInvalidRulesActionUpdateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdateTrafficGenerate
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
    GT_U32                          numTriggers = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          packetsCountRx;
    GT_U32                          packetsCountTx;
    GT_BOOL                         isCountersOk;
    PRV_TGF_PCL_ACTION_STC          pclAction;

    /* AUTODOC: GENERATE TRAFFIC: */
    /* -------------------------------------------------------------------------
     * 0. Setup Capturing
     */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable capture on CpuPort */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPU_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CPU_PORT_IDX_CNS]);

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxModeSet: %d", prvTgfDevNum);

    /* set Cpu Device for TRAP to CPU action */
    rc = tgfTrafficGeneratorCpuDeviceSet(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorCpuDeviceSet: %d", prvTgfDevNum);

    /* AUTODOC: Iterate thru 6 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC("======= Generating Traffic %d =======\n", sendIter);
        PRV_UTF_LOG1_MAC("%s\n", prvTgfIterationNameArray[sendIter]);

        /* -------------------------------------------------------------------------
         * 1. Invalidating, Validating or Updating PCL Rules
         */

        /* AUTODOC: invalidate PCL Rules 0, 1 on Stage#3 */
        if (GT_TRUE == prvTgfActionPclRulesInvalidate[sendIter])
        {
            PRV_UTF_LOG0_MAC("------- Invalidating PCL Rules -------\n");

            rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

            rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E, 1, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);
        }

        /* AUTODOC: validate PCL Rules 0, 1 on Stage#5 */
        if (GT_TRUE == prvTgfActionPclRulesValidate[sendIter])
        {
            PRV_UTF_LOG0_MAC("------- Validating PCL Rules -------\n");

            rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

            rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E, 1, GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);
        }

        /* AUTODOC: update PCL Actions for Rules 0,1 on Stage#5: */
        if (GT_TRUE == prvTgfActionPclActionUpdate[sendIter])
        {
            PRV_UTF_LOG0_MAC("------- Updating PCL Actions -------\n");

            /* Update PCL Action 0: REDIRECT to port 8 any matched with current mask and pattern */
            cpssOsMemSet(&pclAction, 0, sizeof(pclAction));
            pclAction.pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
            pclAction.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            pclAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
            pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
            pclAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_2_IDX_CNS];

            /* AUTODOC:   update PCL Action 0: update cmd to REDIRECT to port 1 */
            rc = prvTgfPclRuleActionUpdate(CPSS_PCL_RULE_SIZE_STD_E, 0, &pclAction);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleActionUpdate: %d", prvTgfDevNum);

            /* Update PCL Action 1: REDIRECT to port 23 and TRAP_TO_CPU any matched with current mask and pattern */
            cpssOsMemSet(&pclAction, 0, sizeof(pclAction));

            pclAction.pktCmd                                           = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            pclAction.mirror.cpuCode                                   = CPSS_NET_CLASS_KEY_MIRROR_E;
            pclAction.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            pclAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
            pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
            pclAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPU_PORT_IDX_CNS];

            /* AUTODOC:   update PCL Action 1: update cmd to TRAP_TO_CPU */
            rc = prvTgfPclRuleActionUpdate(CPSS_PCL_RULE_SIZE_EXT_E, 1, &pclAction);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleActionUpdate: %d", prvTgfDevNum);
        }

        /* -------------------------------------------------------------------------
         * 2. Reseting all counters and clearing the capturing table
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
         * 3. Generating Traffic
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

        /* AUTODOC: for Stages #1, #3, #5 send IPv4 packet on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:55, VID 5 */
        /* AUTODOC:   srcIP=22.1.23.1, dstIP=1.2.1.1 */
        /* AUTODOC: for Stages #2, #4, #6 send IPv4 packet on port 1 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:02, VID 5 */
        /* AUTODOC:   srcIP=1.1.0.2, dstIP=3.3.0.1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdxArray[sendIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdxArray[sendIter]]);

        /* -------------------------------------------------------------------------
         * 4. Checking Ethernet Counters
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
                /* AUTODOC:   Stage#1 - IPv4 packet trapped to CPU */
                /* AUTODOC:   Stage#2 - IPv4 packet redirected on port 3 */
                /* AUTODOC:   Stages #3, #4 - 2 IPv4 packets forwarded on port 2 */
                /* AUTODOC:   Stages #5, #6 - 1 IPv4 packet redirected on port 1 */
                /* AUTODOC:                   2 IPv4 packet trapped to CPU */
                PRV_TGF_VERIFY_COUNTERS_MAC(isCountersOk, expectedCntrs, portCntrs);

                /* print expected values if bug */
                PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isCountersOk, expectedCntrs, portCntrs);
            }
        }

        /* -------------------------------------------------------------------------
         * 5. Checking trapped packets to CPU
         */

        if (GT_TRUE == prvTgfActionTrapToCpuCheck[sendIter])
        {
            GT_U8                 packetBufPtr[64] = {0};
            GT_U32                packetBufLen = 64;
            GT_U32                packetLen;
            GT_U8                 devNum;
            GT_U8                 queueCpu;
            TGF_NET_DSA_STC       rxParams;

            PRV_UTF_LOG0_MAC("CPU port capturing:\n");

            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE,
                                               packetBufPtr,
                                               &packetBufLen,
                                               &packetLen,
                                               &devNum,
                                               &queueCpu,
                                               &rxParams);
        }

        /* -------------------------------------------------------------------------
         * 6. Checking triggers
         */

        if (GT_TRUE == prvTgfActionTriggersCheck[sendIter])
        {
            /* get and check trigger counters */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 0;
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacket1L2Part.daMac, sizeof(TGF_MAC_ADDR));

            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);

            PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            rc = (rc == GT_NO_MORE) ? GT_OK : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
        }
    }
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CPU_PORT_IDX_CNS]);

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

/**
* @internal prvTgfPclValidInvalidRulesActionUpdateConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore PCL Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdateConfigurationRestore
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
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_1_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: disable ingress policy on port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_2_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL Rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL Rule 1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E, 1, GT_FALSE);
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


