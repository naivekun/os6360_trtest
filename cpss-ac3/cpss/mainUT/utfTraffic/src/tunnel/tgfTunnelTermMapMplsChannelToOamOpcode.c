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
* @file tgfTunnelTermMapMplsChannelToOamOpcode.c
*
* @brief SIP5 tunnel term MapMplsChannelToOamOpcode
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
#include <trafficEngine/tgfTrafficEngine.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfOamGen.h>
#include <common/tgfTunnelGen.h>

/* default tunnel term entry index (first rule) */
static GT_U32        prvTgfTtiRuleLookup0Index = 18435;
/* default tunnel term entry index (second rule) */
static GT_U32        prvTgfTtiRuleLookup1Index = 33792;

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS      0

/* port index to receive traffic */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS   1

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/* MPLS Channel Type0  */
#define PRV_TGF_MPLS_CHANNEL_TYPE0_CNS  0x1234

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS          6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4


/* MPLS Channel Type0 Profile */
#define PRV_TGF_MPLS_CHANNEL_TYPE0_PROFILE_CNS  4

/* MPLS Channel Type0  */
#define PRV_TGF_MPLS_CHANNEL_VERSION0_CNS  3

/* OAM opcode0  */
#define PRV_TGF_OAM_OPCODE_CNS  5

/* OAM opcode0  */
#define PRV_TGF_OAM_CMD_PROFILE  6

/* Action Flow Id  */
#define PRV_TGF_ACTION_FLOW_ID_CNS  2

/* MPLS Channel Type0  */
#define PRV_TGF_MPLS_CHANNEL_VERSION0_CNS  3


/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS \
       TGF_L2_HEADER_SIZE_CNS        \
     + TGF_VLAN_TAG_SIZE_CNS          \
     + TGF_ETHERTYPE_SIZE_CNS         \
     + (2 * TGF_MPLS_HEADER_SIZE_CNS) \
     + sizeof(prvTgfPacketMpls1GAchPart)

/******************************************************************************/

/* MPLS Packet */

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketMpls1L2Part =
{
    {0x00, 0x21, 0x43, 0x65, 0x87, 0xA9},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketMpls1VlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMpls1EthertypePart =
{
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/* MPLS LSP addressing */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls1Lsp =
{
    1111 /* Addressing Label */,
    0/*exp*/,
    0/*stack - not last*/,
    100/*timeToLive*/
};

/* MPLS General Associated Label */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls1Gal =
{
    13 /* GAL Generic Associated Label */,
    0/*exp*/,
    1/*stack - last*/,
    101/*timeToLive*/
};


/* MPLS General Associated Channel Header */
static GT_U8 prvTgfPacketMpls1GAchArr[] =
{
    ((1 << 4) | (PRV_TGF_MPLS_CHANNEL_VERSION0_CNS & 0xF)),
    0, /*reserved*/
    ((PRV_TGF_MPLS_CHANNEL_TYPE0_CNS >> 8) & 0xFF),
    (PRV_TGF_MPLS_CHANNEL_TYPE0_CNS & 0xFF)
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketMpls1GAchPart = {
    sizeof(prvTgfPacketMpls1GAchArr),            /* dataLength */
    prvTgfPacketMpls1GAchArr                     /* dataPtr */
};

/* DATA of packet */
static GT_U8 prvTgfPacketMpls1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketMpls1PayloadPart = {
    sizeof(prvTgfPacketMpls1PayloadDataArr),            /* dataLength */
    prvTgfPacketMpls1PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketMpls1PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketMpls1L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketMpls1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMpls1EthertypePart},
    {TGF_PACKET_PART_MPLS_E, &prvTgfPacketMpls1Lsp},
    {TGF_PACKET_PART_MPLS_E, &prvTgfPacketMpls1Gal},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketMpls1GAchPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketMpls1PayloadPart}
};


/* PACKET to send */
static TGF_PACKET_STC packetInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    (sizeof(prvTgfPacketMpls1PartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    prvTgfPacketMpls1PartArray                                          /* partsArray */
};



/**
* @internal tgfTunnelTermMapMplsChannelToOamOpcodeSetTtiRule function
* @endinternal
*
* @brief   Set tti rule to match all the packets and modify
*         iPclUdbConfigTableEnable = GT_TRUE;
*         iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E;
* @param[in] isSecondRule             -  0 - first rule; 1 - second rule
*                                       None
*/
static GT_VOID tgfTunnelTermMapMplsChannelToOamOpcodeSetTtiRule
(
    IN GT_U32                isSecondRule
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ACTION_2_STC ttiAction2;
    GT_U32                   ruleIndex = isSecondRule ? prvTgfTtiRuleLookup1Index:
                                                        prvTgfTtiRuleLookup0Index;

    /* AUTODOC: clear entry */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", 0);

    /* AUTODOC: add TTI rule with: match all the packets */

    /* AUTODOC: set the TTI Rule Action */
    ttiAction2.tag0VlanCmd       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction2.tag0VlanId        = PRV_TGF_EGR_VLANID_CNS;

    ttiAction2.continueToNextTtiLookup  = isSecondRule ? GT_FALSE : GT_TRUE;

    ttiAction2.oamProcessEnable                    = GT_TRUE;
    ttiAction2.oamProfile                          = 0;
    ttiAction2.oamChannelTypeToOpcodeMappingEnable = isSecondRule ? GT_FALSE : GT_TRUE;
    ttiAction2.flowId                              = PRV_TGF_ACTION_FLOW_ID_CNS;

    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_MPLS_E, &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* AUTODOC: validate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(ruleIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);
}


/**
* @internal tgfTunnelTermMapMplsChannelToOamOpcodeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
*                                       None
*/
GT_VOID tgfTunnelTermMapMplsChannelToOamOpcodeTrafficGenerate
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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled ipv6 packet from port 0 */
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
            /* AUTODOC: verify to get packet on port 3 with: */
            UTF_VERIFY_EQUAL3_STRING_MAC(portCntrs.goodPktsRcv.l[0], numOfExpectedRxPackets, 
                "got wrong counters: port[%d], expected [%d], received[%d]\n",
                 prvTgfPortsArray[portIter], numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0]);
        }
    }

}


