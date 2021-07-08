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
* @file prvTgfExactMatchBasicDefaultActionPcl.c
*
* @brief Test Exact Match Default Action functionality with PCL Action Type
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


#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionPcl.h>


#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS 5

/* parameters that is needed to be restored */
static struct
{
    GT_BOOL                             profileIdEn;
    GT_U32                              profileId;
    GT_BOOL                             defaultActionEn;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      defaultAction;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT      firstLookupClientType;
} prvTgfExactMatchRestoreCfg;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   2;


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

/**
* @internal prvTgfExactMatchBasicDefaultActionPclGenericConfig function
* @endinternal
*
* @brief   Set PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclGenericConfig
(
    GT_VOID
)
{
    /* Set PCl generic configuration */
    prvTgfPclEgressTrapMirrorCfgSet();
}
/**
* @internal prvTgfExactMatchBasicDefaultActionPclGenericRestore function
* @endinternal
*
* @brief   Restore PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclGenericRestore
(
    GT_VOID
)
{
    /* Restore PCL generic configuration */
    prvTgfPclEgressTrapMirrorCfgRestore();
}
/**
* @internal prvTgfExactMatchBasicDefaultActionPclInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclInvalidateRule
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_PCL_RULE_SIZE_ENT         ruleSize;
    GT_U32                         ruleIndex;
    GT_BOOL                        valid;

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
    ruleIndex = PRV_TGF_EPCL_RULE_INDEX_CNS;/* configured index in prvTgfPclEgressTrapMirrorCfgSet*/
    valid = GT_FALSE;

    /* invalidate the rule configured in prvTgfPclEgressTrapMirrorCfgSet
       ruleFormat was configured to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
       PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E is translated to CPSS_DXCH_TCAM_RULE_SIZE_30_B_E
       CPSS_PCL_RULE_SIZE_STD_E will be traslated to CPSS_DXCH_TCAM_RULE_SIZE_30_B_E */
    rc =  prvTgfPclRuleValidStatusSet(ruleSize,ruleIndex,valid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");
}
/**
* @internal GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT      firstLookupClientType;
    CPSS_PCL_DIRECTION_ENT              direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT         pclPacketType;
    GT_U32                              subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      exactMatchLookupNum;

    GT_BOOL                             enableExactMatchLookup;
    GT_U32                              profileId;

    GT_BOOL                             enableDefaultAction;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT defaultActionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      defaultActionData;

    /* AUTODOC: keep clientType configured for first lookup , for restore */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,exactMatchLookupNum,&prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set first lookup client type to be PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E */
    firstLookupClientType = PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: keep profileId configured for first lookup , for restore */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_EGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    pclLookupNum=PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchPclProfileIdGet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         &prvTgfExactMatchRestoreCfg.profileIdEn , &prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdGet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: set profileId=5 for PCL first lookup */
    enableExactMatchLookup = GT_TRUE;
    profileId = PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS;
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId,
                                         pclLookupNum , enableExactMatchLookup , profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);


    /* clear entry */
    cpssOsMemSet((GT_VOID*) &defaultActionData, 0, sizeof(defaultActionData));

    defaultActionData.pclAction.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    /* For SIP6 and above for the egress control pipe the CPU code is also 'drop code' so relevant for drops too. */
    defaultActionData.pclAction.mirror.cpuCode=CPSS_NET_FIRST_USER_DEFINED_E+1;
    defaultActionData.pclAction.egressPolicy = GT_TRUE;
    defaultActionType = PRV_TGF_EXACT_MATCH_ACTION_EPCL_E;
    enableDefaultAction = GT_TRUE ;

    /* AUTODOC: save Default Action , for restore */
    rc = prvTgfExactMatchProfileDefaultActionGet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS ,
                                                 defaultActionType,&prvTgfExactMatchRestoreCfg.defaultAction,
                                                 &prvTgfExactMatchRestoreCfg.defaultActionEn);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionGet FAILED: %d", prvTgfDevNum);



    /* AUTODOC: set Default Action for PCL lookup, profileId=5, ActionType=EPCL, enableDefault=GT_TRUE, packet Command = DROP */
     rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                  defaultActionType,&defaultActionData,enableDefaultAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);

    return;
}
/**
* @internal GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    CPSS_PCL_DIRECTION_ENT              direction;
    PRV_TGF_PCL_PACKET_TYPE_ENT         pclPacketType;
    GT_U32                              subProfileId;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      pclLookupNum;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT      exactMatchLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT defaultActionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      zeroAction;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));

    /* AUTODOC: Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    pclLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    direction = CPSS_PCL_DIRECTION_EGRESS_E;
    pclPacketType = PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E;
    subProfileId = 0;/* by default if not configured this field is 0 */
    pclLookupNum=PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchPclProfileIdSet(prvTgfDevNum, direction, pclPacketType, subProfileId, pclLookupNum,
                                         prvTgfExactMatchRestoreCfg.profileIdEn , prvTgfExactMatchRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPclProfileIdSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: Restore first lookup client type */
    exactMatchLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,exactMatchLookupNum, prvTgfExactMatchRestoreCfg.firstLookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    defaultActionType = PRV_TGF_EXACT_MATCH_ACTION_EPCL_E;

    /* AUTODOC: Restore Default Action */
    if (prvTgfExactMatchRestoreCfg.defaultActionEn == GT_TRUE )
    {
        /* AUTODOC: restore - enable Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                     defaultActionType,
                                                     &prvTgfExactMatchRestoreCfg.defaultAction,
                                                     GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: restore - disable Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                     defaultActionType,
                                                     &zeroAction,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }
    return;
}

/**
* @internal GT_VOID prvTgfExactMatchBasicDefaultActionPclTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped or dropped
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclTrafficGenerate
(
    GT_BOOL     expectNoTraffic
)
{
    GT_STATUS                       rc  = GT_OK;
    GT_BOOL                         isOk;
    GT_BOOL                         expectTraffic = GT_FALSE;/* the test only check trap or dropped so no forwarded traffic expected */
    GT_U32                          portIter;
    GT_U32                          packetSize = 0;
    GT_U32                          partsCount = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    /*CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    GT_U32                          counterVal;*/

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");

    #if 0
    /* clear the counter before sending packet*/
    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    #endif
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

        cpssOsBzero((GT_VOID*)&rxParams, sizeof(TGF_NET_DSA_STC));

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_TRUE, GT_TRUE,
                                           packetBufPtr,
                                           &packetBufLen,
                                           &packetLen,
                                           &devNum,
                                           &queueCpu,
                                           &rxParams);
        if(expectNoTraffic==GT_TRUE)
        {
            /* we want to verify no packet was trapped to CPU
               the command in the Exact Match Default Action was DROP */
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(CPSS_NET_IP_HDR_ERROR_E,rxParams.cpuCode);

            PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
            PRV_UTF_LOG0_MAC("Packet is NOT Trapped to  CPU. Packet is dropped\n\n");
        }
        else
        {
            /* we want to verify packet was trapped to CPU */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
            UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_NET_IP_HDR_ERROR_E,rxParams.cpuCode);

            PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
            PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n\n",rxParams.cpuCode);
        }
    }

#if 0
    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    PRV_UTF_LOG1_MAC("No of Replicated packet :%d\n",counterVal);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(prvTgfBurstCount, counterVal, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, counterVal, "cpssDxChCfgReplicationCountersGet");
#endif
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

