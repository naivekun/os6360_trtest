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
* @file prvTgfTunnelTermSourceIdModify.c
*
* @brief Verify the functionality of TTI Source Id Modify
*
* @version   9
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS          6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* PCL rule & action index */
#define EPCL_MATCH_INDEX_CNS      prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(108)

/* Source Id data & mask */
#define SOURCE_ID_MASK_CNS     0xFFF

#define TTI_SET_SOURCE_ID_DATA_CNS  (0xFF << 2)

static struct{

    GT_U32 isValid;
    GT_U32 ttiRuleIndex;
    GT_U32 srcIdMaskOrig;    /* the TTI srcId mask, for config restore value */

    GT_U32 sourceId;         /* source Id value for lookup  */
    GT_U32 sourceIdMask;     /* source Id mask  for lookup  */

}lookupInfoStc[PRV_TGF_MAX_LOOKUPS_NUM_CNS];


/* original tcam segment mode value (for configuration restore) */
static PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT tcamSegmentModeOrig;


/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/******************************* Test packet **********************************/

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x05},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x15}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart = {
    0x1000,             /* label */
    0,                  /* CoS */
    1,                  /* S */
    0xff                /* TTL */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of basic packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfTunnelTermSourceIdModifyBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID tgfTunnelTermSourceIdModifyBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal tgfTunnelTermSourceIdModifyTtiConfigSet function
* @endinternal
*
* @brief   set TTI Basic configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID tgfTunnelTermSourceIdModifyTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_MPLS_E */
    /* AUTODOC: enable the TTI lookup for MPLS at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                        PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: save TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &tcamSegmentModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeGet");
}

/**
* @internal tgfTunnelTermSourceIdModifyTtiConfigPerLookupSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID tgfTunnelTermSourceIdModifyTtiConfigPerLookupSet
(
    GT_U8 lookupNum
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction,  0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Rule =======\n");

    /* AUTODOC: set the TTI Rule Action for all the packets */
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiAction.sourceIdSetEnable = GT_TRUE;
    ttiAction.sourceId          = lookupInfoStc[lookupNum].sourceId;

    ttiAction.ingressPipeBypass = GT_TRUE;

    ttiAction.tag0VlanCmd       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId        = PRV_TGF_EGR_VLANID_CNS;

    if(lookupNum != (PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1) )
    {
        ttiAction.continueToNextTtiLookup = GT_TRUE;
    }

    /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with action: modify srcId */
    rc = prvTgfTtiRule2Set(lookupInfoStc[lookupNum].ttiRuleIndex,
                            PRV_TGF_TTI_KEY_MPLS_E, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}


/**
* @internal tgfTunnelTermSourceIdModifyPclConfigSet function
* @endinternal
*
* @brief   set generic Pcl config
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID tgfTunnelTermSourceIdModifyPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_UTF_LOG0_MAC("======= Setting Pcl Configuration =======\n");

    /* AUTODOC: Init EPCL Engine for port 1 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E  /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);
}

/**
* @internal tgfTunnelTermSourceIdModifyPclConfigPerLookupSet function
* @endinternal
*
* @brief   build Pcl rule to match source Id
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID tgfTunnelTermSourceIdModifyPclConfigPerLookupSet
(
    GT_U32 sourceId
)
{
    GT_STATUS rc;

    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));


    PRV_UTF_LOG0_MAC("======= Setting Pcl Rule  =======\n");

    /* AUTODOC: EPCL Pattern for Source ID */
    pattern.ruleEgrExtNotIpv6.common.sourceId = sourceId;

    /* AUTODOC: EPCL Mask for Source ID */
    mask.ruleEgrExtNotIpv6.common.sourceId = 0xFFF;


    /* AUTODOC: EPCL action - drop the packet */
    action.pktCmd  = CPSS_PACKET_CMD_DROP_HARD_E;
    action.egressPolicy = GT_TRUE;

#if 0
    /* AUTODOC: EPCL action - redirect to port 56 */
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
#endif

    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
                            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E, EPCL_MATCH_INDEX_CNS);
}



/**
* @internal tgfTunnelTermSourceIdModifyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
*                                       None
*/
static GT_VOID tgfTunnelTermSourceIdModifyTrafficGenerate
(
    IN GT_BOOL  expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          numOfExpectedRxPackets;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:05, SA=00:00:00:00:00:15 */
    /* AUTODOC:   VID=5, Ethertype=0x8847, MPLS Label=0x1000 */
    /* AUTODOC:   Passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:12 */

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        if(PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
            /* AUTODOC: verify to get Ethernet packet on port 3 with: */
            UTF_VERIFY_EQUAL1_STRING_MAC(portCntrs.goodPktsRcv.l[0], numOfExpectedRxPackets,
                "there is no traffic on expected port, packet Ether type: 0x%X\n",
                                                prvTgfPacketEtherTypePart.etherType);
        }
    }

}

