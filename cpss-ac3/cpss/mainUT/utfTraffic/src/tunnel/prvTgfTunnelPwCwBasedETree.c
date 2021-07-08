/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvtgfTunnelPwCwBasedETree.c
*
* DESCRIPTION:
*       Verify the functionality of TTI Pw Cw Based ETree
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/
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

#include <tunnel/prvTgfTunnelPwCwBasedETree.h>

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

#define TTI_SET_SOURCE_ID_DATA_CNS  1

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* the TTI Rule index */
static GT_U32 prvTgfTtiRuleLookup0Index; /* using absolute index */
static GT_U32 prvTgfTtiRuleLookup1Index;

static PRV_TGF_TTI_ACTION_2_STC ttiAction;
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;


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

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart2 = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_EGR_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart = {
    0x1,             /* label */
    0,                  /* CoS */
    1,                  /* S */
    0xff                /* TTL */
};

static GT_U8 prvTgfPacketPwCwWithLBitPartArr[] = {
    1<<3, 0x00, 0x00, 0x00
};

/* MPLS PW Label part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPwCwWithLBitPart = {
    sizeof(prvTgfPacketPwCwWithLBitPartArr),   /* dataLength */
    prvTgfPacketPwCwWithLBitPartArr            /* dataPtr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of basic packet with <L>-bit set*/
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPwCwWithLBitPart},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart2},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PACKET MPLS to send */
static TGF_PACKET_STC prvTgfPacketMplsInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfTunnelPwCwBasedETreeBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID tgfTunnelPwCwBasedETreeBridgeConfigSet
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
* @internal tgfTunnelPwCwBasedETreeTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID tgfTunnelPwCwBasedETreeTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Action for all the packets */
    ttiAction.tunnelTerminate                   = GT_TRUE;

    ttiAction.ttPassengerPacketType             = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E,
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum    = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiAction.ingressPipeBypass = GT_TRUE;

    ttiAction.tag0VlanCmd       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId        = PRV_TGF_EGR_VLANID_CNS;

    ttiAction.cwBasedPw            = GT_TRUE;

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_MPLS_E */
    /* AUTODOC: enable the TTI lookup for MPLS at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");
}

/**
* @internal tgfTunnelPwCwBasedETreePclConfigSet function
* @endinternal
*
* @brief   build Pcl rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID tgfTunnelPwCwBasedETreePclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    PRV_UTF_LOG0_MAC("======= Setting Pcl Configuration =======\n");

    /* AUTODOC: Init EPCL Engine for port 2 */
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

    /* AUTODOC: EPCL Mask for Source ID */
    mask.ruleEgrExtNotIpv6.common.sourceId = SOURCE_ID_MASK_CNS;

    /* AUTODOC: EPCL Pattern for Source ID */
    pattern.ruleEgrExtNotIpv6.common.sourceId = TTI_SET_SOURCE_ID_DATA_CNS;

    /* AUTODOC: EPCL action - drop the packet */
    action.pktCmd  = CPSS_PACKET_CMD_DROP_HARD_E;
    action.egressPolicy = GT_TRUE;

    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
                            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
                                 EPCL_MATCH_INDEX_CNS);
}

/**
* @internal tgfTunnelPwCwBasedETreeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
*                                       None
*/
GT_VOID tgfTunnelPwCwBasedETreeTrafficGenerate
(
    IN GT_BOOL  expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
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

    numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketMplsInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from port 0 with: */
    /* AUTODOC:   <L> bit set to 0 if expecting traffic */
    /* AUTODOC:   <L> bit set to 1 if not expecting traffic */
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
            if (expectTraffic)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(portCntrs.goodPktsRcv.l[0], numOfExpectedRxPackets,
                    "there is no traffic on expected port, packet Ether type: 0x%X\n",
                                                    prvTgfPacketEtherTypePart.etherType);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(portCntrs.goodPktsRcv.l[0], numOfExpectedRxPackets,
                    "received traffic but not expecteed on expected port, packet Ether type: 0x%X\n",
                                                    prvTgfPacketEtherTypePart.etherType);
            }
        }
    }

}

