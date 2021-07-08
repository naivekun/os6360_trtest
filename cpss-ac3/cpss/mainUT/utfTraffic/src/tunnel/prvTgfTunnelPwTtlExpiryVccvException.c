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
* @file prvTgfTunnelPwTtlExpiryVccvException.c
*
* @brief Tunnel: Test VCCV exception on PW TTL
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelPwTtlExpiryVccvException.h>


/**************************** packet MPLS **********************************/

#define PRV_TGF_VLANID_5_CNS 5
#define PRV_TGF_VLANID_6_CNS 6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS 0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS 3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* the TTI Rule index */
#define prvTgfTtiRuleIndex (prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3)) /* using absolute index */

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketMplsVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                       /* pri, cfi, VlanId */
};

/* MPLS ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsMplsEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

static GT_U8 prvTgfPacketPwLabelPartArr[] = {
    0x1,    /* label */
    0x77,   /* exp */
    0x01,   /* s-bit */
    0x01    /* ttl */
};

/* MPLS PW Label part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPwLabelPart = {
    sizeof(prvTgfPacketPwLabelPartArr),                         /* dataLength */
    prvTgfPacketPwLabelPartArr                                  /* dataPtr */
};


/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfPacketMplsPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketMplsVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPwLabelPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET MPLS to send */
static TGF_PACKET_STC prvTgfPacketMplsInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketMplsPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMplsPartArray                                        /* partsArray */
};

/* vccv command to restore */
static CPSS_PACKET_CMD_ENT vccvCmdRestore;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfTunnelPwTtlExpiryVccvExceptionBridgeConfigSet function
* @endinternal
*
* @brief   Set bridge configuration
*/
static GT_VOID prvTgfTunnelPwTtlExpiryVccvExceptionBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc         = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelPwTtlExpiryVccvExceptionTtiRuleConfigurationSet function
* @endinternal
*
* @brief   Set basic rule configuration
*/
static GT_VOID prvTgfTunnelPwTtlExpiryVccvExceptionTtiRuleConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT pattern;
    PRV_TGF_TTI_RULE_UNT mask;
    PRV_TGF_TTI_ACTION_2_STC  ruleAction;

    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* AUTODOC: enable TTI lookup for port 0, key PRV_TGF_TTI_KEY_MPLS_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern and Mask to match any packets */
    /* AUTODOC: set the TTI Rule Action for matched packet: redirect to egress and enable ttlExpiryVccvEnable  */
    ruleAction.redirectCommand                 = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ruleAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.tag0VlanId                      = 0x06;

    /* control word is not relevant for ttlExpiryVccvEnable check */
    /*ruleAction.cwBasedPw           = GT_TRUE;*/

    ruleAction.ttlExpiryVccvEnable = GT_TRUE;

    ruleAction.ingressPipeBypass   = GT_TRUE;


    /* set TTI rule */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_MPLS_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);


    /* AUTODOC: save the current command for illegal control word */
    rc = prvTgfTtiPwCwExceptionCmdGet(prvTgfDevNum, PRV_TGF_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E,
                                      &vccvCmdRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwExceptionCmdGet");

}


/**
* @internal prvTgfTunnelPwTtlExpiryVccvExceptionTrafficGenerate function
* @endinternal
*
* @brief   Generate Traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
*                                       None
*/
static GT_VOID prvTgfTunnelPwTtlExpiryVccvExceptionTrafficGenerate
(
    IN GT_BOOL  expectTraffic
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portIter    = 0;
    GT_U32    prvTgfBurstCount = 1;
    GT_U32    numOfExpectedRxPackets;

    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* AUTODOC: GENERATE TRAFFIC */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 1 mpls packet from port 0 with: VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfPacketMplsInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");



    numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;

    /* AUTODOC: verify to get packet on port 3 with: */
    UTF_VERIFY_EQUAL3_STRING_MAC(portCntrs.goodPktsRcv.l[0], numOfExpectedRxPackets,
        "got wrong counters: port[%d], expected [%d], received[%d]\n",
         prvTgfPortsArray[portIter], numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0]);

}

/**
* @internal prvTgfTunnelPwTtlExpiryVccvExceptionConfigurationRestore function
* @endinternal
*
* @brief   Restore base configuration
*/
static GT_VOID prvTgfTunnelPwTtlExpiryVccvExceptionConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */

    GT_STATUS          rc = GT_OK;
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for port 0, key PRV_TGF_TTI_KEY_MPLS_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

}


/**
* @internal prvTgfTunnelPwTtlExpiryVccvException function
* @endinternal
*
* @brief   Test main func
*/
GT_VOID prvTgfTunnelPwTtlExpiryVccvException
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;

    /* AUTODOC: build mpls packet with s-bit 0x1, ttl 0x1 */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    prvTgfTunnelPwTtlExpiryVccvExceptionBridgeConfigSet();

    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_MPLS_E on port 0 VLAN 5 with action: redirect to egress */
    prvTgfTunnelPwTtlExpiryVccvExceptionTtiRuleConfigurationSet();

    /*------------------------------------------------------------*/
    /* AUTODOC: send packet and expect TTI match, and no traffic on port 36, dropped (exception) */
    prvTgfTunnelPwTtlExpiryVccvExceptionTrafficGenerate(GT_FALSE);

    /* -------------------------------------------------------------------------*/
    /* AUTODOC: change ttl field in pw label to 0x2 */
    prvTgfPacketPwLabelPartArr[3] = 0x2;

    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0x2, no exception) */
    prvTgfTunnelPwTtlExpiryVccvExceptionTrafficGenerate(GT_TRUE);

    /* -------------------------------------------------------------------------*/
    /* AUTODOC: change ttl field in pw label to 0 */
    prvTgfPacketPwLabelPartArr[3] = 0;

    /* AUTODOC: set the CPSS_PACKET_CMD_FORWARD_E command for expiry_vccv_error exception */
    rc = prvTgfTtiPwCwExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E,
                                      CPSS_PACKET_CMD_FORWARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwExceptionCmdSet: %d", prvTgfDevNum);

    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0, exception forward) */
    prvTgfTunnelPwTtlExpiryVccvExceptionTrafficGenerate(GT_TRUE);


    /* -------------------------------------------------------------------------*/
    /* AUTODOC: set the CPSS_PACKET_CMD_DROP_HARD_E command for expiry_vccv_error exception */
    rc = prvTgfTtiPwCwExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwExceptionCmdSet: %d", prvTgfDevNum);

    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0, exception drop) */
    prvTgfTunnelPwTtlExpiryVccvExceptionTrafficGenerate(GT_FALSE);

    /* -------------------------------------------------------------------------*/

    /* AUTODOC: restore vccv command */
    rc = prvTgfTtiPwCwExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E,
                                      vccvCmdRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwExceptionCmdSet: %d", prvTgfDevNum);

    /* AUTODOC: test configuration restore */
    prvTgfTunnelPwTtlExpiryVccvExceptionConfigurationRestore();
}