/**
* @internal tgfTunnelTermSourceIdModifyConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
static GT_VOID tgfTunnelTermSourceIdModifyConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      lookupNum;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MPLS */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    for(lookupNum = 0; lookupNum < PRV_TGF_MAX_LOOKUPS_NUM_CNS; lookupNum++)
    {
        if(0 == lookupInfoStc[lookupNum].isValid)
        {
            break;
        }

        /* AUTODOC: invalidate TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(lookupInfoStc[lookupNum].ttiRuleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

        /* AUTODOC: restore tti src id mask */
        rc = prvTgfTtiSourceIdBitsOverrideSet(prvTgfDevNum, lookupNum,
                                              lookupInfoStc[lookupNum].srcIdMaskOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiSourceIdBitsOverrideSet");
    }

    /* AUTODOC: restore TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, tcamSegmentModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate EPCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E, EPCL_MATCH_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_EXT_E, EPCL_MATCH_INDEX_CNS, GT_FALSE);

    /* AUTODOC: Clear EPCL Engine configuration tables */
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal tgfTunnelTermSourceIdModify function
* @endinternal
*
* @brief   Test of TTI Source Id Modify (main func)
*/
GT_VOID tgfTunnelTermSourceIdModify
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       lookupNum;
    GT_U32      pclSourceId = 0;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    tgfTunnelTermSourceIdModifyBridgeConfigSet();

    /* AUTODOC: set generic TTI configuration */
    tgfTunnelTermSourceIdModifyTtiConfigSet();

    /* AUTODOC: set generic PCL configuration */
    tgfTunnelTermSourceIdModifyPclConfigSet();


    /* AUTODOC: set TTI lookups configuration */
    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        PRV_UTF_LOG1_MAC("\n============================ Process lookup %d =======================\n", lookupNum);

        /* AUTODOC: init lookup data */
        lookupInfoStc[lookupNum].isValid = 1;
        lookupInfoStc[lookupNum].ttiRuleIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(lookupNum,3);
        lookupInfoStc[lookupNum].sourceId     = 1 << lookupNum;
        lookupInfoStc[lookupNum].sourceIdMask = 1 << lookupNum;


        /* AUTODOC: save tti src id mask for config restore */
        rc = prvTgfTtiSourceIdBitsOverrideGet(prvTgfDevNum, lookupNum,
                                              &lookupInfoStc[lookupNum].srcIdMaskOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiSourceIdBitsOverrideGet");


        /* AUTODOC: set the TTI Rule Action for all the packets */
        /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with */
        /* AUTODOC: TTI action: modify srcId, modify vid to 6, redirect to port 36 */
        tgfTunnelTermSourceIdModifyTtiConfigPerLookupSet(lookupNum);

        /*--------------------------------*/
        /* AUTODOC: set tti src id mask, so tti will aply 'correct' srcId, so ePcl will match */
        rc = prvTgfTtiSourceIdBitsOverrideSet(prvTgfDevNum, lookupNum,
                                               lookupInfoStc[lookupNum].sourceIdMask);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiSourceIdBitsOverrideSet");

        if( 0 == lookupNum)
        {
            PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT tcamSegmentMode =
                            PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE ?
                                                PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E :
                                                PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E ;

            /* AUTODOC: set TCAM segment mode for Mpls key */
            rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, tcamSegmentMode);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");
        }

        pclSourceId |= lookupInfoStc[lookupNum].sourceId;

        PRV_UTF_LOG1_MAC("=========== Checking final source Id: 0x%X ==========\n", pclSourceId);
    }


    /* AUTODOC: set the ePcl Rule for the packets matching final tti srcId   */
    /* AUTODOC: ePcl rule action: drop packets with tti src id value */
    tgfTunnelTermSourceIdModifyPclConfigPerLookupSet(pclSourceId);


    /* AUTODOC: send packet and expect TTI match, and no traffic on port 36 (dropped by ePcl) */
    tgfTunnelTermSourceIdModifyTrafficGenerate(GT_FALSE);


    /*--------------------------------------------------------------*/
    /* AUTODOC: set tti src id mask, so tti will aply another srcId, so ePcl will not match */
    rc = prvTgfTtiSourceIdBitsOverrideSet(prvTgfDevNum, 0 /* zero lookup */, 0 );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiSourceIdBitsOverrideSet");

    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 (no ePcl match, no drop) */
    tgfTunnelTermSourceIdModifyTrafficGenerate(GT_TRUE);


    /*--------------------------------*/
    /* AUTODOC: test configuration restore */
    tgfTunnelTermSourceIdModifyConfigRestore();
}


