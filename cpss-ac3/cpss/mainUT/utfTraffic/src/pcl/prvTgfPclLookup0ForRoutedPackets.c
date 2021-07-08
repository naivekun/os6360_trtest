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
* @file prvTgfPclLookup0ForRoutedPackets.c
*
* @brief Enable/disable PCL lookup 0 for routed packets.
*
* @version   3
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
#include <pcl/prvTgfPclLookup0ForRoutedPackets.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* VLAN Id */
#define PRV_TGF_VLANID_CNS                5

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            PRV_TGF_PORTS_NUM_CNS

/* port to send traffic to */
#define PRV_TGF_TX_PORT_IDX_CNS           0

/* port number to forward traffic to */
#define PRV_TGF_RX_PORT_IDX_CNS           3

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS       1

/* PCL Ids */
#define PRV_TGF_PCL_ID_FOR_LOOKUP_0       0
#define PRV_TGF_PCL_ID_FOR_LOOKUP_1       1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/******************************* IPv4 packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* ethertype part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
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
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/****************** packets and actions for each iteration ********************/

/* define expected number of transmitted packets when lookup 0 is enabled (drop) */
static GT_U8 prvTgfPacketsCountDropTxArray[PRV_TGF_PORT_COUNT_CNS] = {1,  0,  0,  0};

/* define expected number of received packets when lookup 0 is enabled (drop) */
static GT_U8 prvTgfPacketsCountDropRxArray[PRV_TGF_PORT_COUNT_CNS] = {1,  0,  0,  0};

/* define expected number of transmitted packets when lookup 0 is disabled (forward) */
static GT_U8 prvTgfPacketsCountForwardTxArray[PRV_TGF_PORT_COUNT_CNS] = {1,  0,  0,  1};

/* define expected number of received packets when lookup 0 is disabled (forward) */
static GT_U8 prvTgfPacketsCountForwardRxArray[PRV_TGF_PORT_COUNT_CNS] = {1,  0,  0,  1};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/**
* @internal prvTgfPclLookup0ForRoutedPacketsConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_TTI_MAC_VLAN_STC    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC    macToMeMask;
    GT_STATUS                   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 2 */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
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

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 3 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);

    /* set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xff,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xfff;

    /* AUTODOC: config MAC to ME table: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");
}

/**
* @internal prvTgfPclLookup0ForRoutedPacketsMatchConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsMatchConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt1(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_0_0_E,
                                  PRV_TGF_PCL_ID_FOR_LOOKUP_0,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, second lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt1(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_0_1_E,
                                  PRV_TGF_PCL_ID_FOR_LOOKUP_1,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* set PCL rule 0 - FORWARD packet according to FDB to port 18 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    pattern.ruleStdIpv4L4.common.pclId = PRV_TGF_PCL_ID_FOR_LOOKUP_0;
    mask.ruleStdIpv4L4.common.pclId = 0x3ff;

    ruleIndex     = 0;
    ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;

    /* AUTODOC: set PCL rule 0 - FORWARD IP packet with DA=00:00:00:00:34:02 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

    pattern.ruleStdIpv4L4.common.pclId = PRV_TGF_PCL_ID_FOR_LOOKUP_1;
    mask.ruleStdIpv4L4.common.pclId = 0x3ff;
    ruleIndex     = 1;
    ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    /* AUTODOC: set PCL rule 1 - DROP IP packet with DA=00:00:00:00:34:02 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclLookup0ForRoutedPacketsConfigurationEnableSet function
* @endinternal
*
* @brief   Enable/disable PCL lookup 0 for routed packets.
*
* @param[in] enable                   - GT_TRUE:   the PCL lookup0 for routed packets
*                                      - GT_FALSE: disable the PCL lookup0 for routed packets
*                                       None
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsConfigurationEnableSet
(
    IN GT_BOOL     enable
)
{
    GT_STATUS rc;

    rc = prvTgfPclLookup0ForRoutedPacketsEnableSet(prvTgfDevNum,
                                                   prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                   enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclLookup0ForRoutedPacketsEnableSet: %d, %d, %d",
                                 prvTgfDevNum, enable);
}

/**
* @internal prvTgfPclLookup0ForRoutedPacketsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect traffic
*                                      GT_FALSE: expect no traffic
*                                       None
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsTrafficGenerate
(
    GT_BOOL     expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          packetsCountRx;
    GT_U32                          packetsCountTx;
    GT_BOOL                         isCountersOk;
    CPSS_INTERFACE_INFO_STC         portInterface;

    /* AUTODOC: GENERATE TRAFFIC: */

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");
    PRV_UTF_LOG0_MAC("Check PCL lookup 0\n");

    /* -------------------------------------------------------------------------
     * 1. Reseting all counters and clearing the capturing table
     */

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

    /* AUTODOC: setup RX portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];

    /* AUTODOC: enable capture on RX port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

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

    /* AUTODOC: send an IPv4 packet on port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:55 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on RX port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Checking Ethernet Counters
     */

    /* AUTODOC: check traffic */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* calculate expected numder of pakets */
        if (expectTraffic == GT_TRUE)
        {
            packetsCountRx = prvTgfBurstCount * prvTgfPacketsCountForwardRxArray[portIter];
            packetsCountTx = prvTgfBurstCount * prvTgfPacketsCountForwardTxArray[portIter];
        }
        else
        {
            packetsCountRx = prvTgfBurstCount * prvTgfPacketsCountDropRxArray[portIter];
            packetsCountTx = prvTgfBurstCount * prvTgfPacketsCountDropTxArray[portIter];
        }

        cpssOsMemSet(&expectedCntrs, 0, sizeof(expectedCntrs));
        expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * packetsCountTx;
        expectedCntrs.goodPktsSent.l[0]   = packetsCountTx;
        expectedCntrs.ucPktsSent.l[0]     = packetsCountTx;
        expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * packetsCountRx;
        expectedCntrs.goodPktsRcv.l[0]    = packetsCountRx;
        expectedCntrs.ucPktsRcv.l[0]      = packetsCountRx;

        /* AUTODOC: verify traffic: */
        PRV_TGF_VERIFY_COUNTERS_MAC(isCountersOk, expectedCntrs, portCntrs);

        UTF_VERIFY_EQUAL0_STRING_MAC(isCountersOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isCountersOk, expectedCntrs, portCntrs);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclLookup0ForRoutedPacketsConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore PCL Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsConfigurationRestore
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
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL Rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL Rule 1 */
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


