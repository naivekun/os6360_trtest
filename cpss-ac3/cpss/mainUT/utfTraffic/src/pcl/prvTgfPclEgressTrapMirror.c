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
* @file prvTgfPclEgressTrapMirror.c
*
* @brief Pcl Egress Mirror/Trap packet cmd tests
*
* @version
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   2;

/* default vlanid */
static GT_U16        prvTgfDefVlanId  =   1;

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};

/*Reset the port counters */
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

GT_STATUS prvTgfFdbDefCfgSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* Set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS], PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* Add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
}

GT_STATUS prvTgfFdbDefCfgRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

     /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* Vlan restore */

    /* Restore default PVID on port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS], prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
};

/**
* @internal prvTgfPclEgressTrapMirrorConfigurationSet function
* @endinternal
*
* @brief  Configure Egress PCL Rule with:
*         action pkt cmd : CPSS_PACKET_CMD_TRAP_TO_CPU_E
*         cpu code       :
*/
GT_VOID prvTgfPclEgressTrapMirrorCfgSet
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

    rc = prvTgfFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbDefCfgSet");

    /* PCL Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS],
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

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    action.pktCmd                                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode                        = CPSS_NET_IP_HDR_ERROR_E;
    action.egressPolicy                          = GT_TRUE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEgressTrapMirrorGenerate function
* @endinternal
*
* @brief  Configure to send packet to port 1
*/
GT_VOID prvTgfPclEgressTrapMirrorTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc  = GT_OK;
    GT_BOOL                         isOk;
    GT_BOOL                         expectTraffic = GT_FALSE;
    GT_U32                          portIter;
    GT_U32                          packetSize = 0;
    GT_U32                          partsCount = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    GT_U32                          counterVal;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");

    /* clear the counter before sending packet*/
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

        replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    }

    /* Start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Generate traffic */

    rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, &prvTgfPacketInfo,
            prvTgfBurstCount, 0 , NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth");

    /* Send packet to port 1 */
    rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth");

    partsCount    = sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC);

    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: verify routed packet on port 1 */
        switch (portIter)
        {
        case PRV_TGF_EPCL_SEND_PORT_INDEX_CNS:
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
        case PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS:
                if (expectTraffic == GT_TRUE)
                {
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                 }
                 else
                 {
                    expectedCntrs.goodOctetsSent.l[0] = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsSent.l[0]   = 0;
                    expectedCntrs.ucPktsSent.l[0]     = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.ucPktsRcv.l[0]      = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                 }
                    break;
            default:
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
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
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }
    if (GT_TRUE == prvTgfActionTrapToCpuCheck[PRV_TGF_CPU_PORT_IDX_CNS])
    {
        GT_U8                           packetBufPtr[64] = {0};
        GT_U32                          packetBufLen = 64;
        GT_U32                          packetLen;
        GT_U8                           devNum;
        GT_U8                           queueCpu;
        TGF_NET_DSA_STC                 rxParams;

        PRV_UTF_LOG0_MAC("CPU port capturing:\n");

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_TRUE, GT_TRUE,
                                           packetBufPtr,
                                           &packetBufLen,
                                           &packetLen,
                                           &devNum,
                                           &queueCpu,
                                           &rxParams);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_NET_IP_HDR_ERROR_E,
                                        rxParams.cpuCode);

        PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
        PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n",rxParams.cpuCode);

    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
        PRV_UTF_LOG1_MAC("No of Replicated packet :%d\n",counterVal);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, counterVal, "cpssDxChCfgReplicationCountersGet");

        replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
        rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, counterVal, "cpssDxChCfgReplicationCountersGet");
    }

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfPclEgressTrapMirrorCfgRestore
(
    GT_VOID
)
{
    GT_STATUS                         rc,rc1 = GT_OK;;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disables egress policy on port 2 */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_EPCL_RULE_INDEX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    rc = prvTgfFdbDefCfgRestore();
    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}
