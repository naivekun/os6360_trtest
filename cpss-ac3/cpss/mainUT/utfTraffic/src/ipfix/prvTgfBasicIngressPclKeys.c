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
* @file prvTgfBasicIngressPclKeys.c
*
* @brief Ingress PCL key STD L2 infrastructure for IPFIX
*
* @version   9
********************************************************************************
*/

#include <ipfix/prvTgfBasicIngressPclKeys.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* default number of packets to send */
static GT_U32  prvTgfBurstCountB   = 1;

/* port bitmap VLAN members */
static CPSS_PORTS_BMP_STC localPortsVlanMembersB = {{0, 0}};


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartB = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0PartB = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArrB[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPartB = {
    sizeof(prvTgfPayloadDataArrB),                       /* dataLength */
    prvTgfPayloadDataArrB                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArrayB[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2PartB},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0PartB},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPartB}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArrB)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoB = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArrayB) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayB                                        /* partsArray */
};

static PRV_TGF_POLICER_STAGE_METER_MODE_ENT stageModeSave;
/**
* @internal prvTgfIpfixVlanTestInit function
* @endinternal
*
* @brief   Initialize local vlan default settings
*
* @param[in] vlanId                   - vlan id.
*                                      portMembers - bit map of ports belonging to the vlan.
*                                       None
*/
GT_VOID prvTgfIpfixVlanTestInit
(
    IN GT_U16                                  vlanId,
    CPSS_PORTS_BMP_STC                         portsMembers
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
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
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfIpfixVlanRestore function
* @endinternal
*
* @brief   Restore\Cancel prvTgfIpfixVlanTestInit settings
*
* @param[in] vlanId                   - vlan id.
*                                      portMembers - bit map of ports belonging to the vlan.
*                                       None
*/
GT_VOID prvTgfIpfixVlanRestore
(
    IN GT_U16           vlanId,
    CPSS_PORTS_BMP_STC  portsMembers
)
{
    GT_STATUS                   rc = GT_OK;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", vlanId);
}

/**
* @internal prvTgfIpfixTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
* @param[in] burstCount               - number of packets to send
*                                       None
*/
GT_VOID prvTgfIpfixTestPacketSend
(
    IN GT_U32           portNum,
    IN TGF_PACKET_STC *packetInfoPtr,
    IN GT_U32          burstCount
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, burstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, burstCount, 0, NULL);

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfIpfixHelloTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixHelloTestInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembersB, 0, sizeof(localPortsVlanMembersB));

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembersB,prvTgfPortsArray[portIter]);
    }

    /* set VLAN entry */
     prvTgfIpfixVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembersB);

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* enables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_TRUE);

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                CPSS_PCL_DIRECTION_INGRESS_E,
                                                CPSS_PCL_LOOKUP_0_E,
                                                0,
                                                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_0_E,
                                 0,
                                 PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");


    /* set PCL rule */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = 0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 0x1234;

    cpssOsMemSet(&action, 0, sizeof(action));

    ipfixIndex = 0;

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.policer.policerId = ipfixIndex;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    /* Save stage mode */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, &stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d",
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E);
    /* IPFIX works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_INGRESS_0_E, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum,
                                      PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                      &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    rc = prvTgfIpfixEntrySet(prvTgfDevNum,
                             PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);
}

/**
* @internal prvTgfIpfixHelloTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 0
*         Success Criteria:
*         Packet is captured on ports 0,18,23.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 0x1234
*         Success Criteria:
*         Packet is captured on ports 0,18,23 and IPFIX entry is updated.
*/
GT_VOID prvTgfIpfixHelloTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                  ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfoB, prvTgfBurstCountB);

    cpssOsTimerWkAfter(200);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCountB, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }


        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCountB, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    ipfixIndex = 0;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum,
                             PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);
    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Set ethertype to value 0x1234 */
    prvTgfPayloadDataArrB[0] = 0x12;
    prvTgfPayloadDataArrB[1] = 0x34;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfoB, prvTgfBurstCountB);
    cpssOsTimerWkAfter(200);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCountB, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCountB, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfIpfixEntryGet(prvTgfDevNum,
                             PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                 ipfixIndex);
    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(84, ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixHelloTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixHelloTestRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ruleIndex;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;

    prvTgfPayloadDataArrB[0] = 0;
    prvTgfPayloadDataArrB[1] = 0;

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
    ruleIndex = 0;

    rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 ruleSize, ruleIndex, GT_FALSE);

    /* Disables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_FALSE);

    /* Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* Restore stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_INGRESS_0_E, stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 stageModeSave);

    prvTgfIpfixVlanRestore(PRV_TGF_VLANID_CNS, localPortsVlanMembersB);
}