/**
* @internal tgfTunnelPwCwBasedETreeSetEnable function
* @endinternal
*
* @brief   Sets the TTI Action pwCwBasedETreeEnable
*/
GT_VOID tgfTunnelPwCwBasedETreeSetEnable(GT_BOOL enable)
{
    GT_STATUS rc = GT_OK;

    ttiAction.pwCwBasedETreeEnable = enable;

    /* AUTODOC: add TTI rule  with MPLS key on port 0 VLAN 5 with action: modify srcId */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup0Index, PRV_TGF_TTI_KEY_MPLS_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal tgfTunnelPwCwBasedETreeDualLookupETreeEnable function
* @endinternal
*
* @brief   Enables the TTI1 pwCwBasedETreeEnable
*/
GT_VOID tgfTunnelPwCwBasedETreeDualLookupETreeEnable()
{
    GT_BOOL rc = GT_OK;

    ttiAction.pwCwBasedETreeEnable = GT_TRUE;

    /* AUTODOC: add TTI rule  with MPLS key on port 0 VLAN 5 with action: modify srcId */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleLookup1Index, PRV_TGF_TTI_KEY_MPLS_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal tgfTunnelPwCwBasedETreeConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID tgfTunnelPwCwBasedETreeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

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

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup0Index, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup1Index, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

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
* @internal tgfTunnelPwCwBasedETreeDualLookupEnable function
* @endinternal
*
* @brief   Enables second TTI Lookup
*/
GT_VOID tgfTunnelPwCwBasedETreeDualLookupEnable(GT_VOID)
{
    ttiAction.continueToNextTtiLookup     = GT_TRUE;
}

/**
* @internal tgfTunnelPwCwBasedETree function
* @endinternal
*
* @brief   Test of TTI pwCwBasedETreeEnable (main func)
*/
GT_VOID tgfTunnelPwCwBasedETree
(
    GT_VOID
)
{
    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 ,3);
    prvTgfTtiRuleLookup1Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1 ,0);

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    tgfTunnelPwCwBasedETreeBridgeConfigSet();

    /*-----------------------------------------------------------------------*/
    /* AUTODOC: set the TTI Rule Action for all the packets */
    /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with */
    /* AUTODOC: TTI action: modify srcId, modify vid to 6, redirect to port 36 */
    tgfTunnelPwCwBasedETreeTtiConfigSet();

    /*-----------------------------------------------------------------------*/
    /* AUTODOC: set the ePcl Rule for the packets matching tti srcId   */
    /* AUTODOC: ePcl rule action: drop packets with tti src id value */
    tgfTunnelPwCwBasedETreePclConfigSet();

    /*-----------------------------------------------------------------------*/
    /* AUTODOC: send packet and expect TTI match, and no traffic on port 36 (dropped by ePcl) */
    tgfTunnelPwCwBasedETreeSetEnable(GT_TRUE);
    tgfTunnelPwCwBasedETreeTrafficGenerate(GT_FALSE);

    /*-----------------------------------------------------------------------*/
    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 (no PCL match, packet forward) */
    tgfTunnelPwCwBasedETreeSetEnable(GT_FALSE);
    tgfTunnelPwCwBasedETreeTrafficGenerate(GT_TRUE);

    /*-----------------------------------------------------------------------*/
    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 (no PCL match, packet forward) */
    tgfTunnelPwCwBasedETreeSetEnable(GT_TRUE);
    prvTgfPacketPwCwWithLBitPartArr[0] = 0;
    tgfTunnelPwCwBasedETreeTrafficGenerate(GT_TRUE);

    /*-----------------------------------------------------------------------*/
    /* AUTODOC: send packet and expect TTI match and TTI1 match and no traffic on port 36 (dropped by ePcl) */
    prvTgfPacketPwCwWithLBitPartArr[0] = 1<<3;
    tgfTunnelPwCwBasedETreeDualLookupEnable();
    tgfTunnelPwCwBasedETreeSetEnable(GT_FALSE);
    tgfTunnelPwCwBasedETreeDualLookupETreeEnable();
    tgfTunnelPwCwBasedETreeTrafficGenerate(GT_FALSE);

    /*----------------------------------------------------------------------*/
    /* AUTODOC: test configuration restore */
    tgfTunnelPwCwBasedETreeConfigRestore();
}