/**
* @internal tgfTunnelTermMapMplsChannelToOamOpcodeConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID tgfTunnelTermMapMplsChannelToOamOpcodeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS  rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup0Index, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", 
                                                        prvTgfTtiRuleLookup0Index, GT_FALSE);
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup1Index, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", 
                                                        prvTgfTtiRuleLookup1Index, GT_FALSE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* to do oam : config restore */

}

static GT_VOID tgfTunnelTermMapMplsChannelToOamOpcodeSetOamConfig
(
    GT_VOID
)
{
    GT_STATUS             rc = GT_OK;
    PRV_TGF_OAM_ENTRY_STC oamEntry;

    PRV_TGF_OAM_STAGE_TYPE_ENT oamDirection = PRV_TGF_OAM_STAGE_TYPE_INGRESS_E;

    /* AUTODOC: enable OAM processing */
    rc = prvTgfOamEnableSet(oamDirection, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEnableSet");

    /* AUTODOC: Reset OAM flow Id base */
    rc = prvTgfOamTableBaseFlowIdSet(oamDirection, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");

    cpssOsMemSet(&oamEntry, 0, sizeof(oamEntry));
    oamEntry.opcodeParsingEnable  = GT_TRUE;
    oamEntry.packetCommandProfile = PRV_TGF_OAM_CMD_PROFILE;

    /* AUTODOC: set OAM entry */
    rc = prvTgfOamEntrySet(
        oamDirection,
        PRV_TGF_ACTION_FLOW_ID_CNS, &oamEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEntrySet");

    /* AUTODOC: channel type0 map to channel type profile */
    rc = prvTgfOamMplsCwChannelTypeProfileSet(
        PRV_TGF_MPLS_CHANNEL_TYPE0_PROFILE_CNS,
        PRV_TGF_MPLS_CHANNEL_TYPE0_CNS,
        GT_TRUE /*profileEnable*/);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamMplsCwChannelTypeProfileSet");

    /* AUTODOC: channel type0 profile map to opcode */
    rc = prvTgfPclOamChannelTypeProfileToOpcodeMappingSet(
        PRV_TGF_MPLS_CHANNEL_TYPE0_PROFILE_CNS,
        PRV_TGF_OAM_OPCODE_CNS,
        GT_FALSE /*RBitAssignmentEnable*/);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfPclOamChannelTypeProfileToOpcodeMappingSet");

    /* AUTODOC: set Opcode Command to Drop */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
        oamDirection,
        PRV_TGF_OAM_OPCODE_CNS,
        PRV_TGF_OAM_CMD_PROFILE,
        CPSS_PACKET_CMD_DROP_HARD_E,
        0 /*cpuCodeOffset*/);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamOpcodeProfilePacketCommandEntrySet");

}


/* main test func */
GT_VOID tgfTunnelTermMapMplsChannelToOamOpcodeTest
(
    GT_VOID
)
{
    GT_STATUS             rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /*------------------------------------------------------------*/

    tgfTunnelTermMapMplsChannelToOamOpcodeSetTtiRule(0 /*first rule*/);
    tgfTunnelTermMapMplsChannelToOamOpcodeTrafficGenerate(GT_TRUE);


    tgfTunnelTermMapMplsChannelToOamOpcodeSetOamConfig();
    tgfTunnelTermMapMplsChannelToOamOpcodeTrafficGenerate(GT_FALSE);

#if 0
    tgfTunnelTermMapMplsChannelToOamOpcodeSetTtiRule(1 /*second rule*/);
    tgfTunnelTermMapMplsChannelToOamOpcodeTrafficGenerate(GT_TRUE);
#endif

    tgfTunnelTermMapMplsChannelToOamOpcodeConfigRestore();
}






