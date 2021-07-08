/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prfTgfTunnelDualLookupNestedVLan.c
*
* DESCRIPTION:
*       Tunnel: Test Dual Lookup Nested VLAN
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prfTgfTunnelDualLookupNestedVLan.h>


/**************************** packet Ethernet **********************************/

#define PRV_TGF_VLANID_5_CNS 5
#define PRV_TGF_VLANID_6_CNS 6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS 0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS 3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* the TTI Rule index */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)
#define PRV_TGF_TTI1_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1, 0)

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthernetVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                       /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet Ethernet */
static TGF_PACKET_PART_STC prvTgfPacketEthernetPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketEthernetVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET Ethernet to send */
static TGF_PACKET_STC prvTgfPacketEthernetInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketEthernetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthernetPartArray                                        /* partsArray */
};

static PRV_TGF_TTI_RULE_UNT      pattern;
static PRV_TGF_TTI_RULE_UNT      mask;
static PRV_TGF_TTI_ACTION_2_STC  ruleAction;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prfTgfTunnelDualLookupNestedVLanBridgeConfigSet function
* @endinternal
*
* @brief   Set bridge configuration
*/
static GT_VOID prfTgfTunnelDualLookupNestedVLanBridgeConfigSet
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
* @internal prfTgfTunnelDualLookupNestedVLanTtiRuleConfigurationSet function
* @endinternal
*
* @brief   Set basic rule configuration
*/
static GT_VOID prfTgfTunnelDualLookupNestedVLanTtiRuleConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* AUTODOC: enable TTI lookup for port 0, key PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern and Mask to match any packets */
    /* AUTODOC: set the TTI Rule Action for matched packet: redirect to egress and enable nestedVlanEnable */
    ruleAction.redirectCommand                 = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ruleAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.tag0VlanId                      = 0x06;

    ruleAction.ingressPipeBypass   = GT_TRUE;

    ruleAction.nestedVlanEnable = GT_TRUE;

    ruleAction.continueToNextTtiLookup = GT_TRUE;

    /* set TTI rule */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
}


/**
* @internal prfTgfTunnelDualLookupNestedVLanTtiRuleSet function
* @endinternal
*
* @brief   Sets the TTTI rule nestedVlanEnable
*
* @param[in] enable                   - sets nestedVlanEnable
*                                      rule index - sets the index of a rule to set
*                                       None
*/
GT_VOID prfTgfTunnelDualLookupNestedVLanTtiRuleSet
(
    GT_BOOL enable,
    GT_U32  ruleIndex
)
{
    GT_STATUS rc = GT_OK;

    ruleAction.nestedVlanEnable = enable;

    /* AUTODOC: add TTI rule <ruleIndex> with ETH key on port 0 VLAN 5 with action: redirect to egress */
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_ETH_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prfTgfTunnelDualLookupNestedVLanTrafficOnPort function
* @endinternal
*
* @brief   Checks packet on egress
*
* @param[in] vlanPresent              - vlan present
*                                       None
*/
static GT_VOID prfTgfTunnelDualLookupNestedVLanTrafficOnPort
(
    GT_BOOL vlanPresent
)
{
    GT_STATUS rc          = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];
    GT_U32                  numTriggers  = 0;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* Prepare VFD array entries for VLAN tag */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].cycleCount = sizeof(GT_U8);
    vfdArray[0].modeExtraInfo=0;
    vfdArray[0].incValue=1;
    vfdArray[0].patternPtr[0]= vlanPresent == GT_FALSE ? 0x00 : 0x81;
    vfdArray[0].cycleCount=1;

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1,
                                                        vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                               "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                               " port = %d, rc = 0x%02X\n",
                               portInterface.devPort.portNum, rc);
}

/**
* @internal prfTgfTunnelDualLookupNestedVLanTrafficGenerate function
* @endinternal
*
* @brief   Generate Traffic
*
* @param[in] vlanPresent              - GT_TRUE: expect VLAN tag to be in packet
*                                      GT_FALSE: expect no VLAN tag
*                                       None
*/
static GT_VOID prfTgfTunnelDualLookupNestedVLanTrafficGenerate
(
    IN GT_BOOL  vlanPresent
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portIter    = 0;
    GT_U32    prvTgfBurstCount = 1;
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

    /* AUTODOC: send 1 ethernet packet from port 0 with: VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfPacketEthernetInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

    /* AUTODOC: verify to get packet on port 3 with: */
    UTF_VERIFY_EQUAL3_STRING_MAC(portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
        "got wrong counters: port[%d], expected [%d], received[%d]\n",
         prvTgfPortsArray[portIter], prvTgfBurstCount, portCntrs.goodPktsRcv.l[0]);

    prfTgfTunnelDualLookupNestedVLanTrafficOnPort(vlanPresent);
}

/**
* @internal prfTgfTunnelDualLookupNestedVLanConfigurationRestore function
* @endinternal
*
* @brief   Restore base configuration
*/
static GT_VOID prfTgfTunnelDualLookupNestedVLanConfigurationRestore
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

    /* AUTODOC: disable TTI lookup for port 0, key PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

}


/**
* @internal prfTgfTunnelDualLookupNestedVLan function
* @endinternal
*
* @brief   Test main func
*/
GT_VOID prfTgfTunnelDualLookupNestedVLan
(
    GT_VOID
)
{
    /* AUTODOC: build ethetnet packet with vid 5 */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    prfTgfTunnelDualLookupNestedVLanBridgeConfigSet();

    /* AUTODOC: add TTI rule 0 with PRV_TGF_TTI_KEY_ETH_E with nestedVlan enabled and action: redirect to egress */
    prfTgfTunnelDualLookupNestedVLanTtiRuleConfigurationSet();

    /*------------------------------------------------------------*/
    /* AUTODOC: send packet and expect TTI match and VLAN tag present */
    prfTgfTunnelDualLookupNestedVLanTrafficGenerate(GT_TRUE);

    /*------------------------------------------------------------*/
    /* AUTODOC: change TTI rule 0 nestedVlan to disabled */
    /* AUTODOC: send packet and expect TTI match, and no VLAN tag */
    prfTgfTunnelDualLookupNestedVLanTtiRuleSet(GT_FALSE, PRV_TGF_TTI0_INDEX_CNS);
    prfTgfTunnelDualLookupNestedVLanTrafficGenerate(GT_FALSE);

    /*------------------------------------------------------------*/
    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_ETH_E with nestedVlan enabled and action: redirect to egress */
    /* AUTODOC: send packet and expect TTI match, and VLAN tag present */
    prfTgfTunnelDualLookupNestedVLanTtiRuleSet(GT_TRUE, PRV_TGF_TTI1_INDEX_CNS);
    prfTgfTunnelDualLookupNestedVLanTrafficGenerate(GT_TRUE);

    /* AUTODOC: test configuration restore */
    prfTgfTunnelDualLookupNestedVLanConfigurationRestore();
}

